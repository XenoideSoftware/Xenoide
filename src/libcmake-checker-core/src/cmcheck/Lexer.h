#pragma once

#include <string>
#include <vector>

namespace cmcheck {

    enum class TokenKind {
        Whitespace,
        Newline,
        Comment,
        Word,
        Quoted,
        Bracket,
        LeftParen,
        RightParen,
        Semicolon,
        EndOfFile,
    };

    struct Token {
        TokenKind kind = TokenKind::EndOfFile;
        std::string text;
        std::size_t offset = 0;
        int line = 1;
        int column = 1;
    };

    class Lexer {
    public:
        std::vector<Token> tokenize(const std::string &content) const;
    };

} // namespace cmcheck