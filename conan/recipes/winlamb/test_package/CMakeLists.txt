cmake_minimum_required(VERSION 3.15)
project(test_winlamb CXX)

find_package(winlamb REQUIRED)

add_executable(test_winlamb WIN32 main.cpp)
target_compile_definitions(test_winlamb PRIVATE WINVER=0x0600 _WIN32_WINNT=0x0600)
target_link_libraries(test_winlamb PRIVATE winlamb::winlamb winlamb::manifest)
