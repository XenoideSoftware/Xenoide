set (target "libxe-cmake-checker-testing")

set (headers
    "src/xe/cmake/testing/CstSyntheticGenerator.h"
    "src/xe/cmake/testing/GraphSyntheticGenerator.h"
    "src/xe/cmake/testing/PropertyAssertions.h"
)

set (sources
    ${headers}
    "src/xe/cmake/testing/CstSyntheticGenerator.cpp"
    "src/xe/cmake/testing/GraphSyntheticGenerator.cpp"
    "src/xe/cmake/testing/PropertyAssertions.cpp"
)

add_library(${target} ${sources})
add_library(xe::cmake-checker-testing ALIAS ${target})

target_include_directories(${target} PUBLIC "src")

# one line per dependency
target_link_libraries(${target} PUBLIC xe::cmake-checker-core)
target_link_libraries(${target} PUBLIC xe::cmake-checker-analysis)