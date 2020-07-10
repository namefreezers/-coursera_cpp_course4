cmake_minimum_required(VERSION 3.16)
project(task06_zoo)

set(CMAKE_CXX_STANDARD 17)

add_executable(task06_zoo animals.h test_runner.h zoo.cpp)