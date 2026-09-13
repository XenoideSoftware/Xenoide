#include "glaze/model/TypeDecl.h"

#include <catch2/catch_test_macros.hpp>

using glaze::model::toCString;
using glaze::model::TypeDecl;

TEST_CASE("toCString renders a plain type", "[glaze][model][typedecl]") {
    TypeDecl decl{"GLuint", false, false};
    REQUIRE(toCString(decl) == "GLuint");
}

TEST_CASE("toCString adds const prefix", "[glaze][model][typedecl]") {
    TypeDecl decl{"GLuint", true, false};
    REQUIRE(toCString(decl) == "const GLuint");
}

TEST_CASE("toCString adds pointer suffix", "[glaze][model][typedecl]") {
    TypeDecl decl{"GLubyte", false, true};
    REQUIRE(toCString(decl) == "GLubyte*");
}

TEST_CASE("toCString combines const and pointer", "[glaze][model][typedecl]") {
    TypeDecl decl{"GLchar", true, true};
    REQUIRE(toCString(decl) == "const GLchar*");
}
