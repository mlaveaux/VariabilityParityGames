cmake_minimum_required(VERSION 3.14)
project(createSUVPG)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ../../bin/)

add_executable(createSUVPG main.cpp)