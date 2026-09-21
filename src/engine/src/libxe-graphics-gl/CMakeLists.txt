
find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_package(Backport CONFIG REQUIRED)
find_package(tl-expected CONFIG REQUIRED)

set (target "libxe-graphics-gl")

set (headers
	"src/xe/graphics/gl/GL.h"
	"src/xe/graphics/gl/GraphicsDeviceGL.h"
	"src/xe/graphics/gl/SubsetGL.h"
	"src/xe/graphics/gl/Conversion.h"
	"src/xe/graphics/gl/UtilGL.h"
	"src/xe/graphics/gl/TextureRepository.h"
	"src/xe/graphics/gl/Renderer.h"
	"src/xe/graphics/gl/RendererGL.h"
	"src/xe/graphics/gl/Types.h"

)

set (sources
	${headers}

	"src/xe/graphics/gl/GraphicsDeviceGL.cpp"
	"src/xe/graphics/gl/SubsetGL.cpp"
	"src/xe/graphics/gl/Conversion.cpp"
	"src/xe/graphics/gl/UtilGL.cpp"
	"src/xe/graphics/gl/Renderer.cpp"
	"src/xe/graphics/gl/TextureRepository.cpp"
	"src/xe/graphics/gl/RendererGL.cpp"
)

add_library (${target} ${sources})
add_library(xe::graphics-gl ALIAS ${target})

if (XE_ENABLE_PCH)
	target_precompile_headers (${target} PRIVATE ${headers})
endif()

target_include_directories (${target} PUBLIC "src")
target_link_libraries (${target}
	PUBLIC xe::core
	PUBLIC xe::graphics
	PUBLIC xe::imageloader
	PUBLIC glaze::gl
	PRIVATE xe::interface
	PUBLIC fmt::fmt
	PUBLIC glfw
	PUBLIC Backport::Backport
	PUBLIC Microsoft.GSL::GSL
	PUBLIC tl::expected
)

if (XE_ENABLE_TESTING)
    set(CMAKE_FOLDER "Engine/Tests")
    add_subdirectory("unit-test")
    set(CMAKE_FOLDER "Engine")
endif()
