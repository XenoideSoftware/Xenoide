cmake_minimum_required(VERSION 3.20)
project(test_hexctrl CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(HexCtrl REQUIRED)

add_executable(test_hexctrl main.cpp)
target_link_libraries(test_hexctrl PRIVATE HexCtrl::HexCtrl)
