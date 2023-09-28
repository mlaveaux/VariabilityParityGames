#!/usr/bin/env python

from io import StringIO
import os
import re
import subprocess
import concurrent.futures
import argparse
import logging
import sys
import time
import json
import shutil

from typing import List

formatter = logging.Formatter("%(threadName)-11s %(asctime)s %(levelname)s %(message)s")
logging.basicConfig(level=logging.DEBUG)

# This path should always be consistent
FTSMMC_JAR = "../implementation/FTSMMC/app/build/libs/app.jar"

# A regex matching in=out
mapping_regex = re.compile(r"(.*)=(.*)")

# A regex matching a transition in the aut format '(from, action, to)'
transition_regex = re.compile(r"\(([0-9]*),\"(.*)\",([0-9]*)\)")
confs_regex = re.compile(r"confs .*;")
conf_regex = re.compile(r"\|[-|+|0|1]*")


class MyLogger(logging.Logger):
    """My own logger that stores the log messages into a string stream"""

    def __init__(self, name: str, filename: str | None = None, terminator="\n"):
        """Create a new logger instance with the given name"""
        logging.Logger.__init__(self, name, logging.DEBUG)

        self.stream = StringIO()
        handler = logging.StreamHandler(self.stream)
        handler.terminator = terminator
        handler.setFormatter(formatter)

        if filename is not None:
            self.addHandler(logging.FileHandler(filename))

        standard_output = logging.StreamHandler(sys.stderr)
        standard_output.terminator = terminator

        self.addHandler(handler)
        self.addHandler(standard_output)

    def getvalue(self) -> str:
        """Returns the str that has been logged to this logger"""
        return self.stream.getvalue()


def is_newer(inputfile: str, outputfile: str, ignore=False) -> bool:
    """Returns true iff the input file is newer than the output file"""
    if ignore:
        return True

    try:
        return os.path.getmtime(inputfile) > os.path.getmtime(outputfile)
    except OSError:
        return True


def run_program(cmds, logger, process=None):
    """Runs the given program with sensible defaults, and logs the results to the logger"""

    with subprocess.Popen(
        cmds, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True
    ) as proc:
        for line in proc.stdout:
            logger.info(line)

            if process is not None:
                process(line)

        proc.wait()

        if proc.returncode != 0:
            raise subprocess.CalledProcessError(proc.returncode, proc.args)


def prepare(
    directory: str,
    tmp_directory: str,
    mcrl2_name: str,
    properties: List[str],
    logger: MyLogger,
    executor: concurrent.futures.ThreadPoolExecutor,
):
    """Prepares the parity games for one experiment, consisting of an mCRL2 specification and several properties.
    Returns a set of futures"""

    # Ensure that tmp directory exists since the mCRL2 tools cannot make it
    try:
        os.mkdir(tmp_directory)
    except OSError:
        logger.debug(f"{tmp_directory} already exists")

    # Convert the mcrl2 to an aut file.
    base, _ = os.path.splitext(mcrl2_name)
    mcrl2_file = os.path.join(directory, mcrl2_name)
    lps_file = os.path.join(tmp_directory, base + ".lps")
    aut_file = os.path.join(tmp_directory, base + ".aut")

    mcrl22lps_exe = shutil.which("mcrl22lps")
    lps2lts_exe = shutil.which("lps2lts")

    if is_newer(mcrl2_file, lps_file):
        run_program([mcrl22lps_exe, "--verbose", mcrl2_file, lps_file], logger)
    if is_newer(lps_file, aut_file):
        run_program([lps2lts_exe, "--verbose", lps_file, aut_file], logger)

    # Convert the actions in the .aut files to move features from the data into the action label.
    # File contains from=to per line for each action.
    mapping = {}

    # Indicates that the .aut file has been generated.
    aut_generated = False
    actionrename_file = os.path.join(directory, "actionrename")
    aut_renamed_file = os.path.join(tmp_directory, base + ".renamed.aut")

    if is_newer(actionrename_file, aut_renamed_file) or is_newer(
        aut_file, aut_renamed_file
    ):
        with open(actionrename_file, encoding="utf-8") as file:
            for line in file.readlines():
                result = mapping_regex.match(line)
                if result is not None:
                    mapping[result.group(1)] = result.group(2)

        logger.debug("renaming applied: %s", mapping)

        # Rename the action labels in the aut file based on the mapping computed above
        with open(aut_renamed_file, "w", encoding="utf-8") as outfile:
            with open(aut_file, encoding="utf-8") as file:
                for line in file.readlines():
                    result = transition_regex.match(line)
                    if result is not None:
                        action = result.group(2)
                        action = mapping.get(action, action)
                        outfile.write(
                            f'({result.group(1)},"{action}",{result.group(3)})\n'
                        )
                    else:
                        outfile.write(line)

        aut_generated = True

    # Generate the SVPG for every property
    futures = {}
    games = []
    featurediagram_file = os.path.join(directory, "FD")

    for prop in properties:
        mcf_file = os.path.join(directory, prop)
        prop, _ = os.path.splitext(prop)
        game_file = os.path.join(tmp_directory, prop + ".svpg")

        name = f"{os.path.basename(aut_file)} and {os.path.basename(mcf_file)}"
        games.append((os.path.basename(os.path.normpath(directory)), prop, game_file))
        if (
            is_newer(featurediagram_file, game_file)
            or is_newer(aut_renamed_file, game_file)
            or is_newer(mcf_file, game_file)
        ):
            logger.info(f"Generating parity game for {name}")

            # Convert .aut and formula into a VPG
            # run_program requires no terminator when logging
            program_logger = MyLogger(name, terminator="")
            futures[
                executor.submit(
                    run_program,
                    [
                        "java",
                        "-jar",
                        "-Xss100M",
                        "-Xmx6G",
                        FTSMMC_JAR,
                        "vpg",
                        featurediagram_file,
                        aut_renamed_file,
                        mcf_file,
                        game_file,
                    ],
                    program_logger,
                )
            ] = ("generate", name, program_logger)

    if aut_generated:
        # Generate the projections of the .aut
        name = f"{os.path.basename(aut_file)}"
        aut_projection_name = os.path.join(tmp_directory, base + "_project_")
        logger.info("Generating projections for %s", os.path.basename(aut_file))

        # run_program requires no terminator when logging
        program_logger = MyLogger(name, terminator="")
        futures[
            executor.submit(
                run_program,
                [
                    "java",
                    "-jar",
                    "-Xss100M",
                    "-Xmx6G",
                    FTSMMC_JAR,
                    "project",
                    featurediagram_file,
                    aut_renamed_file,
                    aut_projection_name,
                ],
                program_logger,
            )
        ] = ("project", name, program_logger)

    return (futures, games)


def project_single(
    featurediagram_file,
    aut_file,
    mcf_file,
    game_file,
    single_game_file,
    pg_game_file,
    logger,
):
    """Computes the parity game for a single projection, computes the single
    conf and computes the parity game without features"""

    if is_newer(aut_file, game_file):
        # Create a vpg from the aut and property combination.
        run_program(
            [
                "java",
                "-jar",
                "-Xss100M",
                "-Xmx6G",
                FTSMMC_JAR,
                "vpg",
                featurediagram_file,
                aut_file,
                mcf_file,
                game_file,
            ],
            logger,
        )

    if is_newer(game_file, pg_game_file):
        # Remove the configurations from the resulting svpg.
        with open(pg_game_file, "w", encoding="utf-8") as outfile:
            with open(game_file, encoding="utf-8") as file:
                for line in file.readlines():
                    result = confs_regex.match(line)
                    if result is not None:
                        outfile.write("")
                    else:
                        outfile.write(re.sub(conf_regex, "", line))

    if is_newer(game_file, single_game_file):
        # Remove the configurations from the resulting svpg.
        with open(single_game_file, "w", encoding="utf-8") as outfile:
            with open(game_file, encoding="utf-8") as file:
                for line in file.readlines():
                    result = confs_regex.match(line)
                    if result is not None:
                        outfile.write("confs -;\n")
                    else:
                        outfile.write(re.sub(conf_regex, "|-", line))


def prepare_projections(
    directory: str,
    tmp_directory: str,
    properties: List[str],
    logger: MyLogger,
    executor: concurrent.futures.ThreadPoolExecutor,
):
    """Convert the projected .aut files into parity games"""

    featurediagram_file = os.path.join(directory, "FD")

    # Convert .aut and formula into a VPG
    futures = {}
    games = []
    for filename in os.listdir(tmp_directory):
        if "project" in filename and ".aut" in filename:
            for prop in properties:
                base, _ = os.path.splitext(filename)
                prop_name, _ = os.path.splitext(prop)

                name = f"{base}_{prop_name}"
                mcf_file = os.path.join(directory, prop)
                aut_file = os.path.join(tmp_directory, filename)
                game_file = os.path.join(tmp_directory, name + ".svpg")
                pg_game_file = os.path.join(tmp_directory, name + ".pg")
                single_game_file = os.path.join(tmp_directory, name + "_single.svpg")
                games.append(
                    (os.path.basename(os.path.normpath(directory)), prop, pg_game_file)
                )
                games.append(
                    (
                        os.path.basename(os.path.normpath(directory)),
                        prop,
                        single_game_file,
                    )
                )

                if (
                    is_newer(aut_file, game_file)
                    or is_newer(game_file, single_game_file)
                    or is_newer(game_file, pg_game_file)
                ):
                    logger.info(
                        "Generating parity game for projection '%s' and property '%s'",
                        base,
                        prop,
                    )

                    # run_program requires no terminator when logging
                    program_logger = MyLogger(game_file, terminator="")
                    futures[
                        executor.submit(
                            project_single,
                            featurediagram_file,
                            aut_file,
                            mcf_file,
                            game_file,
                            single_game_file,
                            pg_game_file,
                            program_logger,
                        )
                    ] = (name, program_logger)

    return (futures, games)

def prepare_experiments(
    experiments, logger: MyLogger, executor: concurrent.futures.ThreadPoolExecutor
) -> List[tuple[str, str, str]]:
    """Runs all preparation steps for the given experiments"""

    # First, we submit the jobs to generate the aut files and prepare the experiment.
    futures = {}
    for experiment in experiments:
        directory, mcrl2_name, properties = experiment

        # The directory in which to store all generated files
        tmp_directory = directory + "tmp/"

        prepare_logger = MyLogger(f"prepare_{mcrl2_name}")
        logger.info("Starting preparation for experiment '%s'...", directory)
        futures[
            executor.submit(
                prepare,
                directory,
                tmp_directory,
                mcrl2_name,
                properties,
                prepare_logger,
                executor,
            )
        ] = (directory, properties, prepare_logger)

    # The list of all games.
    all_games = []
    for future in concurrent.futures.as_completed(futures):
        directory, properties, prepare_logger = futures[future]

        try:
            prepare_futures, games = future.result()
            all_games.extend(games)
            logger.info(prepare_logger.getvalue())

            project_futures = {}

            # We generate the main parity games and the projections of the .aut file.
            for future in concurrent.futures.as_completed(prepare_futures):
                future.result()

                action, name, future_logger = prepare_futures[future]
                if action == "generate":
                    logger.info(future_logger.getvalue())
                    logger.info("Finished generating parity game for %s", name)
                elif action == "project":
                    logger.info(future_logger.getvalue())
                    logger.info("Finished generating projections for %s", name)
                    all_games.extend(games)

            # We generate the parity games for the projections     
            tmp_directory = directory + "tmp/"

            # Create a parity game for every projection
            prepare_project_logger = MyLogger(f"prepare_project_{directory}")
            new_futures, games = prepare_projections(
                directory,
                tmp_directory,
                properties,
                prepare_project_logger,
                executor,
            )

            project_futures.update(new_futures)

            # Wait for all projections to be generated.
            for future in concurrent.futures.as_completed(project_futures):
                future.result()
                
            # Add the projections to the list of games
            for prop in properties:
                prop, _ = os.path.splitext(prop)
                for file in os.listdir(tmp_directory):
                    if prop in file and "single.svpg" in file:
                        games.append(
                            (
                                os.path.basename(os.path.normpath(directory)),
                                prop,
                                os.path.join(tmp_directory, file),
                            )
                        )

            logger.info("Finished preparation for experiment '%s'", directory)
        except OSError as exp:
            logger.info("Preparation '%s' failed with exception %s", directory, exp)
            return []
        except KeyboardInterrupt:
            logging.error("Interrupted program")
            return []        

    return all_games

solving_time_regex = re.compile(r"Solving time: (.*) ns")

class TimeParser:
    """Extracts the solving time from the stdout of the program"""

    time: float | None = None

    def __call__(self, line):
        result = solving_time_regex.match(line)
        if result:
            self.time = float(result.group(1))


def run_benchmark_single(tool: str, game: str, logger: MyLogger) -> tuple[float, float]:
    """Run a single benchmark and return the time it took in seconds"""
    tool_exe = shutil.which(tool)

    start = time.time()
    time_parser = TimeParser()
    run_program([tool_exe, game], logger, time_parser)

    assert time_parser.time is not None

    return (time.time() - start, time_parser.time / 1_000_000_000)


def run_benchmark(
    experiment: str,
    prop: str,
    tool: str,
    game: str,
    executor: concurrent.futures.ThreadPoolExecutor,
):
    """Run the benchmarks for a single experiment"""

    # Run several experiments and gather their average
    futures = {}
    for i in range(0, 5):
        # run_program requires no terminator when logging
        benchmark_logger = MyLogger(f"{i}-{tool}-{game}", terminator="")
        futures[executor.submit(run_benchmark_single, tool, game, benchmark_logger)] = (
            experiment,
            prop,
            game,
            tool,
            benchmark_logger,
        )

    return futures


family_solving_regex = re.compile(
    r"For product ([0-9]*) the following vertices are in: ([0-9, ]*)"
)

class FamilySolveParser:
    """Extracts the winners for each product on a family based solve"""

    def __init__(self):
        # A mapping from product to the pair of winning sets.
        self.solution = {}
        self.solutions_for = ""

    def __call__(self, line):
        if "W0" in line:
            # The next lines contains the winners for W0
            self.solutions_for = "W0"

        if "W1" in line:
            # The next lines contains the winners for W1
            self.solutions_for = "W1"

        result = family_solving_regex.match(line)
        if result:
            product = result.group(1)
            vertices = []
            for vert in result.group(2).split(","):
                if vert != "":
                    # Convert to numbers
                    vertices.append(int(vert))

            if not product in self.solution:
                self.solution[product] = (None, None)

            if self.solutions_for == "W0":
                self.solution[product] = (vertices, self.solution[product][1])

            if self.solutions_for == "W1":
                self.solution[product] = (self.solution[product][0], vertices)


def run_solution_solver(tool: str, game: str, logger: MyLogger) -> dict[type, type]:
    """Run a single benchmark and return the time it took in seconds"""

    tool_exe = shutil.which(tool)

    parser = FamilySolveParser()
    run_program([tool_exe, game, "f"], logger, parser)

    return parser.solution


def verify_result(executor, tool, game, experiment, prop):
    """Runs both the family based solver and the product based solver and computes the winning partition."""

    # Run the family based solver and parse the resulting winning partitions
    futures = {}

    logger = logging.Logger(f"solution-{tool}-{game}", logging.FATAL)
    futures[executor.submit(run_solution_solver, tool, game, logger)] = (
        "family",
        os.path.basename(game),
        logger,
        experiment,
        prop,
    )

    return futures

def verify_results(all_games, tools, logger, executor):
    """ Runs the solver on all the product and family games and compares the winning pairs """

    # Verify the solver results.
    verify_futures = {}
    for experiment, prop, game in all_games:
        for tool in tools:
            logger.info(
                "Verifying game for '%s' with tool '%s'",
                os.path.basename(game),
                tool,
            )

            verify_futures.update(verify_result(executor, tool, game, experiment, prop))

    solutions = {}
    for future in concurrent.futures.as_completed(verify_futures):
        _, game, _, experiment, prop = verify_futures[future]

        solution = future.result()
        logger.info("Obtained solution for game '%s' and '%s'", game, experiment)
        if not experiment in solutions:
            solutions[experiment] = {}

        if not prop in solutions[experiment]:
            solutions[experiment][prop] = {}

        solutions[experiment][prop][game] = solution

    for experiment, properties in solutions.items():
        # Find the family solution
        family_solutions = {}
        for prop, games in properties.items():
            for game, solution in games.items():
                if "project" not in game:
                    logging.info("Checking property %s and family game %s", prop, game)
                    family_solutions = solution
                    break

            for product, result in family_solutions.items():
                checked = False
                for product_prop, solution in games.items():
                    logging.info(
                        "Validating product %s for game %s", product, product_prop
                    )
                    if product in product_prop:
                        # The product result must match the family result.
                        logging.info("Checking product %s", product)

                        left_W0 = set(map(int, result[0]))
                        left_W1 = set(map(int, result[1]))

                        right_W0 = set(map(int, solution['1'][0]))
                        right_W1 = set(map(int, solution['1'][1]))

                        diff0 = left_W0 ^ right_W0
                        if diff0:
                            logger.fatal("Mismatch in W0 %s", diff0)
                            assert False

                        diff1 = left_W1 ^ right_W1
                        if diff1:
                            logger.error("Mismatch in W1 %s", diff1)
                            assert False
                            
                        checked = True

                assert checked


def main():
    """The main function"""

    # Parse some configuration options
    parser = argparse.ArgumentParser(
        prog="run.py",
        description="Prepares the variability parity games and runs the solving experiments",
        epilog="",
    )

    parser.add_argument("-m", "--max_workers", action="store", default=1, type=int)
    parser.add_argument(
        "-t", "--mcrl2-binpath", action="store", type=str, required=True
    )
    parser.add_argument(
        "-s", "--solver-binpath", action="store", type=str, required=True
    )

    args = parser.parse_args()

    os.environ["PATH"] += os.pathsep + args.mcrl2_binpath
    os.environ["PATH"] += os.pathsep + args.solver_binpath

    experiments = [
        (
            "../cases/elevator/",
            "elevator.mcrl2",
            [
                "prop1.mcf",
                "prop2.mcf",
                "prop3.mcf",
                "prop4.mcf",
                "prop5.mcf",
                "prop6.mcf",
                "prop7.mcf",
            ],
        ),
        (
            "../cases/minepump/",
            "minepump_fts.mcrl2",
            [
                "phi1.mcf",
                "phi2.mcf",
                "phi3.mcf",
                "phi4.mcf",
                "phi5.mcf",
                "phi6.mcf",
                "phi7.mcf",
                "phi8.mcf",
                "phi9.mcf",
            ],
        ),
    ]

    logger = MyLogger("main", "results.log")
    tools = ["VPGSolver_bdd"]

    # Prepare the variability parity games for all the properties and specifications.
    with concurrent.futures.ThreadPoolExecutor(
        max_workers=args.max_workers
    ) as executor:
        all_games = prepare_experiments(experiments, logger, executor)

        #verify_results(all_games, tools, logger, executor)

        return

        # Execute the solvers to measure the solving time.
        benchmark_futures = {}
        for experiment, prop, game in all_games:
            for tool in tools:
                if ".svpg" in game:
                    logger.info(
                        "Starting benchmarks for '%s' and '%s'",
                        os.path.basename(game),
                        tool,
                    )

                    benchmark_futures.update(
                        run_benchmark(experiment, prop, tool, game, executor)
                    )

        # Collect the benchmark results.
        all_results = {}
        for future in concurrent.futures.as_completed(benchmark_futures):
            experiment, prop, game, tool, benchmark_logger = benchmark_futures[future]

            (total, solving) = future.result()

            logger.info(benchmark_logger.getvalue())
            logger.info(
                "Finished benchmark %s with game %s in %s",
                tool,
                os.path.basename(game),
                total,
            )

            result = {"total": total, "solving": solving}

            # Construct the dictionary
            game = os.path.basename(game)
            if experiment not in all_results:
                all_results[experiment] = {}

            if prop not in all_results[experiment]:
                all_results[experiment][prop] = {}

            if game not in all_results[experiment][prop]:
                all_results[experiment][prop][game] = []

            entry = {tool: result}

            all_results[experiment][prop][game].append(entry)

        # writing the dictionary data into the corresponding JSON file
        for _, result in all_results.items():
            with open("results.json", "w", encoding="utf-8") as json_file:
                json.dump(result, json_file, indent=2)

        # Process the results into a table.


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        logging.error("Interrupted program")
