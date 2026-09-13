cmake_minimum_required (VERSION 3.16)

project (glaze-cpp
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "C++ port of the Glaze OpenGL binding generator"
)

option (GLAZE_CPP_WITH_TESTS         "Build the Catch2 unit test suite" OFF)
option (GLAZE_CPP_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

set (CMAKE_CXX_STANDARD             17)
set (CMAKE_CXX_STANDARD_REQUIRED    ON)
set (CMAKE_CXX_EXTENSIONS           OFF)
set (CMAKE_EXPORT_COMPILE_COMMANDS  ON)

# Flatten binary output so the conanfile.py recipe can locate glaze with
# a single well-known path across single- and multi-config generators.
if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

# Self-contained interface library so the inner libglaze-* CMakeLists
# can link `glaze::interface` for shared warning/compile flags. The
# alias is the only name the subtree sees.
set (target glaze-cpp-interface)
add_library(${target} INTERFACE)
add_library(glaze::interface ALIAS ${target})

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${target} INTERFACE -Wall -Wextra)
    if (GLAZE_CPP_WARNINGS_AS_ERRORS)
        target_compile_options(${target} INTERFACE -Werror)
    endif()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${target} INTERFACE /W4)
    if (GLAZE_CPP_WARNINGS_AS_ERRORS)
        target_compile_options(${target} INTERFACE /WX)
    endif()
endif()

target_compile_definitions(${target} INTERFACE _CRT_SECURE_NO_WARNINGS)

if (GLAZE_CPP_WITH_TESTS)
    include(CTest)
    enable_testing()
    find_package(Catch2 3 CONFIG REQUIRED)
endif()

add_subdirectory (src)
