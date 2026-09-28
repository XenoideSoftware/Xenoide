set (target "libxe-cmake-checker-dsl")

find_package(ryml CONFIG REQUIRED)

set (headers
    "src/xe/cmake/dsl/DslValue.h"
    "src/xe/cmake/dsl/DslRule.h"
    "src/xe/cmake/dsl/YamlRuleLoader.h"
    "src/xe/cmake/dsl/DslPredicateEvaluator.h"
    "src/xe/cmake/dsl/FixTemplateEngine.h"
)

set (sources
    ${headers}
    "src/xe/cmake/dsl/YamlRuleLoader.cpp"
    "src/xe/cmake/dsl/DslPredicateEvaluator.cpp"
    "src/xe/cmake/dsl/FixTemplateEngine.cpp"
)

add_library(${target} ${sources})
add_library(xe::cmake-checker-dsl ALIAS ${target})

target_include_directories(${target} PUBLIC "src")

# one line per dependency
target_link_libraries(${target} PUBLIC xe::cmake-checker-core)
target_link_libraries(${target} PUBLIC xe::cmake-checker-analysis)
target_link_libraries(${target} PUBLIC ryml::ryml)