cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++20 -pedantic")
project(idk CXX)

include_directories(./src)

# add_executable(idk
#     main.cpp
#     ./src/tokenizer.cpp
#     ./src/expressions.cpp
#     ./src/ast.cpp
#     ./src/json.hpp
#     ./src/codegen.cpp
#     )
add_executable(idk
    ./new/checker.cpp
    ./new/str_helper.cpp
    ./new/x86_64_helper.cpp
    ./new/x86_64_nasm_linux.cpp
    ./new/global.cpp    
    ./src/tokenizer.cpp
    ./new/expressions.cpp
    ./new/ast.cpp
    ./src/json.hpp
    #./src/codegen.cpp
    )