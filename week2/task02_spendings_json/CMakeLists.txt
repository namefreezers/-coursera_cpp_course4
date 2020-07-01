cmake_minimum_required(VERSION 3.16)
project(task02_spendings_json)

set(CMAKE_CXX_STANDARD 17)

add_executable(task02_spendings_json json.cpp json.h spendings_json.cpp test_runner.h)