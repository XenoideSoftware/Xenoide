find_package(cxxopts CONFIG REQUIRED)

set (target "glaze-core")

set (sources
    "src/glaze/Glaze.h"
    "src/glaze/Language.h"
    "src/glaze/GenerateOptions.h"
    "src/glaze/BindingGenerator.h"
    "src/glaze/BindingGenerator.cpp"
    "src/glaze/cli/ArgParse.h"
    "src/glaze/cli/ArgParse.cpp"
)

add_library(${target} ${sources})
add_library(glaze::core ALIAS ${target})

target_include_directories(${target} PUBLIC "src")
target_link_libraries(${target}
    PUBLIC  glaze::model
    PUBLIC  glaze::producer
    PUBLIC  glaze::docparser
    PUBLIC  glaze::cgen
    PUBLIC  glaze::cppgen
    PUBLIC  cxxopts::cxxopts
    PRIVATE glaze::interface
)
