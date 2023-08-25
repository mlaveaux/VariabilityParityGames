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


def run_program(cmds, logger):
    """Runs the given program with sensible defaults, and logs the results to the logger"""

    with subprocess.Popen(
        cmds, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True
    ) as proc:
        for line in proc.stdout:
            logger.info(line)

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
                            f'({result.group(1)},"{action}",{result.group(3)})'
                            + os.linesep
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
        games.append((os.path.basename(directory), prop, game_file))
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

    # Generate the projections of the .aut
    if aut_generated:
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
    featurediagram_file, aut_file, mcf_file, game_file, renamed_game_file, logger
):
    """Computes the parity game for  a single projection, and computes the parity game without features"""

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

    if is_newer(game_file, renamed_game_file):
        # Remove the configurations from the resulting svpg.
        with open(renamed_game_file, "w", encoding="utf-8") as outfile:
            with open(game_file, encoding="utf-8") as file:
                for line in file.readlines():
                    result = confs_regex.match(line)
                    if result is not None:
                        outfile.write("confs;" + os.linesep)
                    else:
                        outfile.write(re.sub(conf_regex, "", line))


def prepare_projections(
    directory: str,
    tmp_directory: str,
    properties: List[str],
    logger: MyLogger,
    executor: concurrent.futures.ThreadPoolExecutor,
):
    """Convert the projected .aut files into parity games"""

    featurediagram_file = os.path.join(directory, "FD_project")

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
                renamed_game_file = os.path.join(tmp_directory, name + ".pg")
                games.append((os.path.basename(directory), prop, renamed_game_file))

                if is_newer(aut_file, game_file) or is_newer(
                    game_file, renamed_game_file
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
                            renamed_game_file,
                            program_logger,
                        )
                    ] = (name, program_logger)

    return (futures, games)


def run_benchmark_single(tool: str, game: str, logger: MyLogger) -> float:
    """Run a single benchmark and return the time it took in seconds"""
    tool_exe = shutil.which(tool)

    start = time.time()
    run_program([tool_exe, game], logger)
    return time.time() - start


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
    for i in range(0, 1):
        # run_program requires no terminator when logging
        benchmark_logger = MyLogger(f"{i}-{tool}-{game}", terminator="")
        futures[
            executor.submit(run_benchmark_single, tool, game, benchmark_logger)
        ] = (experiment, prop, game, tool, benchmark_logger)

    return futures

def prepare_experiments(experiments, logger: MyLogger, executor: concurrent.futures.ThreadPoolExecutor):
    """ Runs all preparation steps for the given experiments """

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

            # We generate the parity games for the projections
            for future in concurrent.futures.as_completed(prepare_futures):
                future.result()
                action, name, future_logger = futures[future]
                if action == "generate":
                    name, future_logger = prepare_futures[future]
                    logger.info(future_logger.getvalue())
                    logger.info("Finished generating parity game for %s", name)
                elif action == "project":
                    logger.info(future_logger.getvalue())
                    logger.info("Finished generating projections for %s", name)

                    # The directory in which to store all generated files
                    tmp_directory = directory + "tmp/"

                    # Create a parity game for every projection
                    prepare_project_logger = MyLogger(
                        f"prepare_project_{directory}"
                    )
                    new_futures, games = prepare_projections(
                        directory,
                        tmp_directory,
                        properties,
                        prepare_project_logger,
                        executor,
                    )
                    prepare_futures.update(new_futures)
                    all_games.extend(games)

            logger.info("Finished preparation for experiment '%s'", directory)
        except OSError as exp:
            logger.info("Preparation '%s' failed with exception %s", directory, exp)
            return -1
        except KeyboardInterrupt:
            logging.error("Interrupted program")
            return -1

    return all_games

def main():
    """The main function"""

    # Parse some configuration options
    parser = argparse.ArgumentParser(
        prog="run.py",
        description="Prepares the variability parity games and runs the solving experiments",
        epilog="",
    )

    parser.add_argument("-m", "--max_workers", action="store", default=1, type=int)
    parser.add_argument('-t', "--mcrl2-binpath", action="store", type=str, required=True)
    parser.add_argument('-s', "--solver-binpath", action="store", type=str, required=True)

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

        # Execute the solvers to measure the solving time.
        benchmark_futures = {}
        for experiment, prop, game in all_games:
            for tool in tools:
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

            result = {}
            try:
                measurement = future.result()

                logger.info(benchmark_logger.getvalue())
                logger.info(
                    "Finished benchmark %s with game %s in %s",
                    tool,
                    os.path.basename(game),
                    measurement,
                )

                result = {"timing": measurement}
            except Exception as exp:
                logger.info(
                    "Benchmark %s with game %s failed %s",
                    tool,
                    os.path.basename(game),
                    exp,
                )
                result = {"timing": -1}

            # Construct the dictionary
            game = os.path.basename(game)
            if experiment not in all_results:
                all_results[experiment] = {}

            if prop not in all_results[experiment]:
                all_results[experiment][prop] = {}

            if game not in all_results[experiment][prop]:
                all_results[experiment][prop][game] = {}

            all_results[experiment][prop][game][tool] = result

        # writing the dictionary data into the corresponding JSON file
        for _, result in all_results.items():
            with open(
                "results.json", "w", encoding="utf-8"
            ) as json_file:
                json.dump(result, json_file, indent=2)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        logging.error("Interrupted program")
