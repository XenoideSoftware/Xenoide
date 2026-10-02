cmake_minimum_required(VERSION 3.15)
project(test_angle C)

find_package(angle REQUIRED)

add_executable(test_angle test_angle.c)
target_link_libraries(test_angle PRIVATE angle::EGL angle::GLESv2)
