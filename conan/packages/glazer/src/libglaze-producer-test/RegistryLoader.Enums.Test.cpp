#include "MiniRegistry.h"

#include <catch2/catch_test_macros.hpp>

using glaze::testfixtures::loadMiniRegistry;

TEST_CASE("Enum parser populates groups and per-enum lookups",
          "[glaze][producer][enums]") {
    const auto registry = loadMiniRegistry();
    REQUIRE(registry.findEnum("GL_COLOR_BUFFER_BIT") != nullptr);
    REQUIRE(registry.findEnum("GL_ARRAY_BUFFER") != nullptr);
    REQUIRE(registry.findEnum("GL_FALSE") != nullptr);
    REQUIRE(registry.findEnum("GL_SAMPLES_PASSED") != nullptr);
    REQUIRE(registry.findEnum("NonExistent") == nullptr);

    const auto *colorBit = registry.findEnum("GL_COLOR_BUFFER_BIT");
    REQUIRE(colorBit != nullptr);
    REQUIRE(colorBit->value == "0x00004000");
    REQUIRE(colorBit->groups.size() == 1);
    REQUIRE(colorBit->groups.front() == "ClearBufferMask");
}

TEST_CASE("Enum parser flags bitmask groups", "[glaze][producer][enums]") {
    const auto registry = loadMiniRegistry();
    REQUIRE(registry.isBitmaskGroup("ClearBufferMask"));
    REQUIRE_FALSE(registry.isBitmaskGroup("BufferTargetARB"));
    REQUIRE_FALSE(registry.isBitmaskGroup("QueryTarget"));
}

TEST_CASE("Enum parser groups enums by group name", "[glaze][producer][enums]") {
    const auto registry = loadMiniRegistry();
    const auto *bufferTargets = registry.enumsForGroup("BufferTargetARB");
    REQUIRE(bufferTargets != nullptr);
    REQUIRE(bufferTargets->size() == 2);
    REQUIRE(registry.enumsForGroup("NoSuchGroup") == nullptr);
}
