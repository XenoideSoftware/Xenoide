#include "xe/cmake/core/Lexer.h"
#include "xe/cmake/testing/CstSyntheticGenerator.h"
#include "xe/cmake/testing/PropertyAssertions.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake::core {

    TEST_CASE("CST exposes commands in source order") {
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", "set (target \"lib\")\nset (sources \"src/a.cpp\")\nadd_library(${target} ${sources})\n");
        REQUIRE_FALSE(cst.has_errors());
        REQUIRE(cst.commands().size() == 3);
        REQUIRE(cst.commands()[0]->name == "set");
        REQUIRE(cst.commands()[1]->name == "set");
        REQUIRE(cst.commands()[2]->name == "add_library");
        REQUIRE(testing::requireCstValidSpans(cst));
    }

    TEST_CASE("CST statements interleave commands and trivia") {
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", "# comment\nset (a 1)\n\n");
        const auto &statements = cst.statements();
        REQUIRE(statements.size() >= 4);
        REQUIRE_FALSE(statements[0].is_command);
        REQUIRE_FALSE(statements[1].is_command);
        REQUIRE(statements[2].is_command);
        REQUIRE(statements[2].command.name == "set");
    }

    TEST_CASE("CST builder produces valid trees") {
        const ConcreteSyntaxTree cst = testing::CstBuilder().withPath("/v/CMakeLists.txt").withCommand("set", {"target", "libxe-core"}).build();
        REQUIRE(cst.commands().size() == 1);
        REQUIRE(cst.commands()[0]->name == "set");
        REQUIRE(cst.commands()[0]->argument(1).text == "libxe-core");
    }

    TEST_CASE("CST property assertion verifies command sequence") {
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", "set (a 1)\nadd_executable(app main.cpp)\n");
        REQUIRE(testing::requireCstProperty(cst, {"set", "add_executable"}, "command sequence"));
    }

    TEST_CASE("CST round-trip preserves exact bytes") {
        const std::string source = "cmake_minimum_required (VERSION 3.25)\n\nset (target \"libxe-a\")\n";
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", source);
        REQUIRE(testing::requireCstLosslessRoundTrip(cst, source));
    }

} // namespace xe::cmake::core