set (target "xe-cmake-checker")

find_package(cxxopts CONFIG REQUIRED)

set (sources
    "src/CliOptionsParser.cpp"
    "src/DiagnosticsReporter.cpp"
    "src/CheckerDriver.cpp"
    "src/main.cpp"
)

add_executable(${target} ${sources})

target_include_directories(${target} PUBLIC "src")

# one line per dependency
target_link_libraries(${target} PRIVATE xe::cmake-checker-rule-engine)
target_link_libraries(${target} PRIVATE xe::cmake-checker-io)
target_link_libraries(${target} PRIVATE xe::cmake-checker-core)
target_link_libraries(${target} PRIVATE cxxopts::cxxopts)