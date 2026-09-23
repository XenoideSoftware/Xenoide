
find_package(glfw3 REQUIRED)
find_package(glaze REQUIRED)

set (target xe-glaze-test)
set (sources src/xe-glaze-test.cpp)
add_executable(${target} ${sources})
target_include_directories(${target} PUBLIC src)
target_link_libraries(${target} PRIVATE glfw)
target_link_libraries(${target} PRIVATE glaze::raii glaze::gl xe::math xe::interface)

set (target_renderer xe-render-core)
set (sources_renderer
    src/xe-render-core-test.cpp
)
add_executable(${target_renderer} ${sources_renderer})
target_include_directories(${target_renderer} PUBLIC src)
target_link_libraries(${target_renderer} PRIVATE glfw)
target_link_libraries(${target_renderer} PRIVATE glaze::raii glaze::gl xe::math xe::core xe::graphics xe::graphics-gl3 xe::interface)
