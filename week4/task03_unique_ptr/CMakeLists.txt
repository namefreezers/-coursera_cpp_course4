cmake_minimum_required(VERSION 3.17)
project(task03_unique_ptr)

set(CMAKE_CXX_STANDARD 17)

add_executable(task03_unique_ptr test_runner.h unique_ptr.cpp)