# Variability Parity Games

This repository contains the experiments for the thesis "Verifying SPLs using parity games expressing variability" by Sjef van Loo and supervised by T.A.C. Willemse. These experiments consist of comparing product based and family based solving of variability parity games.

# Building

The experiments require two different tools. First of all, a C++ program called
VPGSolver that performs the actual solving and a Java program called FTSMMC to
generate the benchmark set. To acquire the 3rd-party dependencies we need to
download the git submodules.

```
git submodule update --init --recursive
```

Next, we need to install the dependencies, which are the following:

  - Java JDK
  - [Gradle](https://gradle.org/)
  - A C++17 compiler, tested using MSVC.
  - [CMake 3.14](https://cmake.org/)
  - [Python 3](https://www.python.org/)

For the VPGSolver we set up a build in the `build/` directory using the standard CMake setup. The Java program can be compiled using `gradle build` in the FTSMMC directory, or `gradle.cmd` on Windows.

# Experiments

The experiments are contained in the `cases/` directory, and a Python script `experiments/run.py` is used to generate the benchmark parity games from the given mCRL2 specification and properties, and several auxiliary files `FD` and `actionrename` to define the configuration space (Feature Diagram) and rename action labels to avoid data parameters. Given the above setup the `experiments/run.py` should take care of performing all the experiments and generating a JSON file containing the resulting measurements.

# VPGSolver

The implementation/VPGSolver implements several parity game solvers in C++
  * VPGSolver_bdd implements the VPG algorithms using bdds to represent sets of configurations
  * VPGSolver_explicit implements the VPG algorithms using explicit sets to represent sets of configurations
  * VPGSolver_bdd_single implements the parity game algorithms, it can either interpretate a parity game or a VPG and solve it for a single configuration. For parsing the VPGs bdds are used
  * VPGlambameasure measures the average relative set size of VPGs

# FTSMMC

The `FTSMMC` tool is used to project product parity games from a variability parity games, as well as to compute a variability parity game from a labelled transition system in `.aut` format and a modal mu-calculus specification.