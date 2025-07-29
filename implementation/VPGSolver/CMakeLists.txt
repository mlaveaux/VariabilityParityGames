cmake_minimum_required(VERSION 3.14)
project(VPGSolver)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)

add_executable(VPGSolver_bdd 
  src/main.cpp
  src/Game.cpp 
  src/Game.h 
  src/zlnkPG.cpp 
  src/zlnkPG.h 
  src/zlnkVPG.cpp 
  src/zlnkVPG.h)


  if (ENABLE_BUDDY)
    add_subdirectory(3rd-party/BuDDy)
    target_link_libraries(VPGSolver_bdd PUBLIC buddy)
    add_compile_definitions(ENABLE_BUDDY)
  else()
    add_subdirectory(3rd-party/oxidd)
    target_link_libraries(VPGSolver_bdd PUBLIC oxidd)
  endif(ENABLE_BUDDY)
  
target_compile_definitions(VPGSolver_bdd PUBLIC subsetbdd)
target_include_directories(VPGSolver_bdd PUBLIC 3rd-party/BuDDy/src/ 3rd-party/boost-include-only)