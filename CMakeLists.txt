
cmake_minimum_required(VERSION 3.16)

project(
    LinuxPerformanceProfiler
    VERSION 0.1.0
    DESCRIPTION "Linux process and system performance profiler"
    LANGUAGES CXX
)

# Main executable.
add_executable(
    profiler
    src/main.cpp
)

# Include directory.
target_include_directories(
    profiler
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Use C++17.
set_target_properties(
    profiler
    PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Compiler warnings.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(
        profiler
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()
