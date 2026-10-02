find_package(Catch2 REQUIRED)

set (target "libxe-cmake-checker-io-test")

set (sources
    "src/FileSystemTest.cpp"
    "src/ProjectLoaderTest.cpp"
    "src/BuildTreeReaderTest.cpp"
    "src/SyncWriterTest.cpp"
)

add_executable(${target} ${sources})

target_include_directories(${target} PUBLIC "src")

# one line per dependency
target_link_libraries(${target} PRIVATE Catch2::Catch2WithMain)
target_link_libraries(${target} PUBLIC xe::cmake-checker-io)
target_link_libraries(${target} PUBLIC xe::cmake-checker-testing)

# Enable autodiscovering
include(Catch)
catch_discover_tests(${target})