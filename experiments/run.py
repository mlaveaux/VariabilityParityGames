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

formatter = logging.Formatter('%(threadName)-11s %(asctime)s %(levelname)s %(message)s')
logging.basicConfig(level=logging.DEBUG)

# This is just for testing, setting the PATH to find the executable.
os.environ['PATH'] += os.pathsep + '../../mCRL2-build/stage/bin/RelWithDebInfo/'
os.environ['PATH'] += os.pathsep + '../build/VPGSolver/RelWithDebInfo/'
FTSMMC_JAR = '../implementation/FTSMMC/app/build/libs/app.jar'

# A regex matching in=out
mapping_regex = re.compile(r'(.*)=(.*)')

# A regex matching a transition in the aut format '(from, action, to)'
transition_regex = re.compile(r'\(([0-9]*),\"(.*)\",([0-9]*)\)')

class MyLogger(logging.Logger):
    '''My own logger that stores the log messages into a string stream'''

    def __init__(self, name: str, filename: str|None = None, terminator = '\n'):
        '''Create a new logger instance with the given name'''
        logging.Logger.__init__(self, name, logging.DEBUG)

        self.stream = StringIO()
        handler = logging.StreamHandler(self.stream)
        handler.terminator = terminator
        handler.setFormatter(formatter)

        if filename is not None:
            self.addHandler(logging.FileHandler(filename, mode='w'))

        standard_output = logging.StreamHandler(sys.stderr)
        standard_output.terminator = terminator

        self.addHandler(handler)
        self.addHandler(standard_output)

    def getvalue(self) -> str:
        '''Returns the str that has been logged to this logger'''
        return self.stream.getvalue()

def is_newer(inputfile: str, outputfile: str, ignore=False) -> bool:
    '''Returns true iff the input file is newer than the output file'''
    if ignore:
        return True

    try:
        return os.path.getmtime(inputfile) > os.path.getmtime(outputfile)
    except OSError:
        return True

def run_program(cmds, logger):
    """ Runs the given program with sensible defaults, and logs the results to the logger """

    with subprocess.Popen(cmds,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.STDOUT,
                        text=True) as proc:
        for line in proc.stdout.read():
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
    '''To prepare the experiments we have to perform the following steps.'''

    # Ensure that tmp directory exists since the mCRL2 tools cannot make it
    try:
        os.mkdir(tmp_directory)
    except OSError:
        logger.debug(f'{tmp_directory} already exists')

    # Convert the mcrl2 to an aut file.
    base, _ = os.path.splitext(mcrl2_name)
    mcrl2_file = os.path.join(directory, mcrl2_name)
    lps_file = os.path.join(tmp_directory, base + '.lps')
    aut_file = os.path.join(tmp_directory, base + '.aut')

    mcrl22lps_exe = shutil.which('mcrl22lps')
    lps2lts_exe = shutil.which('lps2lts')

    if is_newer(mcrl2_file, lps_file):
        run_program([mcrl22lps_exe, '--verbose', mcrl2_file, lps_file], logger)
    if is_newer(lps_file, aut_file):
        run_program([lps2lts_exe, '--verbose', lps_file, aut_file], logger)

    # Convert the actions in the .aut files to move features from the data into the action label.
    # File contains from=to per line for each action.
    mapping = {}

    actionrename_file = os.path.join(directory, 'actionrename')
    aut_renamed_file = os.path.join(tmp_directory, base + '.renamed.aut')

    if is_newer(actionrename_file, aut_renamed_file) or is_newer(aut_file, aut_renamed_file):
        with open(actionrename_file, encoding='utf-8') as file:
            for line in file.readlines():
                result = mapping_regex.match(line)
                if result is not None:
                    mapping[result.group(1)] = result.group(2)

        logger.debug('renaming applied: %s', mapping)

        # Rename the action labels in the aut file based on the mapping computed above
        with open(aut_renamed_file, 'w', encoding='utf-8') as outfile:
            with open(aut_file, encoding='utf-8') as file:
                for line in file.readlines():
                    result = transition_regex.match(line)
                    if result is not None:
                        action = result.group(2)
                        action = mapping.get(action, action)
                        outfile.write(f'({result.group(1)},\"{action}\",{result.group(3)})\n')
                    else:
                        outfile.write(line)

    # Generate the SVPG for every property
    futures = {}
    games = []
    featurediagram_file = os.path.join(directory, 'FD')

    for prop in properties:
        mcf_file = os.path.join(directory, prop)
        prop, _ = os.path.splitext(prop)
        game_file = os.path.join(tmp_directory, prop + '.svpg')

        name = f'{os.path.basename(aut_file)} and {os.path.basename(mcf_file)}'
        games.append(game_file)
        if (
            is_newer(featurediagram_file, game_file)
            or is_newer(aut_renamed_file, game_file)
            or is_newer(mcf_file, game_file)
        ):
            logger.info(f'Generating parity game for {name}')

            # Java program do something weird with their logging
            program_logger = MyLogger(name, terminator='')
            futures[
                executor.submit(
                    run_program,
                    [
                        'java',
                        '-jar',
                        '-Xss100M',
                        '-Xmx6G',
                        FTSMMC_JAR,
                        'vpg',
                        featurediagram_file,
                        aut_renamed_file,
                        mcf_file,
                        game_file,
                    ],
                    program_logger
                )
            ] = (name, program_logger)

    return (futures, games)

def run_benchmark_single(tool: str, game: str, logger: MyLogger) -> float:
    '''Run a single benchmark and return the time it took in seconds'''
    tool_exe = shutil.which(tool)

    start = time.time()
    run_program([tool_exe, game], logger)
    return time.time() - start

def run_benchmark(
    tool: str,
    game: str,
    logger: MyLogger,
    executor: concurrent.futures.ThreadPoolExecutor,
):
    '''Run the benchmarks for a single experiment'''

    # Run several experiments and gather their average
    futures = {}
    for i in range(0, 5):
        logger.info('Started benchmark %s for %s with game %s', i, tool, os.path.basename(game))
        benchmark_logger = MyLogger(f'{i}-{tool}-{game}')
        futures[
            executor.submit(run_benchmark_single, tool, game, benchmark_logger)
        ] = benchmark_logger

    return futures

def main():
    '''The main function'''

    # Parse some configuration options
    parser = argparse.ArgumentParser(
        prog='run.py',
        description='Prepares the variability parity games and runs the solving experiments',
        epilog='',
    )

    parser.add_argument('-m', '--max_workers', action='store', default=1, type=int)

    args = parser.parse_args()

    experiments = [
        (
            '../cases/elevator/',
            'elevator.mcrl2',
            [
                'prop1.mcf',
                'prop2.mcf',
                'prop3.mcf',
                'prop4.mcf',
                'prop5.mcf',
                'prop6.mcf',
                'prop7.mcf',
            ],
        ),
        (
            '../cases/minepump/',
            'minepump_fts.mcrl2',
            [
                'phi1.mcf',
                'phi2.mcf',
                'phi3.mcf',
                'phi4.mcf',
                'phi5.mcf',
                'phi6.mcf',
                'phi7.mcf',
                'phi8.mcf',
                'phi9.mcf',
            ],
        ),
    ]

    logger = MyLogger('main', 'results.log')
    tools = ['VPGSolver_explicit', 'VPGSolver_bdd']

    # Prepare the variability parity games for all the properties and specifications.
    with concurrent.futures.ThreadPoolExecutor(
        max_workers=args.max_workers
    ) as executor:
        futures = {}
        for experiment in experiments:
            directory, mcrl2_name, properties = experiment

            # The directory in which to store all generated files
            tmp_directory = directory + 'tmp/'

            prepare_logger = MyLogger(f'prepare_{mcrl2_name}')

            logger.info('Starting preparation for experiment \'%s\'...', directory)
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
            ] = (directory, prepare_logger)

        benchmark_futures = {}
        for future in concurrent.futures.as_completed(futures):
            directory, prepare_logger = futures[future]

            try:
                prepare_futures, games = future.result()

                logger.info(prepare_logger.getvalue())

                # Wait for the parity games to be constructed.
                for future in concurrent.futures.as_completed(prepare_futures):
                    future.result()
                    name, logger = prepare_futures[future]
                    logger.info(logger.getvalue())
                    logger.info('Finished parity game for %s', name)

                logger.info('Finished preparation for experiment \'%s\'', directory)

                # Execute the solvers to measure the solving time.
                for game in games:
                    for tool in tools:
                        benchmark_logger = MyLogger(f'{directory}-{game}-{tool}')
                        print(f'Starting benchmarks for \'{os.path.basename(game)}\' and \'{tool}\'')
                        benchmark_futures[
                            executor.submit(run_benchmark, tool, game, benchmark_logger, executor)
                        ] = (directory, game, tool, benchmark_logger)

            except OSError as exp:
                logger.info(
                    'Preparation \'%s\' failed with exception %s', directory, exp
                )

        all_results = {}
        for future in concurrent.futures.as_completed(benchmark_futures):
            directory, game, tool, benchmark_logger = benchmark_futures[future]
            futures = future.result()
            
            results = []
            for future in concurrent.futures.as_completed(futures):
                measurement = future.result()
                benchmark_logger = futures[future]

                logger.info(benchmark_logger.getvalue())
                logger.info('Finished benchmark %s with game %s in %s', tool, os.path.basename(game), measurement)

                results.append({'timing': measurement})

            logger.info(benchmark_logger.getvalue())
            print(f'Finished benchmarking for experiment \'{game}\' and \'{tool}\'')

            # Construct the dictionary
            if directory not in all_results:
                all_results[directory] = {}

            if game not in all_results[directory]:
                all_results[directory][game] = {}

            all_results[directory][game][tool] = results

        # writing the dictionary data into the corresponding JSON file
        for directory in all_results:

            with open(os.path.join(directory, 'tmp/', 'results.json'), 'w', encoding='utf-8') as json_file:

                json.dump(all_results[directory], json_file)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        logging.error('Interrupted program')
