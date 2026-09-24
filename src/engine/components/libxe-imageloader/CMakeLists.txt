
# find_package(DevIL)

set (target xe-imageloader)
add_library(${target}
        "include/xe/Image.h"
        "include/xe/ImageLoader.h"
        "src/ImageLoader.cpp"

        # "src/ImageLoaderIL.h"
        # "src/ImageLoaderIL.cpp"
)
add_library(xe::imageloader ALIAS ${target})

target_include_directories(${target} PUBLIC include/)
target_link_libraries(${target} xe::core)
# target_link_libraries(${target} DevIL::DevIL)
