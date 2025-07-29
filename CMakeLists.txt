cmake_minimum_required(VERSION 3.14)
project(VariabilityParityGames)

option(ENABLE_BUDDY "Use Buddy for BDD operations instead of oxidd" ON)

add_subdirectory(implementation/VPGSolver/)