
set (target xe-geometry)

set (sources
    "src/xe/geometry/MeshBuffers.h"
    "src/xe/geometry/EllipsoidGenerator.h"
    "src/xe/geometry/PlaneGenerator.h"
    "src/xe/geometry/BoxGenerator.h"
    "src/xe/geometry/EllipsoidGenerator.cpp"
    "src/xe/geometry/PlaneGenerator.cpp"
    "src/xe/geometry/BoxGenerator.cpp"
)

add_library(${target} ${sources})
add_library(xe::geometry ALIAS ${target})

target_include_directories(${target} PUBLIC "src")
target_link_libraries(${target} xe::core)
