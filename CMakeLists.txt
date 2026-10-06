cmake_minimum_required(VERSION 3.28)

project(lob-engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(lob-engine
    src/main.cpp
)

target_include_directories(lob-engine PRIVATE
    include
)

target_compile_options(lob-engine PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)