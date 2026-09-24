#include "glaze/model/CommandParam.h"

#include <catch2/catch_test_macros.hpp>

using namespace glaze::model;

namespace {

CommandParam makeParam(std::string name, std::vector<std::string> parts) {
    CommandParam p;
    p.name = std::move(name);
    p.typeParts = std::move(parts);
    return p;
}

} // namespace

TEST_CASE("baseType returns the first non-qualifier token", "[glaze][model][param]") {
    const auto p = makeParam("buffers", {"const", "GLuint", "*"});
    REQUIRE(baseType(p).has_value());
    REQUIRE(*baseType(p) == "GLuint");
}

TEST_CASE("baseType skips 'void'", "[glaze][model][param]") {
    const auto p = makeParam("data", {"const", "void", "*"});
    REQUIRE_FALSE(baseType(p).has_value());
}

TEST_CASE("pointerIndirection counts '*' characters across tokens", "[glaze][model][param]") {
    REQUIRE(pointerIndirection(makeParam("a", {"GLuint"})) == 0);
    REQUIRE(pointerIndirection(makeParam("a", {"GLuint", "*"})) == 1);
    REQUIRE(pointerIndirection(makeParam("a", {"GLuint", "**"})) == 2);
    REQUIRE(pointerIndirection(makeParam("a", {"const", "GLuint", "*"})) == 1);
}

TEST_CASE("isConst, isPointer, isVoid track their token markers", "[glaze][model][param]") {
    const auto constPtr = makeParam("buffers", {"const", "GLuint", "*"});
    REQUIRE(isConst(constPtr));
    REQUIRE(isPointer(constPtr));
    REQUIRE_FALSE(isVoid(constPtr));

    const auto voidPtr = makeParam("data", {"const", "void", "*"});
    REQUIRE(isConst(voidPtr));
    REQUIRE(isPointer(voidPtr));
    REQUIRE(isVoid(voidPtr));

    const auto scalar = makeParam("count", {"GLsizei"});
    REQUIRE_FALSE(isConst(scalar));
    REQUIRE_FALSE(isPointer(scalar));
    REQUIRE_FALSE(isVoid(scalar));
}

TEST_CASE("hasGroup and hasClass check optional attributes", "[glaze][model][param]") {
    auto p = makeParam("target", {"GLenum"});
    REQUIRE_FALSE(hasGroup(p));
    REQUIRE_FALSE(hasClass(p));

    p.group = "BufferTarget";
    p.classStr = "buffer";
    REQUIRE(hasGroup(p));
    REQUIRE(hasClass(p));

    p.group = "";
    p.classStr = "";
    REQUIRE_FALSE(hasGroup(p));
    REQUIRE_FALSE(hasClass(p));
}
