# Variability Parity Games
Welcome to the variability parity game repository. This repository contains the thesis "Verifying SPLs using parity games expressing variability" by me (Sjef van Loo) and supervised by T.A.C. Willemse.

This repository includes:
 * The thesis including its latex source.
 * The binaries and C++ source code of the algorithms presented in the thesis.
 * Sets of VPGs used to experiment with.
 * A tool to create VPGs from mcrl2 encoded FTSs and mu-calculus formulas (including the java source code).

All the experimental results presented in the thesis are obtained using the binaries and VPGs in this repository.

The algorithms and data structures used to solve the VPGs are implemented neatly. The other parts of the implementation (parsing VPGs, printing solutions and the tool to create VPGs) are implemented trivially, not optimized and the code is not up to standards.

## Structure
* Report/ contains the thesis (Report.pdf) and its latex source
* implementation/ contains the implementation and experiments
  * FTSMMC/ contains the source code of the toolset to create VPGs
  * VPGSolver/ contains the source code of the VPG solving binaries
  * bin/ contains the binaries
  * testsuite/ contains the scripts and VPGs used for experimental evaluation
  * VPG-Collection.tar.gz contains the VPGs of all the experiments. Most of these VPGs can also be found in testsuite, however for convenience they are collected in this single file
  * Elevator.tar.gz contains the mcrl2 implementation of the Elevator example used in the experiments

## Binaries
The toolset to create VPGs is build in a single binary: FTSMMC.jar.

The VPGSolver project creates 4 binaries:
* VPGSolver_bdd implements the VPG algorithms using bdds to represent sets of configurations
* VPGSolver_explicit implements the VPG algorithms using explicit sets to represent sets of configurations
* VPGSolver_bdd_single implements the parity game algorithms, it can either interpretate a parity game or a VPG and solve it for a single configuration. For parsing the VPGs bdds are used
* VPGlambameasure measures the average relative set size of VPGs

## Working names
Sometimes names in the code, scripts and folders differ from the names in the thesis.
* MBR is synonymous with the incremental pre-solve algorithm.
* FF randomgames are type 1 random games that scale in lambda.
* FC randomgames are type 2 random games that scale in lambda.
* BC randomgames are type 3 random games that scale in lambda.
* randomscalegames are type 1 random games that scale in the number of features.
