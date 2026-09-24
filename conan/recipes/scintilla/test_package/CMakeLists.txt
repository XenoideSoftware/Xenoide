cmake_minimum_required(VERSION 3.15)
project(scintillaTest CXX)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(scintilla CONFIG REQUIRED)

set (target scintillaTest)

add_executable(${target} main.cpp hello-window.h hello-window.cpp)

target_link_libraries(${target} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets scintilla)
