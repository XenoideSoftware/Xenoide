
# Windows-specific options
option(XE_IDE_WIN32_UNICODE "Builds an UNICODE version. ANSI if OFF" ON)

option(XE_IDE_UI_WINLAMB "Enable WinLamb UI" OFF)

set(XE_IDE_WIN32_TARGET "10" CACHE STRING
    "Target Windows OS version (9x, xp, vista, 7, 8, 10, 11)")

set_property(CACHE XE_IDE_WIN32_TARGET PROPERTY STRINGS
    "9x" "xp" "vista" "7" "8" "10" "11")

if (XE_IDE_WIN32_TARGET STREQUAL "9x")
    set(XE_IDE_WIN32_TARGET_HEX 0x0410)
elseif (XE_IDE_WIN32_TARGET STREQUAL "xp")
    set(XE_IDE_WIN32_TARGET_HEX 0x0501)
elseif (XE_IDE_WIN32_TARGET STREQUAL "vista")
    set(XE_IDE_WIN32_TARGET_HEX 0x0600)
elseif (XE_IDE_WIN32_TARGET STREQUAL "7")
    set(XE_IDE_WIN32_TARGET_HEX 0x0601)
elseif (XE_IDE_WIN32_TARGET STREQUAL "8")
    set(XE_IDE_WIN32_TARGET_HEX 0x0602)
elseif (XE_IDE_WIN32_TARGET STREQUAL "10")
    set(XE_IDE_WIN32_TARGET_HEX 0x0A00)
elseif (XE_IDE_WIN32_TARGET STREQUAL "11")
    set(XE_IDE_WIN32_TARGET_HEX 0x0A00)
else()
    message(FATAL_ERROR
        "XE_IDE_WIN32_TARGET must be one of: 9x, xp, vista, 7, 8, 10, 11 (got '${XE_IDE_WIN32_TARGET}')")
endif()

message(STATUS "Xenoide: Building for Windows ${XE_IDE_WIN32_TARGET} target (${XE_IDE_WIN32_TARGET_HEX}). Unicode: ${XE_IDE_WIN32_UNICODE}")

if (XE_IDE_UI_WINLAMB)
    find_package(winlamb CONFIG REQUIRED)
endif()

# Unit Tests setup
if (XE_TESTS)
    find_package(GTest REQUIRED)
endif()

add_subdirectory("src")
