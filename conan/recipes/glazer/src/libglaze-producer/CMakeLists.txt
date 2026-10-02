find_package(pugixml CONFIG REQUIRED)

set (target "glaze-producer")

set (sources
    "src/glaze/producer/ParseError.h"
    "src/glaze/producer/ParseError.cpp"
    "src/glaze/producer/RegistryLoader.h"
    "src/glaze/producer/RegistryLoader.cpp"
    "src/glaze/producer/detail/XmlHelpers.h"
    "src/glaze/producer/detail/XmlHelpers.cpp"
    "src/glaze/producer/detail/TypeParser.h"
    "src/glaze/producer/detail/TypeParser.cpp"
    "src/glaze/producer/detail/EnumParser.h"
    "src/glaze/producer/detail/EnumParser.cpp"
    "src/glaze/producer/detail/CommandParser.h"
    "src/glaze/producer/detail/CommandParser.cpp"
    "src/glaze/producer/detail/FeatureParser.h"
    "src/glaze/producer/detail/FeatureParser.cpp"
    "src/glaze/producer/detail/ExtensionParser.h"
    "src/glaze/producer/detail/ExtensionParser.cpp"
)

add_library(${target} ${sources})
add_library(glaze::producer ALIAS ${target})

target_include_directories(${target} PUBLIC "src")
target_link_libraries(${target}
    PUBLIC  glaze::model
    PRIVATE glaze::interface
    PRIVATE pugixml::pugixml
)
