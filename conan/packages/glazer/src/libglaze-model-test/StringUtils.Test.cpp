#include "glaze/model/StringUtils.h"

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>
#include <vector>

using namespace glaze::model;

TEST_CASE("isCapitalized recognizes single-word CamelCase shapes",
          "[glaze][model][string]") {
    REQUIRE(isCapitalized("Buffer"));
    REQUIRE(isCapitalized("ARB"));
    REQUIRE(isCapitalized("A"));
    REQUIRE(isCapitalized("GL"));
    REQUIRE_FALSE(isCapitalized("BufferTarget")); // multi-word CamelCase is not a single word
    REQUIRE_FALSE(isCapitalized(""));
    REQUIRE_FALSE(isCapitalized("buffer"));
    REQUIRE_FALSE(isCapitalized("bufferARB"));
    REQUIRE_FALSE(isCapitalized("123"));
}

TEST_CASE("splitCapitalized splits multi-word CamelCase", "[glaze][model][string]") {
    REQUIRE(splitCapitalized("BufferTarget") == std::vector<std::string>{"Buffer", "Target"});
    REQUIRE(splitCapitalized("BufferTargetARB") ==
            std::vector<std::string>{"Buffer", "Target", "ARB"});
    REQUIRE(splitCapitalized("Buffer") == std::vector<std::string>{"Buffer"});
    REQUIRE(splitCapitalized("ClearBufferMask") ==
            std::vector<std::string>{"Clear", "Buffer", "Mask"});
    REQUIRE(splitCapitalized("GL") == std::vector<std::string>{"GL"});
}

TEST_CASE("camelCase title-cases each space-separated word", "[glaze][model][string]") {
    REQUIRE(camelCase("buffer") == "Buffer");
    REQUIRE(camelCase("program pipeline") == "ProgramPipeline");
    REQUIRE(camelCase("") == "");
    REQUIRE(camelCase("vertex array") == "VertexArray");
}

TEST_CASE("versionToInt encodes MAJOR.MINOR as MAJOR*10+MINOR", "[glaze][model][string]") {
    REQUIRE(versionToInt("1.0") == 10);
    REQUIRE(versionToInt("3.3") == 33);
    REQUIRE(versionToInt("4.6") == 46);
}

TEST_CASE("versionToInt rejects malformed input", "[glaze][model][string]") {
    REQUIRE_THROWS_AS(versionToInt(""), std::invalid_argument);
    REQUIRE_THROWS_AS(versionToInt("3"), std::invalid_argument);
    REQUIRE_THROWS_AS(versionToInt("3.3.0"), std::invalid_argument);
    REQUIRE_THROWS_AS(versionToInt("a.b"), std::invalid_argument);
}
