
set (target "xe-geometry-test")

set (sources
    "BoxGenerator.Test.cpp"
    "EllipsoidGenerator.Test.cpp"
    "PlaneGenerator.Test.cpp"
)

add_executable(${target} ${sources})
target_link_libraries(${target} PRIVATE
    xe::geometry
    xe::math
    Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(${target})
