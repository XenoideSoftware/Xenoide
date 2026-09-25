find_package(Catch2 3 CONFIG REQUIRED)

set (target cmake-checker-test)
set (sources
    "src/LexerTests.cpp"
    "src/TraceParserTests.cpp"
    "src/FileApiParserTests.cpp"
    "src/RulesTests.cpp"
    "src/ConfigTests.cpp"
    "src/ReportTests.cpp"
    "src/GlobTests.cpp"
)

add_executable(${target} ${sources})
target_link_libraries(${target} PRIVATE cmake-checker-core xe::interface Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(${target})