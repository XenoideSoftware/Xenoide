#include "xe/cmake/core/QueryPrimitives.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake::core {

    TEST_CASE("StringPrimitives regex_match performs exact matching") {
        REQUIRE(StringPrimitives::regex_match("libxe-core", "^libxe-.*"));
        REQUIRE_FALSE(StringPrimitives::regex_match("xe-core", "^libxe-.*"));
        REQUIRE_FALSE(StringPrimitives::regex_match("libxe-core", "["));
    }

    TEST_CASE("StringPrimitives regex_search performs substring matching") {
        REQUIRE(StringPrimitives::regex_search("src/engine/CMakeLists.txt", "engine"));
        REQUIRE_FALSE(StringPrimitives::regex_search("src/ide/CMakeLists.txt", "engine"));
    }

    TEST_CASE("StringPrimitives string helpers") {
        REQUIRE(StringPrimitives::str_contains("target_link_libraries", "link"));
        REQUIRE_FALSE(StringPrimitives::str_contains("add_library", "link"));
        REQUIRE(StringPrimitives::str_starts_with("libxe-core", "libxe-"));
        REQUIRE_FALSE(StringPrimitives::str_starts_with("xe-core", "libxe-"));
        REQUIRE(StringPrimitives::str_ends_with("libxe-core", "-core"));
        REQUIRE_FALSE(StringPrimitives::str_ends_with("libxe-core", "-test"));
    }

    TEST_CASE("StringPrimitives split") {
        const std::vector<std::string> parts = StringPrimitives::str_split("a;b;c", ";");
        REQUIRE(parts.size() == 3);
        REQUIRE(parts[0] == "a");
        REQUIRE(parts[1] == "b");
        REQUIRE(parts[2] == "c");
    }

    TEST_CASE("StringPrimitives casing") {
        REQUIRE(StringPrimitives::to_lower("PUBLIC") == "public");
        REQUIRE(StringPrimitives::to_upper("private") == "PRIVATE");
    }

} // namespace xe::cmake::core