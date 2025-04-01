cmake_minimum_required(VERSION 3.10)

project(RocketryVD)

include_directories(../header)
add_executable(test ../src/main.cpp ../src/trajectory.cpp)