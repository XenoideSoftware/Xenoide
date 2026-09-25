#include "cmcheck/Lexer.h"

#include <algorithm>

namespace cmcheck {

    namespace {

        bool isBracketOpen(const std::string &content, std::size_t i, std::size_t &eqCount) {
            if (i >= content.size() || content[i] != '[') {
                return false;
            }
            std::size_t j = i + 1;
            while (j < content.size() && content[j] == '=') {
                ++j;
            }
            if (j < content.size() && content[j] == '[') {
                eqCount = j - i - 1;
                return true;
            }
            return false;
        }

        std::size_t findBracketClose(const std::string &content, std::size_t start, std::size_t eqCount) {
            const std::string close = std::string("]") + std::string(eqCount, '=') + "]";
            const std::size_t pos = content.find(close, start);
            if (pos == std::string::npos) {
                return content.size();
            }
            return pos + close.size();
        }

    } // namespace

    std::vector<Token> Lexer::tokenize(const std::string &content) const {
        std::vector<Token> tokens;
        const std::size_t size = content.size();
        std::size_t i = 0;
        int line = 1;
        int column = 1;

        const auto push = [&](TokenKind kind, std::size_t start, int startLine, int startColumn) {
            Token token;
            token.kind = kind;
            token.text = content.substr(start, i - start);
            token.offset = start;
            token.line = startLine;
            token.column = startColumn;
            tokens.push_back(token);
        };

        while (i < size) {
            const char c = content[i];
            if (c == ' ' || c == '\t' || c == '\r') {
                const std::size_t start = i;
                const int startLine = line;
                const int startColumn = column;
                while (i < size && (content[i] == ' ' || content[i] == '\t' || content[i] == '\r')) {
                    ++i;
                    ++column;
                }
                push(TokenKind::Whitespace, start, startLine, startColumn);
            } else if (c == '\n') {
                const int startColumn = column;
                ++i;
                push(TokenKind::Newline, i - 1, line, startColumn);
                ++line;
                column = 1;
            } else if (c == '#') {
                const std::size_t start = i;
                const int startLine = line;
                const int startColumn = column;
                while (i < size && content[i] != '\n') {
                    ++i;
                    ++column;
                }
                push(TokenKind::Comment, start, startLine, startColumn);
            } else if (c == '"') {
                const std::size_t start = i;
                const int startLine = line;
                const int startColumn = column;
                ++i;
                ++column;
                while (i < size) {
                    if (content[i] == '\\') {
                        ++i;
                        ++column;
                        if (i < size) {
                            if (content[i] == '\n') {
                                ++line;
                                column = 1;
                            } else {
                                ++column;
                            }
                            ++i;
                        }
                        continue;
                    }
                    if (content[i] == '"') {
                        ++i;
                        ++column;
                        break;
                    }
                    if (content[i] == '\n') {
                        ++line;
                        column = 1;
                    } else {
                        ++column;
                    }
                    ++i;
                }
                push(TokenKind::Quoted, start, startLine, startColumn);
            } else {
                std::size_t eqCount = 0;
                if (isBracketOpen(content, i, eqCount)) {
                    const std::size_t start = i;
                    const int startLine = line;
                    const int startColumn = column;
                    i = findBracketClose(content, i + 2 + eqCount, eqCount);
                    const std::size_t consumed = i - start;
                    const std::size_t lastNewline = content.rfind('\n', i > 0 ? i - 1 : 0);
                    if (lastNewline == std::string::npos || lastNewline < start) {
                        column += static_cast<int>(consumed);
                    } else {
                        line += static_cast<int>(std::count(content.begin() + static_cast<std::ptrdiff_t>(start), content.begin() + static_cast<std::ptrdiff_t>(i), '\n'));
                        column = static_cast<int>(i - lastNewline);
                    }
                    push(TokenKind::Bracket, start, startLine, startColumn);
                } else if (c == '(') {
                    push(TokenKind::LeftParen, i, line, column);
                    ++i;
                    ++column;
                } else if (c == ')') {
                    push(TokenKind::RightParen, i, line, column);
                    ++i;
                    ++column;
                } else if (c == ';') {
                    push(TokenKind::Semicolon, i, line, column);
                    ++i;
                    ++column;
                } else {
                    const std::size_t start = i;
                    const int startLine = line;
                    const int startColumn = column;
                    while (i < size) {
                        const char w = content[i];
                        if (w == ' ' || w == '\t' || w == '\r' || w == '\n' || w == '(' || w == ')' || w == '#' || w == ';' || w == '"') {
                            break;
                        }
                        std::size_t eq = 0;
                        if (w == '[' && isBracketOpen(content, i, eq)) {
                            break;
                        }
                        ++i;
                        ++column;
                    }
                    push(TokenKind::Word, start, startLine, startColumn);
                }
            }
        }

        push(TokenKind::EndOfFile, i, line, column);
        return tokens;
    }

} // namespace cmcheck