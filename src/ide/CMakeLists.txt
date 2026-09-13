cmake_minimum_required(VERSION 3.25)

project (Xenoide)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(XENOIDE_WARNINGS_AS_ERRORS "Consider any warnings as build errors" OFF)
option(XENOIDE_WIN32_UNICODE "Builds an UNICODE version. ANSI if OFF" ON)

option(XENOIDE_TESTS "Enable Unit Tests" OFF)
option(XENOIDE_UI_WX "Enable wxWidgets UI" OFF)
option(XENOIDE_UI_QT6 "Enable Qt6 UI" OFF)
option(XENOIDE_UI_WINLAMB "Enable WinLamb UI" OFF)

set(XENOIDE_WIN32_TARGET "10" CACHE STRING
    "Target Windows OS version (9x, xp, vista, 7, 8, 10, 11)")
set_property(CACHE XENOIDE_WIN32_TARGET PROPERTY STRINGS
    "9x" "xp" "vista" "7" "8" "10" "11")

if (XENOIDE_WIN32_TARGET STREQUAL "9x")
    set(XENOIDE_WIN32_TARGET_HEX 0x0410)
elseif (XENOIDE_WIN32_TARGET STREQUAL "xp")
    set(XENOIDE_WIN32_TARGET_HEX 0x0501)
elseif (XENOIDE_WIN32_TARGET STREQUAL "vista")
    set(XENOIDE_WIN32_TARGET_HEX 0x0600)
elseif (XENOIDE_WIN32_TARGET STREQUAL "7")
    set(XENOIDE_WIN32_TARGET_HEX 0x0601)
elseif (XENOIDE_WIN32_TARGET STREQUAL "8")
    set(XENOIDE_WIN32_TARGET_HEX 0x0602)
elseif (XENOIDE_WIN32_TARGET STREQUAL "10")
    set(XENOIDE_WIN32_TARGET_HEX 0x0A00)
elseif (XENOIDE_WIN32_TARGET STREQUAL "11")
    set(XENOIDE_WIN32_TARGET_HEX 0x0A00)
else()
    message(FATAL_ERROR
        "XENOIDE_WIN32_TARGET must be one of: 9x, xp, vista, 7, 8, 10, 11 (got '${XENOIDE_WIN32_TARGET}')")
endif()

message(STATUS "Xenoide: Building for Windows ${XENOIDE_WIN32_TARGET} target (${XENOIDE_WIN32_TARGET_HEX}). Unicode: ${XENOIDE_WIN32_UNICODE}")

find_package(winlamb CONFIG REQUIRED)
# WinLamb setup
if (XENOIDE_UI_WINLAMB)
    find_package(winlamb CONFIG REQUIRED)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# version
set(XENOIDE_VERSION_MAJOR "0" CACHE STRING "Xenoide major version, for breaking feature changes")
set(XENOIDE_VERSION_MINOR "0" CACHE STRING "Xenoide minor version, for non-breaking feature changes")
set(XENOIDE_VERSION_REVISION "0" CACHE STRING "Xenoide bugfix number")

# build tracking information
set(XENOIDE_BUILD_COMMIT "local" CACHE STRING "Git hash used to generate this build")
set(XENOIDE_BUILD_TIMESTAMP "now" CACHE STRING "When this build was made, in YYYYmmDDHHMMSS format (if set)")

# sanitizer
set(XENOIDE_SANITIZER "" CACHE STRING "Which sanitizer to use")

# Unit Tests setup
if (XENOIDE_TESTS)
    find_package(GTest REQUIRED)
endif()

# Qt6 setup
if (XENOIDE_UI_QT6)
    set (CMAKE_INCLUDE_CURRENT_DIR ON)
    set (CMAKE_AUTOMOC ON)
    set (CMAKE_AUTOUIC ON)
    find_package(Qt6 COMPONENTS Core Widgets Gui REQUIRED)
    find_package(scintilla CONFIG REQUIRED)
    find_package(lexilla CONFIG REQUIRED)
endif()

# wxWidgets setup
if (XENOIDE_UI_WX)
    set(wxWidgets_CONFIGURATION mswu)
    find_package(wxWidgets REQUIRED)
endif()

add_subdirectory("src")
