cmake_minimum_required(VERSION 3.15)
project(test_win32xx CXX)

find_package(win32xx REQUIRED)

add_executable(test_win32xx WIN32 main.cpp)
target_compile_definitions(test_win32xx PRIVATE WINVER=0x0600 _WIN32_WINNT=0x0600)
target_link_libraries(test_win32xx PRIVATE win32xx::win32xx)
