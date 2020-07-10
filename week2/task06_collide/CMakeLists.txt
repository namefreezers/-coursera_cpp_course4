cmake_minimum_required(VERSION 3.16)
project(task06_collide)

set(CMAKE_CXX_STANDARD 17)

add_executable(task06_collide collide.cpp game_object.h geo2d.cpp geo2d.cpp test_runner.h)