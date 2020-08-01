cmake_minimum_required(VERSION 3.16)
project(task02_cache)

set(CMAKE_CXX_STANDARD 17)

add_executable(task02_cache Common.h main.cpp Solution.cpp test_runner.h)