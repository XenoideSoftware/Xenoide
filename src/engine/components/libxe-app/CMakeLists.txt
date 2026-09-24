
find_package(glfw3 REQUIRED)

set (target xe-app)
set (sources include/xe/app/Platform.h src/Platform.cpp)

add_library(${target} ${sources})
add_library(xe::app ALIAS ${target})

target_include_directories(${target} PUBLIC include/)
target_link_libraries(${target} PUBLIC xe::core PUBLIC xe::gl PRIVATE glfw)
