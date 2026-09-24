
option (XE_ENABLE_ENGINE_PLUGIN_PNG           "Include PNG image support via LodePNG" ON)
option (XE_ENABLE_ENGINE_PLUGIN_GL            "Include OpenGL 3+ support" ON)
option (XE_ENABLE_ENGINE_PLUGIN_GL_GLFW       "Include GLFW-based Window and Context creation support (requires 'XE_ENABLE_ENGINE_PLUGIN_GL')" ON)
option (XE_ENABLE_ENGINE_PLUGIN_VULKAN        "Include Vulkan support" OFF)
option (XE_ENABLE_ENGINE_PLUGIN_D3D11         "Include d3d11 support" OFF)
option (XE_ENABLE_ENGINE_TOOL_KTXC            "Builds the KTX texture compiler" ON)
option (XE_ENABLE_ENGINE_TOOL_GLTFC           "Builds the GLTF model compiler" ON)
option (XE_ENABLE_ENGINE_TOOL_GLTFV           "Builds the GLTF model compiler" ON)

# Options for external assets location
if (NOT XE_EXTERNAL_ASSET_ROOT_PATH)
    message(WARNING "XE_EXTERNAL_ASSET_ROOT_FOLDER variable not defined. Runtime errors might arise when running certain targets")
    set (XE_EXTERNAL_ASSET_ROOT_PATH "")
endif()

# TODO: Configure the version numbering mechanism externally
set (XE_VERSION_MAJOR 1)
set (XE_VERSION_MINOR 0)
set (XE_VERSION_REVISION 0)
set (XE_VERSION "${XE_VERSION_MAJOR}.${XE_VERSION_MINOR}.${XE_VERSION_REVISION}")

add_definitions(-DXE_EXTERNAL_ASSET_ROOT_PATH="${XE_EXTERNAL_ASSET_ROOT_PATH}")
add_definitions(-DXE_MEDIA_PATH="${CMAKE_CURRENT_LIST_DIR}/media")

add_definitions (-DXE_VERSION_MAJOR=${XE_VERSION_MAJOR})
add_definitions (-DXE_VERSION_MINOR=${XE_VERSION_MINOR})
add_definitions (-DXE_VERSION_REVISION=${XE_VERSION_REVISION})

add_subdirectory ("components")
add_subdirectory ("demos")
add_subdirectory ("tools")
