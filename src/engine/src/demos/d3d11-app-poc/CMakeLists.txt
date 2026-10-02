
set (target "d3d11-app-poc")
set (source_base "src")
set (sources
	"${source_base}/D3D11AppPoc.cpp"
	"${source_base}/Renderer.h"
	"${source_base}/Renderer.cpp"
	"${source_base}/Mesh.h"
	"${source_base}/Mesh.cpp"
	"${source_base}/Shader.h"
	"${source_base}/Shader.cpp"
	"${source_base}/Color.vs"
	"${source_base}/Color.ps"
) 

if (NOT WIN32)
	message(STATUS "Windows platform not detected. d3d11-poc target is not available")
	return()
endif()

add_executable(${target} ${sources})

target_link_libraries(${target} d3d11 d3dcompiler xe::interface xe::core xe::math)
