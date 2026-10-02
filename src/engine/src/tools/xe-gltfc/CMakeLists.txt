find_package(cgltf CONFIG REQUIRED)
find_package(Assimp CONFIG REQUIRED)

set (target "xe-gltfc")
set (sources "src/xe-gltfc-main.cpp")

add_executable(${target} ${sources})
add_executable(xe::gltfc ALIAS ${target})
target_link_libraries(${target} PRIVATE xe::interface PUBLIC cgltf::cgltf PUBLIC assimp::assimp )
