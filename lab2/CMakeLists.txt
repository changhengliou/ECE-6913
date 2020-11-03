cmake_minimum_required(VERSION 3.17)
project(MIPS_pipeline)

set(CMAKE_CXX_STANDARD 17)

add_executable(code MIPS_pipeline.cpp)
add_custom_target(
        remove-files
        rm -f ${CMAKE_CURRENT_BINARY_DIR}/*result.txt
)

add_dependencies(code remove-files)
