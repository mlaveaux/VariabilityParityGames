# Variability Parity Games

Welcome to the variability parity game repository. This repository contains the thesis "Verifying SPLs using parity games expressing variability" by me (Sjef van Loo) and supervised by T.A.C. Willemse.

The algorithms and data structures used to solve the VPGs are implemented neatly. The other parts of the implementation (parsing VPGs, printing solutions and the tool to create VPGs) are implemented trivially, not optimized and the code is not up to standards.

# Experiments

The measurements are taken from the experiments in the cases/ directory. These contain mCRL2 specifications and properties from which the parity games are generated.

# Solver implementation

The implementation/VPGSolver implements several parity game solvers in CPP
  * VPGSolver_bdd implements the VPG algorithms using bdds to represent sets of configurations
  * VPGSolver_explicit implements the VPG algorithms using explicit sets to represent sets of configurations
  * VPGSolver_bdd_single implements the parity game algorithms, it can either interpretate a parity game or a VPG and solve it for a single configuration. For parsing the VPGs bdds are used
  * VPGlambameasure measures the average relative set size of VPGs
