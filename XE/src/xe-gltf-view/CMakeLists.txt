
find_package(cgltf CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_package(Backport CONFIG REQUIRED)

set (target xe-gltf-view)
set (sources
        src/xe-gltf-view-main.cpp
        src/GltfMesh.h
        src/GltfProcessor.h
        src/GltfDataLoader.h
        src/GltfDataLoader.cpp
        src/GltfRenderer.h
        src/GltfRenderer.cpp
        src/Window.h
        src/Window.cpp
        src/bindings/imgui_impl_sdl2.h
        src/bindings/imgui_impl_sdl2.cpp
        src/bindings/imgui_impl_opengl3.h
        src/bindings/imgui_impl_opengl3.cpp
)

add_executable(${target} ${sources})
target_compile_definitions(${target} PRIVATE XE_GLTF_VIEW_SOURCE_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${target} PUBLIC include)
target_link_libraries(${target} PRIVATE xe::interface PUBLIC xe::core PUBLIC xe::gl PUBLIC xe::app PUBLIC cgltf::cgltf PRIVATE SDL2::SDL2 SDL2::SDL2main PUBLIC glaze::gl PUBLIC imgui::imgui PRIVATE Backport::Backport)
