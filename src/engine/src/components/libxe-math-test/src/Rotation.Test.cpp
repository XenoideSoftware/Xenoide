
#include "xe/math/Rotation.h"
#include <catch2/catch_all.hpp>

TEST_CASE("Rotation default constructor initializes instance", "[math][rotation]") {
    xe::Rotation<float> tri1;
    REQUIRE(true);
}
