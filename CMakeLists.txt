
cmake_minimum_required (VERSION 3.25)

project (Xenoide LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

option(XENOIDE_ENABLE_TESTING "Enable the unit test suite" OFF)
option(XE_ENABLE_UI_QT6 "Enable Qt6 UI" OFF)

if (XENOIDE_ENABLE_TESTING)
    find_package(Catch2 REQUIRED)
endif()

if (XE_ENABLE_UI_QT6)
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
endif()

add_subdirectory("src")
