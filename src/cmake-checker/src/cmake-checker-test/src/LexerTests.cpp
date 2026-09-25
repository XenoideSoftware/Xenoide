#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include <cmcheck/Lexer.h>

using namespace cmcheck;

namespace {

    std::vector<Token> tokenize(const std::string &content) {
        return Lexer().tokenize(content);
    }

    int countTokens(const std::vector<Token> &tokens, TokenKind kind) {
        int count = 0;
        for (const Token &token : tokens) {
            if (token.kind == kind) {
                ++count;
            }
        }
        return count;
    }

} // namespace

TEST_CASE("Lexer tokenizes a simple command invocation", "[lexer]") {
    const auto tokens = tokenize("set(target \"xe-core\")\n");

    REQUIRE(tokens.size() >= 7);
    CHECK(tokens[0].kind == TokenKind::Word);
    CHECK(tokens[0].text == "set");
    CHECK(tokens[0].line == 1);
    CHECK(tokens[0].column == 1);
    CHECK(tokens[1].kind == TokenKind::LeftParen);
    CHECK(tokens[2].kind == TokenKind::Word);
    CHECK(tokens[2].text == "target");
    CHECK(tokens[3].kind == TokenKind::Whitespace);
    CHECK(tokens[4].kind == TokenKind::Quoted);
    CHECK(tokens[4].text == "\"xe-core\"");
    CHECK(tokens[5].kind == TokenKind::RightParen);
    CHECK(tokens[6].kind == TokenKind::Newline);
}

TEST_CASE("Lexer tracks line and column across newlines", "[lexer]") {
    const auto tokens = tokenize("a\n  b");

    const Token *wordB = nullptr;
    for (const Token &token : tokens) {
        if (token.kind == TokenKind::Word && token.text == "b") {
            wordB = &token;
        }
    }
    REQUIRE(wordB != nullptr);
    CHECK(wordB->line == 2);
    CHECK(wordB->column == 3);
}

TEST_CASE("Lexer handles comments until end of line", "[lexer]") {
    const auto tokens = tokenize("set(x 1) # a comment\n");
    const Token *comment = nullptr;
    for (const Token &token : tokens) {
        if (token.kind == TokenKind::Comment) {
            comment = &token;
        }
    }
    REQUIRE(comment != nullptr);
    CHECK(comment->text == "# a comment");
    CHECK(comment->line == 1);
}

TEST_CASE("Lexer handles bracket arguments", "[lexer]") {
    const auto tokens = tokenize("set(x [[multi\nline]])");
    const Token *bracket = nullptr;
    for (const Token &token : tokens) {
        if (token.kind == TokenKind::Bracket) {
            bracket = &token;
        }
    }
    REQUIRE(bracket != nullptr);
    CHECK(bracket->text == "[[multi\nline]]");
}

TEST_CASE("Lexer handles escaped quotes inside quoted arguments", "[lexer]") {
    const auto tokens = tokenize("set(x \"a \\\"quoted\\\" b\")");
    const Token *quoted = nullptr;
    for (const Token &token : tokens) {
        if (token.kind == TokenKind::Quoted) {
            quoted = &token;
        }
    }
    REQUIRE(quoted != nullptr);
    CHECK(quoted->text == "\"a \\\"quoted\\\" b\"");
}

TEST_CASE("Lexer separates words by semicolons", "[lexer]") {
    const auto tokens = tokenize("set(x a;b)");
    REQUIRE(countTokens(tokens, TokenKind::Semicolon) == 1);
    const Token *wordA = nullptr;
    const Token *wordB = nullptr;
    for (const Token &token : tokens) {
        if (token.kind == TokenKind::Word && token.text == "a") {
            wordA = &token;
        }
        if (token.kind == TokenKind::Word && token.text == "b") {
            wordB = &token;
        }
    }
    REQUIRE(wordA != nullptr);
    REQUIRE(wordB != nullptr);
}

TEST_CASE("Lexer reports whitespace tokens for indentation", "[lexer]") {
    const auto tokens = tokenize("    set(x 1)");
    REQUIRE(tokens.size() >= 2);
    CHECK(tokens[0].kind == TokenKind::Whitespace);
    CHECK(tokens[0].text == "    ");
    CHECK(tokens[0].column == 1);
}

TEST_CASE("Lexer ends with an end-of-file token", "[lexer]") {
    const auto tokens = tokenize("x");
    REQUIRE(tokens.back().kind == TokenKind::EndOfFile);
}