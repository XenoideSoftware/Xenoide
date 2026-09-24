
add_subdirectory("libxe-imgui")

if (XE_ENABLE_ENGINE_TOOL_GLTFV)
    add_subdirectory ("xe-gltf-view")
endif()

if (XE_ENABLE_ENGINE_TOOL_GLTFC)
    add_subdirectory ("xe-gltfc")
endif()

if (XE_ENABLE_ENGINE_TOOL_KTXC)
    add_subdirectory ("xe-ktxc")
endif()
