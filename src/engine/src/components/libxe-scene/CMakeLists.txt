
set (target "xe-scene")
set (sources
	"src/xe/scene/Projection.h"
	"src/xe/scene/Trackball.h"
    "src/xe/scene/VirtualSphere.h"
	"src/xe/scene/Trackball.cpp"
	"src/xe/scene/VirtualSphere.cpp"
)

add_library(${target} ${sources})
add_library(xe::scene ALIAS ${target})

target_include_directories(${target} PUBLIC "src")
target_link_libraries(${target} PUBLIC xe::core PRIVATE xe::interface)
