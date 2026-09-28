set (target "libxe-cmake-checker-analysis")

set (headers
    "src/xe/cmake/analysis/SemanticModel.h"
    "src/xe/cmake/analysis/DirectedDependencyGraph.h"
)

set (sources
    ${headers}
    "src/xe/cmake/analysis/SemanticModel.cpp"
    "src/xe/cmake/analysis/DirectedDependencyGraph.cpp"
)

add_library(${target} ${sources})
add_library(xe::cmake-checker-analysis ALIAS ${target})

target_include_directories(${target} PUBLIC "src")

# one line per dependency
target_link_libraries(${target} PUBLIC xe::cmake-checker-core)