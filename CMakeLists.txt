cmake_minimum_required(VERSION 3.28.0)
project(neon)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-O3 -g")

add_executable(neon main.cpp)