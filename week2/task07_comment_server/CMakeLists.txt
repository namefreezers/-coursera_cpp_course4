cmake_minimum_required(VERSION 3.16)
project(task07_comment_server)

set(CMAKE_CXX_STANDARD 17)

add_executable(task07_comment_server comment_server.cpp test_runner.h)