cmake_minimum_required (VERSION 3.16)

if (NOT XE_BUILD_VERSION)
    set (XE_BUILD_VERSION_DEFAULT "0.0.0")
    message (WARNING "Xenoide: XE_BUILD_VERSION not set, defaulting to ${XE_BUILD_VERSION_DEFAULT}")
    set(XE_BUILD_VERSION "${XE_BUILD_VERSION_DEFAULT}")
endif()

if (NOT XE_BUILD_COMMIT)
    set(XE_BUILD_COMMIT_DEFAULT "local")
    message (WARNING "Xenoide: XE_BUILD_COMMIT not set, defaulting to '${XE_BUILD_COMMIT_DEFAULT}'")
    set(XE_BUILD_COMMIT "${XE_BUILD_COMMIT_DEFAULT}")
endif()

if (NOT XE_BUILD_TIMESTAMP)
    set(XE_BUILD_TIMESTAMP_DEFAULT "now")
    message (WARNING "Xenoide: XE_BUILD_TIMESTAMP not set, defaulting to '${XE_BUILD_TIMESTAMP_DEFAULT}'")
    set(XE_BUILD_TIMESTAMP "${XE_BUILD_TIMESTAMP_DEFAULT}")
endif()

project (xenoide VERSION ${XE_BUILD_VERSION})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakePredefinedTargets")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(TargetFolders)

set (CMAKE_EXPORT_COMPILE_COMMANDS  ON)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

option (XE_ENABLE_ENGINE "Build with engine" ON)
option (XE_ENABLE_IDE "Build with IDE" ON)

# options suited for development
option (XE_ENABLE_TESTING       "Enable unit test suite" ON)
option (XE_ENABLE_WERROR        "Marks all warnings as errors" ON)
option (XE_ENABLE_DOCS          "Generate Doxygen documentation for the engine libraries" ON)
option (XE_ENABLE_PCH           "Enable precompiled headers for internal libraries, to improve build times" OFF)
option (XE_ENABLE_COVERAGE      "Instrument code for code coverage reporting during unit tests" OFF)

# Which sanitizer to use
set (XE_SANITIZER "" CACHE STRING "")

if (XE_ENABLE_TESTING)
    set(CMAKE_FOLDER "CMakePredefinedTargets")
    include(CTest)
    enable_testing()
    find_package(Catch2 3 CONFIG REQUIRED)
    unset(CMAKE_FOLDER)
endif()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions (-D_DEBUG)
endif ()

add_subdirectory(src)

if (TARGET xenoidew)
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT xenoidew)
endif()
