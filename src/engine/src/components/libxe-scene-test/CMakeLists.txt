
set (target "xe-scene-test")
set (sources
    "src/Projection.Test.cpp"
    "src/Trackball.Test.cpp"
    "src/VirtualSphere.Test.cpp"
)

add_executable(${target} ${sources})
target_link_libraries(${target} PRIVATE Catch2::Catch2WithMain xe::scene xe::interface)

include(Catch)
catch_discover_tests(${target})
