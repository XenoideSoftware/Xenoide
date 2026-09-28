set (target "libxe-cmake-checker-core")

set (headers
    "src/xe/cmake/core/SourceSpan.h"
    "src/xe/cmake/core/Trivia.h"
    "src/xe/cmake/core/ConcreteSyntaxTree.h"
    "src/xe/cmake/core/Lexer.h"
    "src/xe/cmake/core/QueryPrimitives.h"
    "src/xe/cmake/core/MutationEngine.h"
    "src/xe/cmake/core/RefactoringPrimitives.h"
)

set (sources
    ${headers}
    "src/xe/cmake/core/ConcreteSyntaxTree.cpp"
    "src/xe/cmake/core/Lexer.cpp"
    "src/xe/cmake/core/QueryPrimitives.cpp"
    "src/xe/cmake/core/MutationEngine.cpp"
    "src/xe/cmake/core/RefactoringPrimitives.cpp"
)

add_library(${target} ${sources})
add_library(xe::cmake-checker-core ALIAS ${target})

target_include_directories(${target} PUBLIC "src")

# one line per dependency
target_link_libraries(${target} PUBLIC xe::interface)