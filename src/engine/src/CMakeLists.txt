
add_subdirectory ("libxe-core")
if (XE_ENABLE_TESTING)
    set(CMAKE_FOLDER "Engine/Tests")
    add_subdirectory ("libxe-core-test")
    set(CMAKE_FOLDER "Engine")
endif()

add_subdirectory ("libxe-math")
add_subdirectory ("libxe-scene")
add_subdirectory ("libxe-geometry")
if (XE_ENABLE_TESTING)
    set(CMAKE_FOLDER "Engine/Tests")
    add_subdirectory ("libxe-geometry-test")
    set(CMAKE_FOLDER "Engine")
endif()
# add_subdirectory ("libxe-app")

# DevIL doesn't build in MSVC
add_subdirectory ("libxe-imageloader")

# add_subdirectory ("xe-gltf-view")
# add_subdirectory ("xe-gltfc")

if (XE_ENABLE_ENGINE_PLUGIN_KTX)
    # add_subdirectory ("xe-ktxc")
endif()

if (XE_ENABLE_ENGINE_PLUGIN_PNG)
    # add_subdirectory("libxe-core-imageloader-lodepng")
endif ()

add_subdirectory ("libxe-graphics")
add_subdirectory ("libxe-graphics-gl")
add_subdirectory ("libxe-graphics-gl3")
add_subdirectory ("libxe-mesh")

if (XE_ENABLE_ENGINE_PLUGIN_GL_GLFW)
    # add_subdirectory("libxe-core-platform-glfw")
endif()
