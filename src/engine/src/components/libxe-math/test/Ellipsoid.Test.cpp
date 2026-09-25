
#include <catch2/catch_all.hpp>
#include <sstream>
#include <xe/math/Ellipsoid.h>

TEST_CASE("Ellipsoid default constructor initializes at the origin and with radius one", "[math][ellipsoid]") {
    xe::Ellipsoid e1;
    REQUIRE(e1.center == xe::Vector3(0.0f, 0.0f, 0.0f));
    REQUIRE(e1.size == xe::Vector3(1.0f, 1.0f, 1.0f));
}

TEST_CASE("Ellipsoid constructor initializes at the specified origin and radii", "[math][ellipsoid]") {
    xe::Ellipsoid e1{{1.0f, 2.0f, -4.0f}, {4.0f, 2.0f, 1.0f}};
    REQUIRE(e1.center == xe::Vector3(1.0f, 2.0f, -4.0f));
    REQUIRE(e1.size == xe::Vector3(4.0f, 2.0f, 1.0f));
}

TEST_CASE("Ellipsoid can be serialized to a non-empty string", "[math][ellipsoid]") {
    xe::Ellipsoid ellipsoid;

    std::stringstream ss;
    ss << ellipsoid;

    REQUIRE(ss.str() != "");
}
