cmake_minimum_required (VERSION 3.16)

if (NOT XE_BUILD_VERSION)
    message (WARNING "[Xenoide] XE_BUILD_VERSION not set, defaulting to 0.0.0")
    set(XE_BUILD_VERSION "0.0.0")
endif()

if (NOT XE_BUILD_COMMIT)
    message (WARNING "[Xenoide] XE_BUILD_COMMIT not set, defaulting to 0")
    set(XE_BUILD_COMMIT "local")
endif()

if (NOT XE_BUILD_TIMESTAMP)
    message (WARNING "[Xenoide] XE_BUILD_TIMESTAMP not set, defaulting to 0")
    set(XE_BUILD_TIMESTAMP "now")
endif()

project (xenoide VERSION ${XE_BUILD_VERSION})

set (CMAKE_EXPORT_COMPILE_COMMANDS  ON)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

option (XE_ENGINE_ENABLE "Build with engine" ON)
option (XE_IDE_ENABLE "Build with IDE" ON)

# options suited for development
option (XE_DEV_TESTING_ENABLE       "Enable unit test suite" ON)
option (XE_DEV_WERROR_ENABLE        "Marks all warnings as errors" ON)
option (XE_DEV_DOCS_ENABLE          "Generate Doxygen documentation for the engine libraries" ON)
option (XE_DEV_PCH_ENABLE           "Enable precompiled headers for internal libraries, to improve build times" OFF)
option (XE_DEV_SANITIZER_ENABLE     "Enable sanitizer usage" OFF)
option (XE_DEV_COVERAGE_ENABLE      "Instrument code for code coverage reporting during unit tests" OFF)

# Which sanitizer to use
set (XE_DEV_SANITIZER "" CACHE STRING "")
if (XE_DEV_SANITIZER_ENABLE AND XE_DEV_SANITIZER STREQUAL "")
    message (WARNING "[Xenoide] XE_DEV_SANITIZER_ENABLE is OFF, but XE_DEV_SANITIZER is set to '${XE_DEV_SANITIZER}'. Sanitizer will not be used.")
endif()

if (XE_DEV_TESTING_ENABLE)
    include(CTest)
    enable_testing()
    find_package(Catch2 3 CONFIG REQUIRED)
endif()

if (XE_DEV_DOCS_ENABLE)
    find_package(Doxygen REQUIRED)

    set (XE_DOXYFILE_IN  "${CMAKE_SOURCE_DIR}/Doxyfile.in")
    set (XE_DOXYFILE_OUT "${CMAKE_BINARY_DIR}/Doxyfile")

    configure_file(${XE_DOXYFILE_IN} ${XE_DOXYFILE_OUT} @ONLY)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${XE_DOXYFILE_OUT}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating XE Doxygen documentation"
        VERBATIM)
endif()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions (-D_DEBUG)
endif ()

add_subdirectory(src)
