
set (target "xe-math")

set (headers
    "src/xe/math/Math.h"
    "src/xe/math/Math.h"
    "src/xe/math/Common.h"
    "src/xe/math/Vector.h"
    "src/xe/math/Matrix.h"
    "src/xe/math/Quaternion.h"
    "src/xe/math/Boundary.h"
    "src/xe/math/Box.h"
    "src/xe/math/Plane.h"
    "src/xe/math/Ray.h"
    "src/xe/math/Rect.h"
    "src/xe/math/Sphere.h"
    "src/xe/math/Triangle.h"
    "src/xe/math/Ellipsoid.h"
    "src/xe/math/Rotation.h"
    "src/xe/math/Range.h"
    "src/xe/math/Boundary.h"
    "src/xe/math/VectorExpr.h")

set (sources
    ${headers}
        src/xe/math/Quaternion.cpp
        src/xe/math/Vector.cpp
        src/xe/math/Matrix.cpp
        src/xe/math/Box.cpp
        src/xe/math/Plane.cpp
        src/xe/math/Range.cpp
        src/xe/math/Ray.cpp
        src/xe/math/Rect.cpp
        src/xe/math/Ellipsoid.cpp
        src/xe/math/Sphere.cpp
        src/xe/math/Triangle.cpp)

add_library (${target} ${sources})
add_library(xe::math ALIAS ${target})

if (XE_DEV_PCH_ENABLE)
    target_precompile_headers(${target} PRIVATE ${headers})
endif()

target_include_directories(${target} PUBLIC "src")
target_link_libraries(${target} PRIVATE xe::interface)

if (XE_DEV_TESTING_ENABLE)
    add_subdirectory("test")
endif()
