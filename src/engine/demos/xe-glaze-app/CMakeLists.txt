
find_package(glfw3 REQUIRED)
find_package(glaze REQUIRED)

set (target xe-glaze-app)
set (sources src/xe-glaze-app.cpp)
add_executable(${target} ${sources})
target_include_directories(${target} PUBLIC src)
target_link_libraries(${target} PRIVATE glfw)
target_link_libraries(${target} PRIVATE glaze::raii glaze::gl xe::math xe::interface)
