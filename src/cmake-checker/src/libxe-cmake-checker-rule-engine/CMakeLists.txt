set (target "libxe-cmake-checker-rule-engine")

set (headers
    "src/xe/cmake/rules/RuleRegistry.h"
    "src/xe/cmake/rules/CheckRunner.h"
    "src/xe/cmake/rules/FixConflictResolver.h"
)

set (sources
    ${headers}
    "src/xe/cmake/rules/RuleRegistry.cpp"
    "src/xe/cmake/rules/CheckRunner.cpp"
    "src/xe/cmake/rules/FixConflictResolver.cpp"
)

add_library(${target} ${sources})
add_library(xe::cmake-checker-rule-engine ALIAS ${target})

target_include_directories(${target} PUBLIC "src")

# one line per dependency
target_link_libraries(${target} PUBLIC xe::cmake-checker-dsl)
target_link_libraries(${target} PUBLIC xe::cmake-checker-script)
target_link_libraries(${target} PUBLIC xe::cmake-checker-analysis)
target_link_libraries(${target} PUBLIC xe::cmake-checker-core)