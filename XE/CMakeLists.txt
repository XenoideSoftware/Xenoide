cmake_minimum_required (VERSION 3.16)

project (XE)

option (XE_PLUGIN_PNG           "Include PNG image support via LodePNG" ON)
option (XE_PLUGIN_GL            "Include OpenGL 3+ support" ON)
option (XE_PLUGIN_GL_GLFW       "Include GLFW-based Window and Context creation support (requires 'XE_PLUGIN_GL')" ON)
option (XE_PLUGIN_KTX           "Include KTX image support" ON)

# options suited for development
option (XE_DEV_UNIT_TEST            "Enable unit test suite" ON)
option (XE_DEV_WARNINGS_AS_ERRORS   "Marks all warnings as errors" OFF)
option (XE_DEV_INSTRUMENT_COVERAGE  "Instrument code for code coverage reporting during unit tests" OFF)
option (XE_DEV_PRECOMPILED_HEADERS  "Enable precompiled headers for internal libraries, to improve build times" OFF)
option (XE_DEV_SANITIZER_ENABLE     "Enable sanitizer usage" OFF)
option (XE_DEV_DOCS                 "Generate Doxygen documentation for the engine libraries" OFF)

# Which sanitizer to use
set (XE_DEV_SANITIZER "" CACHE STRING "")

# Options for external assets location
if (NOT XE_EXTERNAL_ASSET_ROOT_PATH)
    message(WARNING "XE_EXTERNAL_ASSET_ROOT_FOLDER variable not defined. Runtime errors might arise when running certain targets")
    set (XE_EXTERNAL_ASSET_ROOT_PATH "")
endif()

set (CMAKE_EXPORT_COMPILE_COMMANDS  ON)
set (CMAKE_CXX_STANDARD             17)
set (CMAKE_CXX_STANDARD_REQUIRED    ON)
set (CMAKE_CXX_EXTENSIONS           OFF)

# TODO: Configure the version numbering mechanism externally
set (XE_VERSION_MAJOR 1)
set (XE_VERSION_MINOR 0)
set (XE_VERSION_REVISION 0)
set (XE_VERSION "${XE_VERSION_MAJOR}.${XE_VERSION_MINOR}.${XE_VERSION_REVISION}")

add_definitions(-DXE_EXTERNAL_ASSET_ROOT_PATH="${XE_EXTERNAL_ASSET_ROOT_PATH}")
add_definitions(-DXE_MEDIA_PATH="${CMAKE_CURRENT_LIST_DIR}/media")
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

if (XE_DEV_UNIT_TEST)
    include(CTest)
    enable_testing()
    find_package(Catch2 3 CONFIG REQUIRED)
endif()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions (-D_DEBUG)
endif ()

add_definitions (-DXE_VERSION_MAJOR=${XE_VERSION_MAJOR})
add_definitions (-DXE_VERSION_MINOR=${XE_VERSION_MINOR})
add_definitions (-DXE_VERSION_REVISION=${XE_VERSION_REVISION})

set (target xe-interface)
add_library(${target} INTERFACE)
add_library(xe::interface ALIAS ${target})

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set (XE_CXXFLAGS_WALL -Wall -Wextra)
    set (XE_CXXFLAGS_WERROR -Werror)
    set (XE_CXXFLAGS_COVERAGE -fprofile-arcs -ftest-coverage)
    set (XE_LDFLAGS_COVERAGE )

elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set (XE_CXXFLAGS_WALL -Wall -Wextra)
    set (XE_CXXFLAGS_WERROR -Werror)
    set (XE_CXXFLAGS_COVERAGE -fprofile-instr-generate -fcoverage-mapping)
    set (XE_LDFLAGS_COVERAGE -fprofile-instr-generate)

elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set (XE_CXXFLAGS_WALL /W4)
    set (XE_CXXFLAGS_WERROR /WX)
    set (XE_CXXFLAGS_COVERAGE )
    set (XE_LDFLAGS_COVERAGE )
endif()

target_compile_definitions(${target} INTERFACE _CRT_SECURE_NO_WARNINGS)

target_compile_options(${target} INTERFACE ${XE_CXXFLAGS_WALL})
if (XE_DEV_WARNINGS_AS_ERRORS)
    target_compile_options(${target} INTERFACE ${XE_CXXFLAGS_WERROR})
endif()

if (XE_DEV_INSTRUMENT_COVERAGE)
    target_compile_options(${target} INTERFACE ${XE_CXXFLAGS_COVERAGE})
    target_link_options(${target} INTERFACE ${XE_LDFLAGS_COVERAGE})
endif() 

if (XE_DEV_SANITIZER_ENABLE)
    target_compile_options(${target} INTERFACE -fsanitize=${XE_DEV_SANITIZER})
    target_link_options(${target} INTERFACE -fsanitize=${XE_DEV_SANITIZER})
endif()

if (XE_DEV_DOCS)
    find_package(Doxygen REQUIRED)

    set (XE_DOXYFILE_IN  "${CMAKE_SOURCE_DIR}/Doxyfile.in")
    set (XE_DOXYFILE_OUT "${CMAKE_BINARY_DIR}/Doxyfile")

    configure_file(${XE_DOXYFILE_IN} ${XE_DOXYFILE_OUT} @ONLY)

    # Single project-wide Doxygen target. Covers every library under src/
    # in one Doxygen run; INPUT and EXCLUDE_PATTERNS in Doxyfile.in control
    # what is documented.
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${XE_DOXYFILE_OUT}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating XE Doxygen documentation"
        VERBATIM)
endif()

add_subdirectory ("src")
