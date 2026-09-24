#include "glaze/cppgen/detail/CppKeywords.h"

#include <catch2/catch_test_macros.hpp>

using glaze::cppgen::detail::cppKeywords;
using glaze::cppgen::detail::sanitizeMethodName;

TEST_CASE("cppKeywords contains the canonical subset from cpp_generator.py",
          "[glaze][cppgen][keywords]") {
    const auto &kws = cppKeywords();
    // Sanity-check a handful of representative entries.
    REQUIRE(kws.count("delete") == 1);
    REQUIRE(kws.count("new") == 1);
    REQUIRE(kws.count("class") == 1);
    REQUIRE(kws.count("template") == 1);
    REQUIRE(kws.count("operator") == 1);
    REQUIRE(kws.count("return") == 1);
    REQUIRE(kws.count("namespace") == 1);
    REQUIRE(kws.count("private") == 1);
    // And one that definitely should not be in the set.
    REQUIRE(kws.count("clear") == 0);
    REQUIRE(kws.count("bindBuffer") == 0);
}

TEST_CASE("sanitizeMethodName appends '_' only to reserved names",
          "[glaze][cppgen][keywords]") {
    REQUIRE(sanitizeMethodName(std::string{"delete"}) == "delete_");
    REQUIRE(sanitizeMethodName(std::string{"new"}) == "new_");
    REQUIRE(sanitizeMethodName(std::string{"operator"}) == "operator_");
    REQUIRE(sanitizeMethodName(std::string{"return"}) == "return_");

    // Unchanged for normal GL-derived identifiers.
    REQUIRE(sanitizeMethodName(std::string{"clear"}) == "clear");
    REQUIRE(sanitizeMethodName(std::string{"bindBuffer"}) == "bindBuffer");
    REQUIRE(sanitizeMethodName(std::string{"texImage2D"}) == "texImage2D");
    REQUIRE(sanitizeMethodName(std::string{""}) == "");
}

TEST_CASE("sanitizeMethodName string_view overload matches the string one",
          "[glaze][cppgen][keywords]") {
    const std::string_view deleteView{"delete"};
    const std::string_view clearView{"clear"};
    REQUIRE(sanitizeMethodName(deleteView) == "delete_");
    REQUIRE(sanitizeMethodName(clearView) == "clear");
}
