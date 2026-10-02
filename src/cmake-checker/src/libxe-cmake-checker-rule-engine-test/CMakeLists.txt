find_package(Catch2 REQUIRED)

set (target "libxe-cmake-checker-rule-engine-test")

set (sources
    "src/RuleRegistryTest.cpp"
    "src/CheckRunnerTest.cpp"
    "src/FixConflictResolverTest.cpp"
    "src/RuleEngineIntegrationTest.cpp"
)

add_executable(${target} ${sources})

target_include_directories(${target} PUBLIC "src")

# one line per dependency
target_link_libraries(${target} PRIVATE Catch2::Catch2WithMain)
target_link_libraries(${target} PUBLIC xe::cmake-checker-rule-engine)
target_link_libraries(${target} PUBLIC xe::cmake-checker-io)
target_link_libraries(${target} PUBLIC xe::cmake-checker-script)
target_link_libraries(${target} PUBLIC xe::cmake-checker-testing)

# Enable autodiscovering
include(Catch)
catch_discover_tests(${target})