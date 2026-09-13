cmake_minimum_required(VERSION 3.15)
project(test_mctrl C)

find_package(mctrl REQUIRED)

add_executable(test_mctrl main.c)
target_link_libraries(test_mctrl PRIVATE mctrl::mctrl)
