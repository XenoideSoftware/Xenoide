cmake_minimum_required(VERSION 3.15)
project(win32_property_grid_pkg C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# propertyGrid.cpp is plain C source despite the .cpp extension.
set_source_files_properties(propertyGrid.cpp PROPERTIES LANGUAGE C)

add_library(win32-property-grid STATIC propertyGrid.cpp)
target_include_directories(win32-property-grid PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(win32-property-grid PUBLIC user32 comctl32 gdi32)
