find_package(inja CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

include("${CMAKE_SOURCE_DIR}/cmake/EmbedTextFile.cmake")

set (target "glaze-cppgen")

set (generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
file (MAKE_DIRECTORY "${generated_dir}")

glaze_embed_text_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/templates/gl.hpp.inja"
    "${generated_dir}/gl_hpp_inja.cpp"
    "glaze::cppgen::detail::templates"
    "gl_hpp_inja"
)
glaze_embed_text_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/templates/gl_handle.hpp.inja"
    "${generated_dir}/gl_handle_hpp_inja.cpp"
    "glaze::cppgen::detail::templates"
    "gl_handle_hpp_inja"
)
glaze_embed_text_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/raii.hpp"
    "${generated_dir}/raii_hpp.cpp"
    "glaze::cppgen::detail::templates"
    "raii_hpp"
)

set (sources
    "src/glaze/cppgen/CppGenerator.h"
    "src/glaze/cppgen/CppGenerator.cpp"
    "src/glaze/cppgen/detail/GroupRename.h"
    "src/glaze/cppgen/detail/GroupRename.cpp"
    "src/glaze/cppgen/detail/PatternDetector.h"
    "src/glaze/cppgen/detail/PatternDetector.cpp"
    "src/glaze/cppgen/detail/ParamFormat.h"
    "src/glaze/cppgen/detail/ParamFormat.cpp"
    "src/glaze/cppgen/detail/EnumClassEmitter.h"
    "src/glaze/cppgen/detail/EnumClassEmitter.cpp"
    "src/glaze/cppgen/detail/RaiiCollector.h"
    "src/glaze/cppgen/detail/RaiiCollector.cpp"
    "src/glaze/cppgen/detail/FunctorEmitter.h"
    "src/glaze/cppgen/detail/FunctorEmitter.cpp"
    "src/glaze/cppgen/detail/OverloadEmitter.h"
    "src/glaze/cppgen/detail/OverloadEmitter.cpp"
    "src/glaze/cppgen/detail/DsaEmitter.h"
    "src/glaze/cppgen/detail/DsaEmitter.cpp"
    "src/glaze/cppgen/detail/HandleEmitter.h"
    "src/glaze/cppgen/detail/HandleEmitter.cpp"
    "src/glaze/cppgen/detail/CppKeywords.h"
    "src/glaze/cppgen/detail/CppKeywords.cpp"
    "${generated_dir}/gl_hpp_inja.cpp"
    "${generated_dir}/gl_handle_hpp_inja.cpp"
    "${generated_dir}/raii_hpp.cpp"
)

add_library(${target} ${sources})
add_library(glaze::cppgen ALIAS ${target})

target_include_directories(${target} PUBLIC "src")
target_link_libraries(${target}
    PUBLIC  glaze::model
    PUBLIC  glaze::docparser
    PRIVATE glaze::interface
    PRIVATE pantor::inja
    PRIVATE nlohmann_json::nlohmann_json
)
