cmake_minimum_required(VERSION 3.14)
project(VPGSolver)

add_subdirectory(3rd-party/BuDDy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/)

add_executable(VPGSolver_bdd 
  main.cpp
  Game.cpp 
  Game.h 
  Algorithms/zlnkPG.cpp 
  Algorithms/zlnkPG.h 
  Algorithms/zlnkVPG.cpp 
  Algorithms/zlnkVPG.h 
  conf.h)

target_compile_definitions(VPGSolver_bdd PUBLIC subsetbdd)
target_link_libraries(VPGSolver_bdd PUBLIC bdd)
target_include_directories(VPGSolver_bdd PUBLIC 3rd-party/BuDDy/src/ 3rd-party/boost-include-only)