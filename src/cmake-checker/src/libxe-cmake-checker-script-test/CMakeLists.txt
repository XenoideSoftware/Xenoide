find_package(Catch2 REQUIRED)
find_package(chaiscript CONFIG REQUIRED)

set (target "libxe-cmake-checker-script-test")

set (sources
    "src/ScriptEngineFacadeTest.cpp"
    "src/ScriptBindingsTest.cpp"
    "src/ScriptProxyNullTest.cpp"
)

add_executable(${target} ${sources})

# ChaiScript's operator-precedence table trips a known GCC/Clang 13
# -Warray-bounds false positive; suppress it locally (Werror is global).
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${target} PRIVATE -Wno-array-bounds)
endif()

target_include_directories(${target} PUBLIC "src")

# one line per dependency
target_link_libraries(${target} PRIVATE Catch2::Catch2WithMain)
target_link_libraries(${target} PUBLIC xe::cmake-checker-script)
target_link_libraries(${target} PUBLIC xe::cmake-checker-testing)
target_link_libraries(${target} PRIVATE chaiscript::chaiscript)

# Enable autodiscovering
include(Catch)
catch_discover_tests(${target})