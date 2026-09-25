
find_package(lodepng CONFIG REQUIRED)

set (target "xe-graphics-png")
set (sources 
    "src/xe/graphics/ImageLoaderPNG.h"
    "src/xe/graphics/ImageLoaderPNG.cpp"
)

add_library(${target} ${sources})
add_library(xe::graphics-png ALIAS ${target})

target_include_directories(${target} PUBLIC "src")
target_link_libraries(${target}
    PUBLIC xe::core
    PRIVATE lodepng::lodepng
    PRIVATE xe::interface
)
