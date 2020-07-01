cmake_minimum_required(VERSION 3.16)
project(task01_spendings_xml)

set(CMAKE_CXX_STANDARD 17)

add_executable(task01_spendings_xml spendings_xml.cpp test_runner.h xml.cpp xml.h)