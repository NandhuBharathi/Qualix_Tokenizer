
cmake_minimum_required(VERSION 3.21)

project(
    QualixTokenizer
    VERSION 0.1.0
    DESCRIPTION "High-performance Unicode-first tokenizer library"
    LANGUAGES CXX
)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(QUALIX_BUILD_TESTS "Build unit tests" ON)
option(QUALIX_BUILD_EXAMPLES "Build examples" ON)
option(QUALIX_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(QUALIX_BUILD_TOOLS "Build command-line tools" ON)

include(GNUInstallDirs)

add_subdirectory(src)

if(QUALIX_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(QUALIX_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(QUALIX_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(QUALIX_BUILD_TOOLS)
    add_subdirectory(tools)
endif()
