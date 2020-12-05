cmake_minimum_required(VERSION 3.17)
project(MIPS_pipeline)

set(CMAKE_CXX_STANDARD 17)

add_executable(code cachesimulator.cpp)
add_custom_target(
        remove-files
        rm -f ${CMAKE_SOURCE_DIR}/trace.txt.out
)

add_dependencies(code remove-files)
