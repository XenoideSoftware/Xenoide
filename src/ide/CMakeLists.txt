
# Windows-specific options
option(XE_ENABLE_IDE_WINLAMB_UNICODE "Builds an UNICODE version. ANSI if OFF" ON)
option(XE_ENABLE_IDE_WINLAMB "Enable WinLamb UI" OFF)

if (XE_ENABLE_IDE_WINLAMB)
    find_package(winlamb CONFIG REQUIRED)
endif()

add_subdirectory("core")
