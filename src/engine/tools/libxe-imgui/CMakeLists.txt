
find_package(imgui CONFIG REQUIRED)

set (target "libxe-imgui")
set (sources )

foreach(backend ${XE_ENGINE_IMGUI_BACKENDS})
    message(STATUS "XE: Adding Imgui Backend ${item}")

    set (header "src/imgui_impl_${backend}.h")
    set (source "src/imgui_impl_${backend}.cpp")

    set(sources ${sources} ${header})
    set(sources ${sources} ${source})
endforeach()

add_library(${target} ${sources})
add_library(xe::imgui ALIAS ${target})

target_include_directories(${target} PUBLIC "src")

target_link_libraries(${target} PUBLIC imgui::imgui)
