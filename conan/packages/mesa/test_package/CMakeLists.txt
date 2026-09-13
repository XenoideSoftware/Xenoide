cmake_minimum_required(VERSION 3.15)
project(test_mesa C)

find_package(mesa REQUIRED)

add_executable(test_gl test_gl.c)
target_link_libraries(test_gl PRIVATE mesa::OSMesa)
