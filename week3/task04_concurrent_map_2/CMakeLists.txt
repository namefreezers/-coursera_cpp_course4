cmake_minimum_required(VERSION 3.16)
project(task04_concurrent_map_2)

set(CMAKE_CXX_STANDARD 17)

add_executable(task04_concurrent_map_2 concurrent_map_2.cpp profile.h test_runner.h)