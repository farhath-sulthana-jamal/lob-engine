cmake_minimum_required(VERSION 3.28)

project(lob-engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warning flags in one place. Any target that links this gets them.
add_library(lob_warnings INTERFACE)

target_compile_options(lob_warnings INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
)

# Header-only library: compiles nothing itself, only carries settings to whoever links it.
add_library(lob INTERFACE)

target_include_directories(lob INTERFACE
    include
)

add_executable(lob-engine
    src/main.cpp
)

target_link_libraries(lob-engine PRIVATE
    lob
    lob_warnings
)

# ---- Tests ----
enable_testing()

find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(lob-tests
    tests/types_test.cpp
)

target_link_libraries(lob-tests PRIVATE
    lob
    lob_warnings
    GTest::gtest_main
)

gtest_discover_tests(lob-tests)
