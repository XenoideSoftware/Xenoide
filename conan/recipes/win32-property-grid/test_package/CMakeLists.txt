cmake_minimum_required(VERSION 3.15)
project(test_win32_property_grid C)

find_package(win32-property-grid REQUIRED)

add_executable(test_win32_property_grid main.c)
target_link_libraries(test_win32_property_grid PRIVATE win32-property-grid::win32-property-grid)
