
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
    src/ProcessLauncher.cpp
    src/ProcfsReader.cpp
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


# Enable CTest.
enable_testing()

# Create the test executable.
add_executable(
    procfs_reader_tests
    tests/test_procfs_reader.cpp
    src/ProcfsReader.cpp
)

# Include directories for tests.
target_include_directories(
    procfs_reader_tests
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Use C++17.
set_target_properties(
    procfs_reader_tests
    PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Compiler warnings.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(
        procfs_reader_tests
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# Register the test.
add_test(
    NAME ProcfsReaderTests
    COMMAND procfs_reader_tests
)
