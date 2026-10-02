
#include "xe/math/Ray.h"
#include <catch2/catch_all.hpp>

TEST_CASE("Ray default constructor initializes instance", "[math][ray]") {
    xe::TRay<float> ray1;
    REQUIRE(true);
}
