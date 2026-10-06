cmake_minimum_required(VERSION 3.28)

project(lob-engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

add_executable(lob-engine
    src/main.cpp
)

target_include_directories(lob-engine PRIVATE
    include
)