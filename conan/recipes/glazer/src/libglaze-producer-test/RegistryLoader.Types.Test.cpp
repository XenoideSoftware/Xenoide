#include "MiniRegistry.h"

#include <catch2/catch_test_macros.hpp>

using glaze::testfixtures::loadMiniRegistry;

TEST_CASE("Type parser extracts inline typedef names", "[glaze][producer][types]") {
    const auto registry = loadMiniRegistry();
    REQUIRE(registry.findType("GLuint") != nullptr);
    REQUIRE(registry.findType("GLenum") != nullptr);
    REQUIRE(registry.findType("GLbitfield") != nullptr);
    REQUIRE(registry.findType("GLubyte") != nullptr);
    REQUIRE(registry.findType("Nonexistent") == nullptr);
}

TEST_CASE("Type parser preserves the full C definition text", "[glaze][producer][types]") {
    const auto registry = loadMiniRegistry();
    const auto *glUint = registry.findType("GLuint");
    REQUIRE(glUint != nullptr);
    // The definition includes the full text including the nested <name>.
    REQUIRE(glUint->cDefinition.find("typedef unsigned int") != std::string::npos);
    REQUIRE(glUint->cDefinition.find("GLuint") != std::string::npos);
}
