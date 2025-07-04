cmake_minimum_required(VERSION 3.21)
project(RocketryVD)

set(CMAKE_CXX_STANDARD 17)

# Conan-generated toolchain must be included early
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL)

# Find Arrow and Parquet provided by Conan
find_package(Arrow REQUIRED CONFIG)
find_package(Parquet REQUIRED CONFIG)

add_executable(test
    src/main.cpp
    src/trajectory.cpp
)

target_link_libraries(test PRIVATE Arrow::arrow_shared Parquet::parquet_shared)

# Add your headers directory
target_include_directories(test PRIVATE ${CMAKE_SOURCE_DIR}/header)
