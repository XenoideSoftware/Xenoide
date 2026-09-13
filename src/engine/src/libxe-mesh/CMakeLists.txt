
find_package(tl-expected CONFIG REQUIRED)

set (target "xe-mesh")

set (sources
    "src/xe/mesh/MeshUpload.h"
    "src/xe/mesh/MeshUpload.cpp"
)

add_library(${target} ${sources})
add_library(xe::mesh ALIAS ${target})

target_include_directories(${target} PUBLIC "src")
target_link_libraries(${target}
    PUBLIC xe::geometry
    PUBLIC xe::graphics
    PUBLIC tl::expected
)
