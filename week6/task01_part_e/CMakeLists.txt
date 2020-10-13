cmake_minimum_required(VERSION 3.17)
project(task01_part_e)

set(CMAKE_CXX_STANDARD 17)

add_executable(task01_part_e coords.cpp coords.h database.cpp database.h
        graph.h json.cpp json.h parse_input.cpp parse_input.h profile.h
        requests_input.h requests_read.cpp requests_read.h
        route_query_result.cpp route_query_result.h router.h
        routing_settings.h task01_part_e.cpp)