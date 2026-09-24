set(CMAKE_FOLDER "Engine/Demos")

if (XE_ENABLE_ENGINE_PLUGIN_D3D11)
    add_subdirectory(d3d11-app-poc)
endif()

if (XE_ENABLE_ENGINE_PLUGIN_VULKAN)
  add_subdirectory(vulkan-app-poc)
endif()

add_subdirectory(xe-glaze-app)  
add_subdirectory(xe-render-core-3d-app)
add_subdirectory(xe-render-core-app)

set(CMAKE_FOLDER "Engine")
