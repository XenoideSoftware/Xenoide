
find_package(glfw3 REQUIRED)
find_package(glaze REQUIRED)

set(target xe-render-core-3d)
set(sources src/xe-render-core-3d-app-main.cpp)

add_executable(${target} ${sources})

target_include_directories(${target} PUBLIC src)

target_link_libraries(${target} PRIVATE glfw)
target_link_libraries(${target} PRIVATE glaze::raii glaze::gl xe::math xe::core xe::graphics xe::geometry xe::interface)
