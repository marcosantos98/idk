cmake_minimum_required(VERSION 3.20)

project(idk CXX)

set(CMAKE_CXX_COMPILER ${gcc})
set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++20 -pedantic")

include_directories(./src)

add_executable(idk
    main.cpp
    ./src/tokenizer.cpp
    )