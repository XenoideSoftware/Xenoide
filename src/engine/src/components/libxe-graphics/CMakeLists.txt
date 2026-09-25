
find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_package(Backport CONFIG REQUIRED)
find_package(tl-expected CONFIG REQUIRED)

set (target "libxe-graphics")

set (headers
	# xe/graphics/
	"src/xe/graphics/Graphics.h"
	"src/xe/graphics/GraphicsAPI.h"
	"src/xe/graphics/GraphicsContext.h"
	"src/xe/graphics/GraphicsDevice.h"
	"src/xe/graphics/GraphicsDeviceFactory.h"
	"src/xe/graphics/GraphicsManager.h"
	"src/xe/graphics/IWindow.h"
	"src/xe/graphics/BufferDescriptor.h"
	"src/xe/graphics/Subset.h"
	"src/xe/graphics/Texture.h"
	"src/xe/graphics/PixelFormat.h"
	"src/xe/graphics/Shader.h"
	"src/xe/graphics/Program.h"
	"src/xe/graphics/Vertex.h"
	"src/xe/graphics/VertexLayout.h"
	"src/xe/graphics/VertexFormat.h"
	"src/xe/graphics/Uniform.h"
	"src/xe/graphics/ImageImpl.h"
	"src/xe/graphics/Image.h"
	"src/xe/graphics/ImageLoader.h"
	"src/xe/graphics/Material.h"
	"src/xe/graphics/Viewport.h"

	# xe/graphics/
	"src/xe/graphics/RenderBackend.h"
	"src/xe/graphics/types.h"
)

set (sources
	${headers}

	# xe/graphics/ implementations
	"src/xe/graphics/GraphicsContext.cpp"
	"src/xe/graphics/GraphicsDevice.cpp"
	"src/xe/graphics/GraphicsDeviceFactory.cpp"
	"src/xe/graphics/GraphicsManager.cpp"
	"src/xe/graphics/IWindow.cpp"
	"src/xe/graphics/Subset.cpp"
	"src/xe/graphics/Texture.cpp"
	"src/xe/graphics/PixelFormat.cpp"
	"src/xe/graphics/Shader.cpp"
	"src/xe/graphics/Program.cpp"
	"src/xe/graphics/ImageImpl.cpp"
	"src/xe/graphics/Image.cpp"
	"src/xe/graphics/ImageLoader.cpp"
	"src/xe/graphics/Material.cpp"

	# xe/graphics/ implementations
	"src/xe/graphics/RenderBackend.cpp"
)

add_library (${target} ${sources})
add_library(xe::graphics ALIAS ${target})

if (XE_ENABLE_PCH)
	target_precompile_headers (${target} PRIVATE ${headers})
endif()

target_include_directories (${target} PUBLIC "src")
target_link_libraries (${target}
	PUBLIC xe::core
	PUBLIC glaze::gl
	PRIVATE xe::interface
	PUBLIC fmt::fmt
	PUBLIC glfw
	PUBLIC Backport::Backport
	PUBLIC Microsoft.GSL::GSL
	PUBLIC tl::expected
)
