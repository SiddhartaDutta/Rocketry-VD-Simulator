cmake_minimum_required(VERSION 3.10)
project(RocketryVD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../header)
include_directories(/home/siddh/vcpkg/installed/x64-linux/include)

link_directories(/home/siddh/vcpkg/installed/x64-linux/lib)

add_executable(test ../src/main.cpp ../src/trajectory.cpp)

# Link directly to static libraries
target_link_libraries(test PRIVATE arrow parquet)
