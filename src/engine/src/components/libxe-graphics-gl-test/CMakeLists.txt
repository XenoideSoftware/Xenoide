set (target "libxe-graphics-test")

set (sources
    "src/XEGraphicsGLUnitTest.cpp"
)

add_executable(${target} ${sources})
target_link_libraries(${target} PRIVATE Catch2::Catch2WithMain xe::graphics xe::interface)

include(Catch)
catch_discover_tests(${target})
