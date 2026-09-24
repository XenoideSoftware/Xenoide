set (target "glaze-model")

set (sources
    "src/glaze/model/ApiProfile.h"
    "src/glaze/model/TypeDecl.h"
    "src/glaze/model/TypeDecl.cpp"
    "src/glaze/model/Type.h"
    "src/glaze/model/Enum.h"
    "src/glaze/model/EnumGroup.h"
    "src/glaze/model/EnumGroup.cpp"
    "src/glaze/model/CommandParam.h"
    "src/glaze/model/CommandParam.cpp"
    "src/glaze/model/Command.h"
    "src/glaze/model/Command.cpp"
    "src/glaze/model/Feature.h"
    "src/glaze/model/Extension.h"
    "src/glaze/model/ConsolidatedRequire.h"
    "src/glaze/model/StringUtils.h"
    "src/glaze/model/StringUtils.cpp"
    "src/glaze/model/Registry.h"
    "src/glaze/model/Registry.cpp"
    "src/glaze/codegen/NameTransform.h"
    "src/glaze/codegen/NameTransform.cpp"
    "src/glaze/codegen/TypeMapper.h"
    "src/glaze/codegen/TypeMapper.cpp"
    "src/glaze/codegen/ExtensionFilter.h"
    "src/glaze/codegen/ExtensionFilter.cpp"
)

add_library(${target} ${sources})
add_library(glaze::model ALIAS ${target})

target_include_directories(${target} PUBLIC "src")
target_link_libraries(${target} PRIVATE glaze::interface)
