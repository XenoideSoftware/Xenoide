
#include "xe/math/Sphere.h"
#include <catch2/catch_all.hpp>

TEST_CASE("Sphere default constructor initializes instance", "[math][sphere]") {
    xe::TSphere<float> tri1;
    REQUIRE(true);
}
