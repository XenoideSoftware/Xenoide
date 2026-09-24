cmake_minimum_required(VERSION 3.15)
project(spdlog_test CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(spdlog REQUIRED CONFIG)

add_executable(test_spdlog main.cpp)
target_link_libraries(test_spdlog PRIVATE spdlog::spdlog)

# Bundled fmt (10+) static_asserts that consumers compile with /utf-8 on
# MSVC; the IDE codepage default otherwise rejects the Unicode literals
# inside fmt's headers.
if(MSVC)
    target_compile_options(test_spdlog PRIVATE /utf-8)
endif()
