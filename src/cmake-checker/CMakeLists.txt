cmake_minimum_required (VERSION 3.25)

list(APPEND CMAKE_MODULE_PATH "../cmake")

include(XEVersion)
project (cmake-checker VERSION ${XE_BUILD_VERSION})
include(XETesting)
include(XEBaseTarget)

option (XE_CMAKE_CHECKER_WITH_TESTS "Build the cmake-checker unit test suite" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(cxxopts CONFIG REQUIRED)

add_subdirectory("src")
