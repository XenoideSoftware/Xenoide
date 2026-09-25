
find_package(DevIL REQUIRED)

set (target xe-graphics-il)
set (sources 
    "src/xe/graphics/ImageLoaderIL.h"
    "src/xe/graphics/ImageLoaderIL.cpp")

add_library(${target} ${sources})
add_library(xe::graphics-il ALIAS ${target})

target_include_directories(${target} PUBLIC src)
target_link_libraries(${target} xe::core)
target_link_libraries(${target} DevIL::DevIL)
