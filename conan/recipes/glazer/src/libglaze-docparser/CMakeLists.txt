find_package(pugixml CONFIG REQUIRED)

set (target "glaze-docparser")

set (sources
    "src/glaze/docparser/FunctionDoc.h"
    "src/glaze/docparser/DocIndex.h"
    "src/glaze/docparser/DocParser.h"
    "src/glaze/docparser/DocParser.cpp"
)

add_library(${target} ${sources})
add_library(glaze::docparser ALIAS ${target})

target_include_directories(${target} PUBLIC "src")
target_link_libraries(${target}
    PRIVATE glaze::interface
    PRIVATE pugixml::pugixml
)
