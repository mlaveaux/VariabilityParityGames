#!/usr/bin/env python

from io import StringIO
import os
import re
import subprocess
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

#
# UTILITY
#

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
    """Runs the given program with sensible defaults, and logs the results to the logger.
    Returns the execution time in seconds."""

    start_time = time.time()
    
    with subprocess.Popen(
        cmds, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True
    ) as proc:
        for line in proc.stdout:
            logger.info(line.strip())

            if process is not None:
                process(line.strip())

        proc.wait()

        if proc.returncode != 0:
            raise subprocess.CalledProcessError(proc.returncode, proc.args)

    elapsed_time = time.time() - start_time
    return elapsed_time

#
# PREPARATION
#

def prepare(
    directory: str,
    tmp_directory: str,
    mcrl2_name: str,
    properties: List[str],
    logger: MyLogger,
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
    update_projections = False
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

    # Generate the SVPG for every property
    featurediagram_file = os.path.join(directory, "FD")

    for prop in properties:
        mcf_file = os.path.join(directory, prop)
        prop, _ = os.path.splitext(prop)
        game_file = os.path.join(tmp_directory, prop + ".svpg")

        name = f"{os.path.basename(aut_file)} and {os.path.basename(mcf_file)}"
        if (
            is_newer(featurediagram_file, game_file)
            or is_newer(aut_renamed_file, game_file)
            or is_newer(mcf_file, game_file)
        ):
            logger.info(f"Generating parity game for {name}")

            # Convert .aut and formula into a VPG
            run_program(
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
            logger)

    update_projections = True

    vpgsolver_exe = shutil.which("VPGSolver_bdd")

    if update_projections:
        for file in os.listdir(tmp_directory):
            file = tmp_directory + file
            if ".svpg" in file:
                prop, _ = os.path.splitext(file)
                logger.info("Generating projections for %s", os.path.basename(file))

                run_program(
                [
                    vpgsolver_exe,
                    file,
                    "--project",
                    f"{prop}_project_"
                ],
                logger)
        
        for file in os.listdir(tmp_directory):
            file = tmp_directory + file
            if ".pg" in file and "reachable" not in file:
                prop, _ = os.path.splitext(file)
                logger.info("Generating reachable part for %s", os.path.basename(file))

                run_program(
                [
                    vpgsolver_exe,
                    file,
                    "--reachable",
                    f"{prop}_reachable.pg",
                    "--parity-game"
                ],
                logger)

def prepare_experiments(
    experiments, logger: MyLogger
):
    """Runs all preparation steps for the given experiments"""

    for experiment in experiments:
        directory, mcrl2_name, properties = experiment

        # The directory in which to store all generated files
        tmp_directory = directory + "tmp/"

        logger.info("Starting preparation for experiment '%s'...", directory)
        prepare(
                directory,
                tmp_directory,
                mcrl2_name,
                properties,
                logger)  
                  
#
# VALIDATION
#
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
            vertices = set()
            for vert in result.group(2).split(","):
                if vert != "":
                    # Convert to numbers
                    vertices.add(int(vert))

            if not product in self.solution:
                self.solution[product] = (None, None)

            if self.solutions_for == "W0":
                self.solution[product] = (vertices, self.solution[product][1])

            if self.solutions_for == "W1":
                self.solution[product] = (self.solution[product][0], vertices)

product_solving_regex = re.compile(
    r"(W[0,1]): ([0-9, ]*)"
)

class ProductSolveParser:
    """Extracts the winners for each product on a product based solve"""

    def __init__(self):
        # A mapping from product to the pair of winning sets.
        self.solution = (set(), set())

    def __call__(self, line):

        result = product_solving_regex.match(line)
        if result:
            vertices = set()
            for vert in result.group(2).split(","):
                if vert != "":
                    # Convert to numbers
                    vertices.add(int(vert))

            if result.group(1) == "W0":
                self.solution = (vertices, self.solution[1])
            elif result.group(1) == "W1":
                self.solution = (self.solution[0], vertices)
            else:
                assert False, "Neither expected values match"

pgsolve_solving_regex = re.compile(
    r"\{([0-9, ]*)\}"
)

class PGSolveParser:
    def __init__(self):
        # A mapping from product to the pair of winning sets.
        self.solution = (set(), set())
        self.solutions_for = ""

    def __call__(self, line):
        if "Player 0 wins from nodes" in line:
            # The next lines contains the winners for W0
            self.solutions_for = "W0"

        if "Player 1 wins from nodes" in line:
            # The next lines contains the winners for W1
            self.solutions_for = "W1"

        result = pgsolve_solving_regex.match(line.strip())
        if result:
            vertices = set()
            for vert in result.group(1).split(","):
                if vert != "":
                    # Convert to numbers
                    vertices.add(int(vert))

            if self.solutions_for == "W0":
                self.solution = (vertices, self.solution[1])
            elif self.solutions_for == "W1":
                self.solution = (self.solution[0], vertices)

def verify_results(experiments, logger):
    """ Runs the solver on all the product and family games and compares the winning pairs """

    vpgsolver_exe = shutil.which("VPGSolver_bdd")
    pgsolver_exe = shutil.which("pgsolver")

    for experiment in experiments:
        directory, _, _ = experiment

        # The directory in which to store all generated files
        tmp_directory = directory + "tmp/"

        # For every property solve the family based game, and then each corresponding product game.
        for alg in range(0, 3):
            logging.info(f"Checking algorithm {alg}")

            for file in os.listdir(tmp_directory):
                file = tmp_directory + file
                if ".svpg" in file:
                    logging.info("Checking solutions for game %s", os.path.basename(file))

                    family_parser = FamilySolveParser()
                    print(vpgsolver_exe)
                    run_program([vpgsolver_exe, file, "--print-solution", "--algorithm", f"{alg}"], logging.Logger('ignore'), family_parser)

                    for product, solution in family_parser.solution.items():

                        base, _ = os.path.splitext(file)
                        for product_file in os.listdir(tmp_directory):
                            product_file = tmp_directory + product_file
                            if base in product_file and product in product_file and "reachable" not in product_file:
                                # The product result must match the family result.
                                logging.info("Checking product  %s", product)

                                product_parser = ProductSolveParser()
                                run_program([vpgsolver_exe, product_file, "--print-solution", "--parity-game"], logging.Logger('ignore'), product_parser)

                                if pgsolver_exe:
                                    pgsolve_parser = PGSolveParser()
                                    run_program([pgsolver_exe, product_file, "-global", "recursive"], logging.Logger('ignore'), pgsolve_parser)

                                    diff0 = product_parser.solution[0] ^ pgsolve_parser.solution[0]
                                    assert not diff0, f"Mismatch in W0 {diff0}"

                                    diff1 = product_parser.solution[1] ^ pgsolve_parser.solution[1]
                                    assert not diff1, f"Mismatch in W1 {diff1}"

                                diff0 = product_parser.solution[0] ^ solution[0]
                                assert not diff0, f"Mismatch in W0 {diff0}"

                                diff1 = product_parser.solution[1] ^ solution[1]
                                assert not diff1, f"Mismatch in W1 {diff1}"    

solving_time_regex = re.compile(r"Solving time: (.*) ms")
recursive_calls_regex = re.compile(r"Performed ([0-9]*) recursive calls")

class TimeParser:
    """Extracts the solving time from the stdout of the program"""

    time: float|None = None
    recursive_calls: int|None = None
    solve: FamilySolveParser

    def __init__(self):
        self.solve = FamilySolveParser()

    def __call__(self, line):
        result = solving_time_regex.match(line)
        if result:
            self.time = float(result.group(1))

        result = recursive_calls_regex.match(line)
        if result:
            self.recursive_calls = int(result.group(1))

        self.solve(line)

    def even_wins(self) -> int:
        """ The number of games won by player even """
        even = 0
        for (_, solution) in self.solve.solution.items():
            if solution[0] is not None:
                if '0' in solution[0]:
                    even += 1

        return even

def run_benchmark(
    game: str,
):
    """Run the benchmarks for a single experiment"""
    vpgsolver_exe = shutil.which("VPGSolver_bdd")

    # Run several experiments and gather their average
    results = {}

    name = os.path.basename(game)
    results[name] = {}
    for i in range(0, 5):
        start = time.time()
        time_parser = TimeParser()

        if ".svpg" in game:
            for alg in range(0, 3):
                run_program([vpgsolver_exe, game, "--algorithm", f"{alg}"], logging.Logger('ignore'), time_parser)
                assert time_parser.time is not None
                assert time_parser.recursive_calls is not None
                results[name].setdefault(f"algorithm{alg}", []).append({"total": time.time() - start, "solving": time_parser.time / 1_000, "recursive_calls": time_parser.recursive_calls, "even_wins": time_parser.even_wins()})
        else:            
            run_program([vpgsolver_exe, game, "--parity-game"], logging.Logger('ignore'), time_parser)

            # Add the result
            assert time_parser.time is not None
            assert time_parser.recursive_calls is not None
            results[name].setdefault("solver", []).append({"total": time.time() - start, "solving": time_parser.time / 1_000, "recursive_calls": time_parser.recursive_calls, "even_wins": time_parser.even_wins()})

    return results

def main():
    """The main function"""

    # Parse some configuration options
    parser = argparse.ArgumentParser(
        prog="run.py",
        description="Prepares the variability parity games and runs the solving experiments",
        epilog="",
    )

    parser.add_argument(
        "-t", "--mcrl2-binpath", action="store", type=str, required=True
    )
    parser.add_argument(
        "-s", "--solver-binpath", action="store", type=str, required=True
    )
    parser.add_argument(
        "-p", "--pgsolver-binpath", action="store", type=str
    )

    args = parser.parse_args()

    os.environ["PATH"] += os.pathsep + args.mcrl2_binpath.strip()
    os.environ["PATH"] += os.pathsep + args.solver_binpath.strip()
    if args.pgsolver_binpath:
        os.environ["PATH"] += os.pathsep + args.pgsolver_binpath.strip()

    experiments = [
        # (
        #     "../cases/elevator/",
        #     "elevator.mcrl2",
        #     [
        #         "prop1.mcf",
        #         "prop2.mcf",
        #         "prop3.mcf",
        #         "prop4.mcf",
        #         "prop5.mcf",
        #         "prop6.mcf",
        #         "prop7.mcf",
        #     ],
        # ),
        # (
        #     "../cases/minepump/",
        #     "minepump_fts.mcrl2",
        #     [
        #         "phi1.mcf",
        #         "phi2.mcf",
        #         "phi3.mcf",
        #         "phi4.mcf",
        #         "phi5.mcf",
        #         "phi6.mcf",
        #         "phi7.mcf",
        #         "phi8.mcf",
        #         "phi9.mcf",
        #     ],
        # ),
        (
            "../cases/vending_machine/",
            "VendingMachine.mcrl2",
            [
                "infinitely_often_cappuccino.mcf"
            ]
        )
    ]

    logger = MyLogger("main", "results.log")

    # Prepare the variability parity games for all the properties and specifications.
    prepare_experiments(experiments, logger)

    # Run the family solver and for every product check the corresponding results.
    verify_results(experiments, logger)

    all_results = {}
    for experiment in experiments:
        directory, _, properties = experiment

        # The directory in which to store all generated files
        tmp_directory = directory + "tmp/"

        # Collect the benchmark results.
        experiment = os.path.basename(os.path.normpath(directory))
        all_results[experiment] = {}

        for prop in properties:
            logger.info("Starting benchmarks for experiment '%s' and property '%s'...", experiment, prop)
            all_results[experiment][prop] = {}

            for file in os.listdir(tmp_directory):
                base, _ = os.path.splitext(prop)
                if base in file:
                    logger.info("Benchmaking solving %s", file)
                    file = tmp_directory + file
                    result = run_benchmark(file)

                    all_results[experiment][prop].update(result)

    # writing the dictionary data into the corresponding JSON file
    with open("results.json", "w", encoding="utf-8") as json_file:
        json.dump(all_results, json_file, indent=2)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        logging.error("Interrupted program")
