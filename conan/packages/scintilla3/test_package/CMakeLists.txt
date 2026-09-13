cmake_minimum_required(VERSION 3.15)
project(scintilla3_test_package CXX)

find_package(scintilla3 REQUIRED CONFIG)

add_executable(test_package test_package.cpp)
target_link_libraries(test_package PRIVATE scintilla3)
