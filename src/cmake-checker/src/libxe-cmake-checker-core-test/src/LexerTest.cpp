#include "xe/cmake/core/Lexer.h"
#include "xe/cmake/testing/CstSyntheticGenerator.h"
#include "xe/cmake/testing/PropertyAssertions.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake::core {

    TEST_CASE("Lexer parses a simple command with raw arguments") {
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", "add_library(foo STATIC src/a.cpp)");
        REQUIRE_FALSE(cst.has_errors());
        REQUIRE(cst.commands().size() == 1);
        const CommandNode &command = *cst.commands()[0];
        REQUIRE(command.name == "add_library");
        REQUIRE(command.argument_count() == 3);
        REQUIRE(command.argument(0).text == "foo");
        REQUIRE(command.argument(0).is_raw());
        REQUIRE(command.argument(1).text == "STATIC");
        REQUIRE(command.argument(2).text == "src/a.cpp");
    }

    TEST_CASE("Lexer handles quoted arguments with escapes") {
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", "set (target \"lib\\\"xe\\\"-core\")");
        REQUIRE_FALSE(cst.has_errors());
        REQUIRE(cst.commands().size() == 1);
        const CommandNode &command = *cst.commands()[0];
        REQUIRE(command.argument_count() == 2);
        REQUIRE(command.argument(1).is_quoted());
        REQUIRE(command.argument(1).text == "lib\"xe\"-core");
    }

    TEST_CASE("Lexer preserves trivia losslessly") {
        const std::string source = "# header comment\n\nset (target \"foo\")\n";
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", source);
        REQUIRE_FALSE(cst.has_errors());
        REQUIRE(testing::requireCstLosslessRoundTrip(cst, source));
        REQUIRE(testing::requireCstValidSpans(cst));
    }

    TEST_CASE("Lexer detects unterminated commands") {
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", "set (target \"foo\"");
        REQUIRE(cst.has_errors());
    }

    TEST_CASE("Lexer parses bracket arguments") {
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", "set (banner [=[hello\nworld]=])");
        REQUIRE_FALSE(cst.has_errors());
        REQUIRE(cst.commands().size() == 1);
        const CommandNode &command = *cst.commands()[0];
        REQUIRE(command.argument(1).is_bracket());
        REQUIRE(command.argument(1).text == "hello\nworld");
    }

    TEST_CASE("Lexer supports commands with a space before the paren") {
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", "project (Foo)");
        REQUIRE_FALSE(cst.has_errors());
        REQUIRE(cst.commands().size() == 1);
        REQUIRE(cst.commands()[0]->name == "project");
    }

    TEST_CASE("Lexer file_path is recorded on commands") {
        const ConcreteSyntaxTree cst = testing::parse_cst("/proj/lib/CMakeLists.txt", "add_library(x)");
        REQUIRE(cst.commands()[0]->file_path == "/proj/lib/CMakeLists.txt");
    }

    TEST_CASE("Lexer handles comments inside commands") {
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", "set (target \"lib\" # inline comment\n    \"src\")\n");
        REQUIRE_FALSE(cst.has_errors());
        REQUIRE(cst.commands().size() == 1);
        REQUIRE(cst.commands()[0]->argument_count() == 3);
        REQUIRE(cst.commands()[0]->argument(2).text == "src");
    }

    TEST_CASE("Lexer handles empty commands and no trailing newline") {
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", "set ()\n");
        REQUIRE_FALSE(cst.has_errors());
        REQUIRE(cst.commands().size() == 1);
        REQUIRE(cst.commands()[0]->argument_count() == 0);
    }

    TEST_CASE("Lexer treats bare identifiers as trivia") {
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", "some_bare_token\n");
        REQUIRE_FALSE(cst.has_errors());
        REQUIRE(cst.commands().empty());
    }

    TEST_CASE("Lexer handles bracket comments") {
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", "#[=[ block comment ]=]\nset (a 1)\n");
        REQUIRE_FALSE(cst.has_errors());
        REQUIRE(cst.commands().size() == 1);
    }

    TEST_CASE("Lexer handles multiple commands across lines") {
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", "add_library(a)\nadd_library(b)\nadd_library(c)\n");
        REQUIRE(cst.commands().size() == 3);
        REQUIRE(cst.commands()[0]->name == "add_library");
        REQUIRE(cst.commands()[2]->argument(0).text == "c");
    }

    TEST_CASE("Lexer raw argument with escaped characters") {
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", "set (a some\\ value)\n");
        REQUIRE_FALSE(cst.has_errors());
        REQUIRE(cst.commands()[0]->argument(1).text == "some value");
    }

    TEST_CASE("Lexer unterminated bracket argument flags errors") {
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", "set (a [=[unterminated)\n");
        REQUIRE(cst.has_errors());
    }

    TEST_CASE("Lexer fill_coordinates computes multi-line spans") {
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", "set (a 1)\nadd_library(lib)\n");
        REQUIRE_FALSE(cst.has_errors());
        const CommandNode *second = cst.commands()[1];
        REQUIRE(second->span.start_line == 2);
        REQUIRE(second->span.start_column == 1);
        REQUIRE(second->span.start_offset < second->arguments[0].span.start_offset);
        // The first command occupies line 1.
        REQUIRE(cst.commands()[0]->span.start_line == 1);
        REQUIRE(cst.commands()[0]->span.end_line == 1);
    }

    TEST_CASE("Lexer fill_coordinates handles trailing newline and boundaries") {
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", "set (a 1)\nset (b 2)\n");
        REQUIRE_FALSE(cst.has_errors());
        const CommandNode *second = cst.commands()[1];
        REQUIRE(second->span.start_line == 2);
        REQUIRE(second->span.end_line == 2);
        // The command span ends just before the trailing newline trivia.
        REQUIRE(second->span.end_offset == cst.source().size() - 1);
        REQUIRE(second->span.end_column >= second->span.start_column);
    }

    TEST_CASE("Lexer coordinate fill for argument on later line") {
        const ConcreteSyntaxTree cst = testing::parse_cst("/v/CMakeLists.txt", "set (target \"lib\"\n    \"extra\")\n");
        REQUIRE_FALSE(cst.has_errors());
        REQUIRE(cst.commands()[0]->argument(0).span.start_line == 1);
        REQUIRE(cst.commands()[0]->argument(1).span.start_line == 1);
        REQUIRE(cst.commands()[0]->argument(2).span.start_line == 2);
        REQUIRE(cst.commands()[0]->argument(2).span.start_column == 5);
    }

} // namespace xe::cmake::core