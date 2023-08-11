#!/usr/bin/env python

from io import StringIO
import os
import re
import subprocess
import concurrent.futures
import argparse
import logging
import sys

from typing import List

formatter = logging.Formatter('%(threadName)-11s %(asctime)s %(levelname)s %(message)s')
logging.basicConfig(level=logging.DEBUG)

# This is just for testing, setting the PATH to find the executable.
os.environ["PATH"] += os.pathsep + '../../mCRL2-build/stage/bin/RelWithDebInfo/'
os.environ["PATH"] += os.pathsep + '../build/RelWithDebInfo'

# A regex matching in=out
mapping_regex = re.compile(r'(.*)=(.*)')

# A regex matching a transition in the aut format '(from, action, to)'
transition_regex = re.compile(r'\(([0-9]*),\"(.*)\",([0-9]*)\)')

class MyLogger(logging.Logger):
    """ My own logger that stores the log messages into a string stream """

    def __init__(self, name: str):
        """ Create a new logger instance with the given name """ 
        logging.Logger.__init__(self, name)

        self.stream = StringIO()
        handler = logging.StreamHandler(self.stream)
        handler.setFormatter(formatter)

        standard_output = logging.StreamHandler(sys.stderr)

        logger = logging.getLogger(name)
        logger.setLevel(logging.DEBUG)
        logger.addHandler(handler)
        logger.addHandler(standard_output)
    
    def getvalue(self) -> str:
        """ Returns the str that has been logged to this logger """
        return self.stream.getvalue()


def is_newer(inputfile: str, outputfile: str, ignore=False) -> bool:
    """ Returns true iff the input file is newer than the output file """
    if ignore:
        return True

    try:
        return os.path.getmtime(inputfile) > os.path.getmtime(outputfile)
    except OSError:
        return True

def prepare(directory: str, tmp_directory: str, mcrl2_name: str, properties: List[str], logger: MyLogger, executor: concurrent.futures.ThreadPoolExecutor) -> List[str]:
    """ To prepare the experiments we have to perform the following steps. """

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

    if is_newer(mcrl2_file, lps_file):
        subprocess.call(['mcrl22lps', '--verbose', mcrl2_file, lps_file])
    if is_newer(lps_file, aut_file):
        subprocess.call(['lps2lts', '--verbose', lps_file, aut_file])
    
    # Convert the actions in the .aut files to move features from the data into the action label.
    # File contains from=to per line for each action.
    mapping = {}
    with open(os.path.join(directory, 'actionrename'), encoding='utf-8') as file:
        for line in file.readlines():
            result = mapping_regex.match(line)
            if result is not None:
                mapping[result.group(1)] = result.group(2)

    #logger.debug(mapping)

    # Rename the action labels in the aut file based on the mapping computed above
    aut_renamed_file = os.path.join(tmp_directory, base + '.renamed.aut')
    with open(aut_renamed_file, 'w', encoding='utf-8') as outfile:
        with open(aut_file, encoding='utf-8') as file:
            for line in file.readlines():
                result = transition_regex.match(line)
                if result is not None:
                    action = result.group(2)
                    action = mapping.get(action, action)
                    outfile.write(f'({result.group(1)},"{action}",{result.group(3)})\n')
                else:
                    outfile.write(line)

    # Generate the SVPG for every property
    futures = {}
    games = []
    ftsmmc_jar = '../implementation/FTSMMC/app/build/libs/app.jar'
    featurediagram_file = os.path.join(directory, 'FD')

    for prop in properties:
        mcf_file = os.path.join(directory, prop)
        prop, _ = os.path.splitext(prop)
        game_file = os.path.join(tmp_directory, prop + '.svpg')
        
        name = f'{mcrl2_file}:{prop}'
        games.append(game_file)
        if is_newer(aut_renamed_file, game_file):
            logger.info(f'Generating parity game for {name}')
            futures[executor.submit(subprocess.call, ['java', '-jar', '-Xss100M', '-Xmx6G', ftsmmc_jar, 'vpg', 
                                                      featurediagram_file,
                                                      aut_renamed_file,
                                                      mcf_file,
                                                      game_file])] = name

    for future in concurrent.futures.as_completed(futures):
        logger.info(f'Finished parity game for {futures[future]}')

    return games

def run_benchmark(tool: str, game: str, executor: concurrent.futures.ThreadPoolExecutor):
    """ Run the benchmarks for a single experiment """

    # Run several experiments and gather their average
    futures = []
    for i in range(0, 5):
        print(f'Started benchmark {i} for {tool} with game {game}')
        futures.append(executor.submit(subprocess.call, [tool, game]))

    for future in concurrent.futures.as_completed(futures):
        print(f'Finished benchmark {tool} with game {game}')



def main():
    """ The main function """
    
    # Parse some configuration options
    parser = argparse.ArgumentParser(
        prog='run.py',
        description='Prepares the variability parity games and runs the solving experiments',
        epilog=''
    )

    parser.add_argument('-m', '--max_workers', action='store', default=1)

    args = parser.parse_args()

    experiments = [('../cases/elevator/', 'elevator.mcrl2',
                        ['prop1.mcf',
                         'prop2.mcf',
                         'prop3.mcf',
                         'prop4.mcf',
                         'prop5.mcf',
                         'prop6.mcf',
                         'prop7.mcf']),
                  ]
    
    tools = [
        'VPGSolver_explicit',
        'VPGSolver_bdd'
    ]

    # Prepare the variability parity games for all the properties and specifications.
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.max_workers) as executor:

        futures = {}
        for experiment in experiments:
            directory, mcrl2_name, properties = experiment

            # The directory in which to store all generated files
            tmp_directory = directory + 'tmp/'

            logger = MyLogger(f'prepare_{mcrl2_name}')

            logging.info(f'Starting preparation for experiment \'{directory}\'...')
            futures[executor.submit(prepare, 
                                    directory, 
                                    tmp_directory,
                                    mcrl2_name, 
                                    properties,
                                    logger,
                                    executor)] = (directory, logger)

        future_benchmarks = {}
        for future in concurrent.futures.as_completed(futures):
            directory, logger = futures[future]

            try:
                games = future.result()

                logging.info(f'Finished preparation for experiment \'{directory}\'')
                logging.info(logger.getvalue())

                # Execute the solvers to measure the solving time.
            #    for tool in tools:
            #        for game in games:
            #            name = f'tool: {tool} and game: {game}'
            #            print(f'Starting benchmarks for \'{name}\'')
            #            future_benchmarks[executor.submit(run_benchmark, tool, game, executor)] = name

            except OSError as exp:
                logging.info(f'Preparation \'{directory}\' failed with exception {exp}')

                
        #for future in concurrent.futures.as_completed(future_benchmarks):
        #    name = future_benchmarks[future]
        #    print(f'Finished preparation for experiment \'{name}\'')

if __name__ == "__main__":

    try:
        main()
    except KeyboardInterrupt:
        logging.error('Interrupted program')
