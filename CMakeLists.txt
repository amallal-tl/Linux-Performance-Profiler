
cmake_minimum_required(VERSION 3.16)

project(
    LinuxPerformanceProfiler
    VERSION 0.1.0
    DESCRIPTION "Linux process and system performance profiler"
    LANGUAGES CXX
)

# --------------------------------------------------
# Core library
# --------------------------------------------------

add_library(
    profiler_core
    src/ProcessLauncher.cpp
    src/ProcfsReader.cpp
    src/ProcessMonitor.cpp
)

target_include_directories(
    profiler_core
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(
    profiler_core
    PUBLIC
    cxx_std_17
)

set_target_properties(
    profiler_core
    PROPERTIES
    CXX_EXTENSIONS OFF
)

# --------------------------------------------------
# Compiler warnings
# --------------------------------------------------

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(
        profiler_core
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# --------------------------------------------------
# Main application
# --------------------------------------------------

add_executable(
    profiler
    src/main.cpp
)

target_link_libraries(
    profiler
    PRIVATE
    profiler_core
)

# --------------------------------------------------
# Tests
# --------------------------------------------------

enable_testing()

add_executable(
    procfs_reader_tests
    tests/test_procfs_reader.cpp
)

target_link_libraries(
    procfs_reader_tests
    PRIVATE
    profiler_core
)

add_test(
    NAME ProcfsReaderTests
    COMMAND procfs_reader_tests
)

# ProcessMonitor tests

add_executable(
    process_monitor_tests
    tests/test_process_monitor.cpp
)

target_link_libraries(
    process_monitor_tests
    PRIVATE
    profiler_core
)

add_test(
    NAME ProcessMonitorTests
    COMMAND process_monitor_tests
)