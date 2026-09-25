set (test_target "libxe-graphics-test")

set (test_sources
    "XEGraphicsGLUnitTest.cpp"
)

add_executable(${test_target} ${test_sources})
target_link_libraries(${test_target} PRIVATE Catch2::Catch2WithMain xe::graphics xe::interface)

# Let CTest discover each TEST_CASE individually.
include(Catch)
catch_discover_tests(${test_target})
