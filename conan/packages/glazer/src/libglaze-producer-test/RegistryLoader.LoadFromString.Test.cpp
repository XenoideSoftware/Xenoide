#include "MiniRegistry.h"

#include "glaze/producer/ParseError.h"
#include "glaze/producer/RegistryLoader.h"

#include <catch2/catch_test_macros.hpp>

using glaze::producer::ParseError;
using glaze::producer::RegistryLoader;
using glaze::testfixtures::kMiniXml;
using glaze::testfixtures::loadMiniRegistry;

TEST_CASE("loadFromString parses MINI_XML into a populated Registry",
          "[glaze][producer][loader]") {
    const auto registry = loadMiniRegistry();
    REQUIRE(registry.types().size() == 9);
    REQUIRE(registry.enumGroups().size() == 4);
    REQUIRE(registry.commands().size() >= 20);
    REQUIRE(registry.features().size() == 6);
    REQUIRE(registry.extensions().size() == 4);
}

TEST_CASE("loadFromString throws ParseError for malformed XML",
          "[glaze][producer][loader][errors]") {
    REQUIRE_THROWS_AS(RegistryLoader{}.loadFromString("<registry><unterminated"), ParseError);
}

TEST_CASE("loadFromString throws ParseError when the root element is wrong",
          "[glaze][producer][loader][errors]") {
    REQUIRE_THROWS_AS(RegistryLoader{}.loadFromString("<?xml version=\"1.0\"?><root/>"),
                      ParseError);
}
