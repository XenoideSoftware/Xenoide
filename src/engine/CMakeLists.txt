cmake_minimum_required (VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH "../cmake")

include(XEVersion)
project (XE VERSION ${XE_BUILD_VERSION})
include(XETesting)
include(XEBaseTarget)

message(STATUS "Xenoide: Build version: ${XE_BUILD_VERSION} (${XE_BUILD_COMMIT})")

option (XE_ENABLE_ENGINE_PLUGIN_PNG           "Include PNG image support via LodePNG" ON)
option (XE_ENABLE_ENGINE_PLUGIN_GL            "Include OpenGL 3+ support" ON)
option (XE_ENABLE_ENGINE_PLUGIN_GL_GLFW       "Include GLFW-based Window and Context creation support (requires 'XE_ENABLE_ENGINE_PLUGIN_GL')" ON)
option (XE_ENABLE_ENGINE_PLUGIN_VULKAN        "Include Vulkan support" OFF)
option (XE_ENABLE_ENGINE_PLUGIN_D3D11         "Include d3d11 support" OFF)
option (XE_ENABLE_ENGINE_TOOL_KTXC            "Builds the KTX texture compiler" ON)
option (XE_ENABLE_ENGINE_TOOL_GLTFC           "Builds the GLTF model compiler" ON)
option (XE_ENABLE_ENGINE_TOOL_GLTFV           "Builds the GLTF model compiler" ON)

add_subdirectory("src")
