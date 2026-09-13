
find_package(glaze CONFIG REQUIRED)

set (target "libxe-graphics-gl3")

set (sources 
	"src/xe/graphics/gl3/glcore3-api.h"
	"src/xe/graphics/gl3/glcore3-geometry.h"
	"src/xe/graphics/gl3/glcore3-pipeline.h"
	"src/xe/graphics/gl3/glcore3-program.h"
	"src/xe/graphics/gl3/glcore3-sampler.h"
	"src/xe/graphics/gl3/glcore3-texture.h"
	"src/xe/graphics/gl3/glcore3-vertexlayout.h"
	"src/xe/graphics/gl3/glcore3-buffer.h"
	"src/xe/graphics/gl3/glcore3-common.h"
    "src/xe/graphics/gl3/glcore3-context.h"

	"src/xe/graphics/gl3/glcore3-api.cpp"
	"src/xe/graphics/gl3/glcore3-geometry.cpp"
	"src/xe/graphics/gl3/glcore3-pipeline.cpp"
	"src/xe/graphics/gl3/glcore3-program.cpp"
	"src/xe/graphics/gl3/glcore3-sampler.cpp"
	"src/xe/graphics/gl3/glcore3-texture.cpp"
	"src/xe/graphics/gl3/glcore3-vertexlayout.cpp"
	"src/xe/graphics/gl3/glcore3-buffer.cpp"
	"src/xe/graphics/gl3/glcore3-common.cpp"
    "src/xe/graphics/gl3/glcore3-context.cpp"
)

add_library (${target} ${sources})
add_library(xe::graphics-gl3 ALIAS ${target})

target_include_directories(${target} PUBLIC "src")

target_link_libraries (${target} PRIVATE xe::interface)
target_link_libraries (${target} PUBLIC xe::graphics)
target_link_libraries (${target} PUBLIC glaze::gl)
