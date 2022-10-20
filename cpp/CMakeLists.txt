cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++20 -pedantic")
project(nava CXX)

include_directories(./src)

add_executable(nava
    ./main.cpp    
    ./src/debug.cpp
    ./src/checker.cpp
    ./src/str_helper.cpp
    ./src/x86_64_helper.cpp
    ./src/x86_64_nasm_linux.cpp
    ./src/tokenizer.cpp
    ./src/expressions.cpp
    ./src/ast.cpp
    ./src/log.cpp
    ./src/json.hpp
    ./src/argparser.hpp
    )