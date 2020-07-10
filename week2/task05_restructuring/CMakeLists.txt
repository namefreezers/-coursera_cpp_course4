cmake_minimum_required(VERSION 3.16)
project(task05_restructuring)

set(CMAKE_CXX_STANDARD 17)

add_executable(task05_restructuring main.cpp stats_aggregator.cpp stats_aggregator.h stats_aggregator_test.cpp test_runner.h)