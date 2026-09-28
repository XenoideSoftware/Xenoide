find_package(Catch2 REQUIRED)

set (target "libxe-cmake-checker-core-test")

set (sources
    "src/ConcreteSyntaxTreeTest.cpp"
    "src/LexerTest.cpp"
    "src/QueryPrimitivesTest.cpp"
    "src/MutationEngineTest.cpp"
    "src/RefactoringPrimitivesTest.cpp"
    "src/SourceSpanTest.cpp"
)

add_executable(${target} ${sources})

target_include_directories(${target} PUBLIC "src")

# one line per dependency
target_link_libraries(${target} PRIVATE Catch2::Catch2WithMain)
target_link_libraries(${target} PUBLIC xe::cmake-checker-core)
target_link_libraries(${target} PUBLIC xe::cmake-checker-testing)

# Enable autodiscovering
include(Catch)
catch_discover_tests(${target})