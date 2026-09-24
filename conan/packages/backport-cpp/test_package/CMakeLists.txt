cmake_minimum_required(VERSION 3.15)
project(test_backport CXX)

find_package(Backport REQUIRED)

add_executable(test_backport test_backport.cpp)
target_link_libraries(test_backport PRIVATE Backport::Backport)
