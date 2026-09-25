
project (Xenoide)

option(XE_ENABLE_IDE_WINLAMB_UNICODE "Builds an UNICODE version. ANSI if OFF" ON)
option(XE_ENABLE_IDE_WINLAMB "Enable WinLamb UI" OFF)

option(XE_ENABLE_IDE_QT6 "Enable Qt6 UI" OFF)
option(XE_ENABLE_IDE_WX3 "Enable wxWidgets3 UI" OFF)
option(XE_ENABLE_IDE_GTK4 "Enable GTK4 UI" OFF)

add_subdirectory("../base-target")
add_subdirectory("src")
