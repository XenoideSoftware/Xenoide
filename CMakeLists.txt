cmake_minimum_required (VERSION 3.25)
project (Xenoide LANGUAGES CXX)

include(XenoideConfig)

set(XE_ENABLE_TESTING "${XENOIDE_ENABLE_TESTING}" CACHE BOOL "Enable the unit test suite" FORCE)

option(XE_ENABLE_IDE_WINLAMB_UNICODE "Builds an UNICODE version. ANSI if OFF" ON)
option(XE_ENABLE_IDE_WINLAMB "Enable WinLamb UI" OFF)
option(XE_ENABLE_IDE_QT6 "Enable Qt6 UI" OFF)
option(XE_ENABLE_IDE_WX3 "Enable wxWidgets3 UI" OFF)
option(XE_ENABLE_IDE_GTK4 "Enable GTK4 UI" OFF)
option(XE_ENABLE_IDE_GTK3 "Enable GTK3 UI" OFF)

add_subdirectory("src")
