cmake_minimum_required(VERSION 3.14)
project(VPGSolver)

add_subdirectory(3rd-party/BuDDy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/)

add_executable(VPGSolver_explicit 
  main.cpp 
  Game.cpp 
  Game.h 
  Algorithms/zlnkVPG.cpp 
  Algorithms/zlnkVPG.h 
  conf.h 
  Algorithms/Datastructures/ConfSetExplicit.cpp 
  Algorithms/Datastructures/ConfSetExplicit.h 
  Algorithms/FPIte.cpp 
  Algorithms/FPIte.h
  Algorithms/MBR.cpp 
  Algorithms/MBR.h 
  Algorithms/Datastructures/UnorderedVertexSet.cpp 
  Algorithms/Datastructures/UnorderedVertexSet.h)

target_compile_definitions(VPGSolver_explicit PUBLIC subsetexplicit)
target_include_directories(VPGSolver_explicit PUBLIC 3rd-party/boost-include-only)

add_executable(VPGSolver_bdd 
  main.cpp
  Game.cpp 
  Game.h 
  Algorithms/zlnkVPG.cpp 
  Algorithms/zlnkVPG.h 
  conf.h 
  Algorithms/Datastructures/ConfSetExplicit.cpp 
  Algorithms/Datastructures/ConfSetExplicit.h
  Algorithms/FPIte.cpp 
  Algorithms/FPIte.h 
  Algorithms/MBR.cpp 
  Algorithms/MBR.h 
  Algorithms/Datastructures/bintree.cpp 
  Algorithms/Datastructures/bintree.h
  Algorithms/Datastructures/UnorderedVertexSet.cpp 
  Algorithms/Datastructures/UnorderedVertexSet.h)

target_compile_definitions(VPGSolver_bdd PUBLIC subsetbdd)
target_link_libraries(VPGSolver_bdd PUBLIC bdd)
target_include_directories(VPGSolver_bdd PUBLIC 3rd-party/BuDDy/src/ 3rd-party/boost-include-only)

add_executable(VPGSolver_bdd_single 
  main.cpp 
  Game.cpp 
  Game.h 
  conf.h 
  Algorithms/Datastructures/ConfSetExplicit.cpp 
  Algorithms/Datastructures/ConfSetExplicit.h
  Algorithms/FPIte.cpp 
  Algorithms/FPIte.h 
  Algorithms/MBR.h 
  Algorithms/Datastructures/UnorderedVertexSet.cpp 
  Algorithms/Datastructures/UnorderedVertexSet.h 
  Algorithms/zlnkPG.cpp
  Algorithms/zlnkPG.h)

target_compile_definitions(VPGSolver_bdd_single  PUBLIC subsetbdd SINGLEMODE)
target_link_libraries(VPGSolver_bdd_single PUBLIC bdd)
target_include_directories(VPGSolver_bdd_single PUBLIC 3rd-party/BuDDy/src/ 3rd-party/boost-include-only)

add_executable(VPGlambdameasure
  main.cpp 
  Game.cpp 
  Game.h 
  Algorithms/zlnkVPG.cpp 
  Algorithms/zlnkVPG.h 
  conf.h 
  Algorithms/Datastructures/ConfSetExplicit.cpp 
  Algorithms/Datastructures/ConfSetExplicit.h 
  Algorithms/FPIte.cpp 
  Algorithms/FPIte.h 
  Algorithms/MBR.cpp 
  Algorithms/MBR.h 
  Algorithms/Datastructures/UnorderedVertexSet.cpp 
  Algorithms/Datastructures/UnorderedVertexSet.h 
  Algorithms/Datastructures/bool_reference.cpp
  Algorithms/Datastructures/bool_reference.h)

target_compile_definitions(VPGlambdameasure PUBLIC subsetbdd lambdameasure)
target_link_libraries(VPGlambdameasure PUBLIC bdd)
target_include_directories(VPGlambdameasure PUBLIC 3rd-party/BuDDy/src/ 3rd-party/boost-include-only)