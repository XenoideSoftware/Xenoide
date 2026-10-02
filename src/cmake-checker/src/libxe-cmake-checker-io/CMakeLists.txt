set (target "libxe-cmake-checker-io")

find_package(nlohmann_json CONFIG REQUIRED)

set (headers
    "src/xe/cmake/io/FileSystem.h"
    "src/xe/cmake/io/ICMakeParser.h"
    "src/xe/cmake/io/DefaultCmakeParser.h"
    "src/xe/cmake/io/ProjectLoader.h"
    "src/xe/cmake/io/BuildTreeReader.h"
    "src/xe/cmake/io/SyncWriter.h"
)

set (sources
    ${headers}
    "src/xe/cmake/io/FileSystem.cpp"
    "src/xe/cmake/io/DefaultCmakeParser.cpp"
    "src/xe/cmake/io/ProjectLoader.cpp"
    "src/xe/cmake/io/BuildTreeReader.cpp"
    "src/xe/cmake/io/SyncWriter.cpp"
)

add_library(${target} ${sources})
add_library(xe::cmake-checker-io ALIAS ${target})

target_include_directories(${target} PUBLIC "src")

# one line per dependency
target_link_libraries(${target} PUBLIC xe::cmake-checker-core)
target_link_libraries(${target} PUBLIC xe::cmake-checker-analysis)
target_link_libraries(${target} PUBLIC nlohmann_json::nlohmann_json)