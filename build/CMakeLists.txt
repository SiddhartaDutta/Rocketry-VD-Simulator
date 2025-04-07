cmake_minimum_required(VERSION 3.10)
project(RocketryVD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../header)
add_executable(test ../src/main.cpp ../src/trajectory.cpp)