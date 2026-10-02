
find_package(imgui CONFIG REQUIRED)

if("sdl2" IN_LIST XE_ENGINE_IMGUI_BACKENDS)
    find_package(SDL2 CONFIG REQUIRED)
endif()

set (target "libxe-imgui")
set (sources )

foreach(backend ${XE_ENGINE_IMGUI_BACKENDS})
    message(STATUS "XE: Adding Imgui Backend ${item}")

    set (header "src/xe/imgui/imgui_impl_${backend}.h")
    set (source "src/xe/imgui/imgui_impl_${backend}.cpp")

    set(sources ${sources} ${header})
    set(sources ${sources} ${source})
endforeach()

add_library(${target} ${sources})
add_library(xe::imgui ALIAS ${target})

target_include_directories(${target} PUBLIC "src")

target_link_libraries(${target} PUBLIC imgui::imgui)
target_link_libraries(${target} PRIVATE SDL2::SDL2)
