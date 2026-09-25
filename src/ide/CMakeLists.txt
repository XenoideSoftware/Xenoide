
project (Xenoide)

option(XE_ENABLE_IDE_WINLAMB_UNICODE "Builds an UNICODE version. ANSI if OFF" ON)
option(XE_ENABLE_IDE_WINLAMB "Enable WinLamb UI" OFF)

option(XE_ENABLE_IDE_QT6 "Enable Qt6 UI" OFF)
option(XE_ENABLE_IDE_WX3 "Enable Qt6 UI" OFF)
option(XE_ENABLE_IDE_GTK4 "Enable Qt6 UI" OFF)

set(CMAKE_FOLDER "IDE/Core")
    add_subdirectory("core")
set(CMAKE_FOLDER "IDE")

if (XE_ENABLE_IDE_WINLAMB)
    set(CMAKE_FOLDER "IDE/Win32")
    find_package(winlamb CONFIG REQUIRED)
    add_subdirectory("win32")
    set(CMAKE_FOLDER "IDE")
endif()

if (XE_ENABLE_IDE_QT6)
    set(CMAKE_FOLDER "IDE/Qt6")
    add_subdirectory("qt6")
    set(CMAKE_FOLDER "IDE")
endif()

if (XE_ENABLE_IDE_WX3)
    set(CMAKE_FOLDER "IDE/WX3")
    add_subdirectory("wx3")
    set(CMAKE_FOLDER "IDE")
endif()

if (XE_ENABLE_IDE_GTK4)
    set(CMAKE_FOLDER "IDE/GTK4")
    add_subdirectory("gtk4")
    set(CMAKE_FOLDER "IDE")
endif()
