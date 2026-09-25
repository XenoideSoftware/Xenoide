
#include "xe/math/Common.h"
#include <catch2/catch_all.hpp>

TEST_CASE("Common equals with no epsilon compares exactly", "[math][common]") {
    REQUIRE(xe::equals(1.0f, 1.0f, 0.0f));
    REQUIRE(xe::equals(1.0f, 1.0f));
}

TEST_CASE("Common equals with epsilon compares approximately", "[math][common]") {
    REQUIRE(xe::equals(1.00001f, 1.0f, 0.0001f));
    REQUIRE(xe::equals(-1.00001f, -1.0f, 0.0001f));
    REQUIRE_FALSE(xe::equals(1.001f, 1.0f, 0.0001f));
    REQUIRE_FALSE(xe::equals(-1.001f, -1.0f, 0.0001f));
}
