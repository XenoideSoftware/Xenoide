
option (XE_ENGINE_PLUGIN_PNG_ENABLE           "Include PNG image support via LodePNG" ON)
option (XE_ENGINE_PLUGIN_GL_ENABLE            "Include OpenGL 3+ support" ON)
option (XE_ENGINE_PLUGIN_GL_GLFW_ENABLE       "Include GLFW-based Window and Context creation support (requires 'XE_ENGINE_PLUGIN_GL_ENABLE')" ON)
option (XE_ENGINE_PLUGIN_KTX_ENABLE           "Include KTX image support" ON)

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
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

add_definitions (-DXE_VERSION_MAJOR=${XE_VERSION_MAJOR})
add_definitions (-DXE_VERSION_MINOR=${XE_VERSION_MINOR})
add_definitions (-DXE_VERSION_REVISION=${XE_VERSION_REVISION})

add_subdirectory ("src")
