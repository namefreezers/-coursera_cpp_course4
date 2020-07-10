cmake_minimum_required(VERSION 3.16)
project(task04_refactoring)

set(CMAKE_CXX_STANDARD 17)

add_executable(task04_refactoring json.cpp json.h refactoring.cpp test_runner.h xml.cpp xml.h)