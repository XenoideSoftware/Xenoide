set (target "xe-math-test")

set (sources
    "src/Box.Test.cpp"
    "src/Common.Test.cpp"
    "src/Ellipsoid.Test.cpp"
    "src/Matrix.Test.cpp"
    "src/Plane.Test.cpp"
    "src/Quaternion.Test.cpp"
    "src/Range.Test.cpp"
    "src/Ray.Test.cpp"
    "src/Rect.Test.cpp"
    "src/Rotation.Test.cpp"
    "src/Sphere.Test.cpp"
    "src/Triangle.Test.cpp"
    "src/Vector.Test.cpp"
)

add_executable(${target} ${sources})
target_link_libraries(${target} PRIVATE xe::math xe::core xe::interface Catch2::Catch2WithMain)

# Enable autodiscovering    
include(Catch)
catch_discover_tests(${target})
