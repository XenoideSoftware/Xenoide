cmake_minimum_required (VERSION 3.16)

# NOTE: This breaks when modifying any CMakeLists.txt file after the build system is generated
#list(APPEND CMAKE_MODULE_PATH "../cmake")

include("../cmake/XEVersion.cmake")
project (Xenoide VERSION ${XE_BUILD_VERSION})
include("../cmake/XETesting.cmake")
include("../cmake/XEBaseTarget.cmake")

option(XE_ENABLE_IDE_WINLAMB_UNICODE "Builds an UNICODE version. ANSI if OFF" ON)
option(XE_ENABLE_IDE_WINLAMB "Enable WinLamb UI" OFF)

option(XE_ENABLE_IDE_QT6 "Enable Qt6 UI" OFF)
option(XE_ENABLE_IDE_WX3 "Enable wxWidgets3 UI" OFF)
option(XE_ENABLE_IDE_GTK4 "Enable GTK4 UI" OFF)
option(XE_ENABLE_IDE_GTK3 "Enable GTK3 UI" OFF)

add_subdirectory("src")
