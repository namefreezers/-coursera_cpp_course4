cmake_minimum_required(VERSION 3.17)
project(task04_booking_manager)

set(CMAKE_CXX_STANDARD 17)

add_executable(task04_booking_manager new_booking_providers.h new_trip_manager.h old_booking_providers.h old_trip_manager.h test.cpp test_runner.h booking.h)