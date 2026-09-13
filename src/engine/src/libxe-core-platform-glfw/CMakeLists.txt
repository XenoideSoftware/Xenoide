
find_package(glfw3 CONFIG REQUIRED)

set (target xe-platform-glfw)

set (headers
    "src/xe/platform/glfw/WindowGLFW.h"
    "src/xe/platform/glfw/GraphicsContextGLFW.h"
    "src/xe/platform/glfw/InputManagerGLFW.h"
)

set (sources
    ${headers}
    "src/xe/platform/glfw/WindowGLFW.cpp"
    "src/xe/platform/glfw/GraphicsContextGLFW.cpp"
    "src/xe/platform/glfw/InputManagerGLFW.cpp"    
)

add_library(${target} ${sources})
add_library(xe::platform-glfw ALIAS ${target})

if (XE_DEV_PCH_ENABLE)
    target_precompile_headers(${target} PRIVATE ${headers})
endif()

target_include_directories(${target} PUBLIC "src")
target_link_libraries(${target}
    PUBLIC xe::core
    PRIVATE glfw
    PRIVATE xe::interface
)
