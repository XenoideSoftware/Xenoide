set (test_target "xe-math-test")

set (test_sources
    "Box.Test.cpp"
    "Common.Test.cpp"
    "Ellipsoid.Test.cpp"
    "Matrix.Test.cpp"
    "Plane.Test.cpp"
    "Quaternion.Test.cpp"
    "Range.Test.cpp"
    "Ray.Test.cpp"
    "Rect.Test.cpp"
    "Rotation.Test.cpp"
    "Sphere.Test.cpp"
    "Triangle.Test.cpp"
    "Vector.Test.cpp"
)

add_executable(${test_target} ${test_sources})
target_link_libraries(${test_target} PRIVATE xe::math xe::core xe::interface Catch2::Catch2WithMain)

# NOTE: The test name must have the same name as the command name
add_test(NAME ${test_target} COMMAND ${test_target})
