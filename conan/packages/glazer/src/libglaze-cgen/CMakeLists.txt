find_package(inja CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

include("${CMAKE_SOURCE_DIR}/cmake/EmbedTextFile.cmake")

set (target "glaze-cgen")

set (generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
file (MAKE_DIRECTORY "${generated_dir}")

glaze_embed_text_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/templates/gl.h.inja"
    "${generated_dir}/gl_h_inja.cpp"
    "glaze::cgen::detail::templates"
    "gl_h_inja"
)

glaze_embed_text_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/templates/gl.c.inja"
    "${generated_dir}/gl_c_inja.cpp"
    "glaze::cgen::detail::templates"
    "gl_c_inja"
)

set (sources
    "src/glaze/cgen/CGenerator.h"
    "src/glaze/cgen/CGenerator.cpp"
    "src/glaze/cgen/detail/CCommandFormat.h"
    "src/glaze/cgen/detail/CCommandFormat.cpp"
    "src/glaze/cgen/detail/CHeaderContext.h"
    "src/glaze/cgen/detail/CHeaderContext.cpp"
    "src/glaze/cgen/detail/CSourceContext.h"
    "src/glaze/cgen/detail/CSourceContext.cpp"
    "${generated_dir}/gl_h_inja.cpp"
    "${generated_dir}/gl_c_inja.cpp"
)

add_library(${target} ${sources})
add_library(glaze::cgen ALIAS ${target})

target_include_directories(${target} PUBLIC "src")
target_link_libraries(${target}
    PUBLIC  glaze::model
    PUBLIC  glaze::docparser
    PRIVATE glaze::interface
    PRIVATE pantor::inja
    PRIVATE nlohmann_json::nlohmann_json
)
