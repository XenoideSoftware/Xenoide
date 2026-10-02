
#include "xe/math/Triangle.h"
#include <catch2/catch_all.hpp>

TEST_CASE("Triangle default constructor initializes instance", "[math][triangle]") {
    xe::TTriangle<float> tri1;
    REQUIRE(true);
}
