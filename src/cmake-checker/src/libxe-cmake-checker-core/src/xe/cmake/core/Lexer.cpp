#include "Lexer.h"

#include <cctype>
#include <string>

namespace xe::cmake::core {
    namespace {

        // Advances over a bracket argument like [[...]] or [=[...]=] starting at the
        // opening bracket. Returns the position one past the closing bracket, or npos
        // when unterminated.
        std::size_t consume_bracket_argument(std::string_view source, std::size_t pos) {
            std::size_t equals = 0;
            std::size_t i = pos + 1;
            while (i < source.size() && source[i] == '=') {
                ++equals;
                ++i;
            }
            if (i >= source.size() || source[i] != '[') {
                return std::string::npos;
            }
            std::string closer = "]";
            closer.append(equals, '=');
            closer.push_back(']');
            const std::size_t content = i + 1;
            const std::size_t end = source.find(closer, content);
            if (end == std::string_view::npos) {
                return std::string::npos;
            }
            return end + closer.size();
        }

        bool is_identifier_char(char c) {
            return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-';
        }

        // Fills line/column coordinates for a span by scanning the buffer.
        void fill_coordinates(std::string_view buffer, SourceSpan &span) {
            std::size_t line = 1;
            std::size_t column = 1;
            const std::size_t limit = span.end_offset < buffer.size() ? span.end_offset : buffer.size();
            for (std::size_t i = 0; i < limit; ++i) {
                if (i == span.start_offset) {
                    span.start_line = line;
                    span.start_column = column;
                }
                if (buffer[i] == '\n') {
                    ++line;
                    column = 1;
                } else {
                    ++column;
                }
            }
            span.end_line = line;
            span.end_column = column;
        }

    } // namespace

    ConcreteSyntaxTree Lexer::parse(std::string_view file_path, std::string_view source) {
        ConcreteSyntaxTree tree{std::string(file_path), std::string(source)};
        const std::string &buffer = tree.source();

        std::size_t pos = 0;
        const std::size_t size = buffer.size();

        auto add_trivia = [&tree](TriviaKind kind, SourceSpan span) {
            StatementNode statement;
            statement.is_command = false;
            statement.trivia.kind = kind;
            statement.trivia.span = span;
            tree.statements().push_back(statement);
        };

        while (pos < size) {
            const char c = buffer[pos];

            // Blank line: two or more consecutive newlines collapse into trivia.
            if (c == '\n') {
                const std::size_t start = pos;
                while (pos < size && buffer[pos] == '\n') {
                    ++pos;
                }
                add_trivia(TriviaKind::BlankLine, SourceSpan{start, pos, 1, 1, 1, 1});
                continue;
            }

            if (c == ' ' || c == '\t' || c == '\r') {
                const std::size_t start = pos;
                while (pos < size && (buffer[pos] == ' ' || buffer[pos] == '\t' || buffer[pos] == '\r')) {
                    ++pos;
                }
                add_trivia(TriviaKind::Whitespace, SourceSpan{start, pos, 1, 1, 1, 1});
                continue;
            }

            if (c == '#') {
                const std::size_t start = pos;
                ++pos;
                // Bracket comment #[[...]] or #=[...]=]
                if (pos < size && buffer[pos] == '[') {
                    const std::size_t end = consume_bracket_argument(buffer, pos);
                    if (end != std::string::npos) {
                        pos = end;
                    } else {
                        pos = size;
                    }
                } else {
                    while (pos < size && buffer[pos] != '\n') {
                        ++pos;
                    }
                }
                add_trivia(TriviaKind::Comment, SourceSpan{start, pos, 1, 1, 1, 1});
                continue;
            }

            // A command starts with an identifier character.
            if (is_identifier_char(c)) {
                const std::size_t name_start = pos;
                while (pos < size && is_identifier_char(buffer[pos])) {
                    ++pos;
                }
                std::string name = buffer.substr(name_start, pos - name_start);

                // Skip whitespace between the command name and the opening paren.
                std::size_t open_paren = pos;
                while (open_paren < size && (buffer[open_paren] == ' ' || buffer[open_paren] == '\t')) {
                    ++open_paren;
                }

                if (open_paren < size && buffer[open_paren] == '(') {
                    CommandNode command;
                    command.name = std::move(name);
                    command.file_path = std::string(file_path);
                    command.name_span = SourceSpan{name_start, pos, 1, 1, 1, 1};
                    command.span.start_offset = name_start;
                    command.span.end_offset = name_start;

                    pos = open_paren + 1;

                    std::size_t argument_index = 0;
                    bool unterminated = false;
                    while (pos < size && buffer[pos] != ')') {
                        if (buffer[pos] == ' ' || buffer[pos] == '\t' || buffer[pos] == '\n' || buffer[pos] == '\r') {
                            ++pos;
                            continue;
                        }

                        if (buffer[pos] == '#') {
                            while (pos < size && buffer[pos] != '\n') {
                                ++pos;
                            }
                            continue;
                        }

                        const std::size_t arg_start = pos;
                        ArgumentNode argument;
                        argument.index = argument_index;

                        if (buffer[pos] == '"') {
                            argument.quote_kind = QuoteKind::Quoted;
                            ++pos;
                            std::string text;
                            while (pos < size && buffer[pos] != '"') {
                                if (buffer[pos] == '\\' && pos + 1 < size) {
                                    const char escaped = buffer[pos + 1];
                                    if (escaped == '"' || escaped == '\\') {
                                        text.push_back(escaped);
                                        pos += 2;
                                        continue;
                                    }
                                }
                                text.push_back(buffer[pos]);
                                ++pos;
                            }
                            if (pos < size) {
                                ++pos; // closing quote
                            } else {
                                unterminated = true;
                            }
                            argument.text = std::move(text);
                        } else if (buffer[pos] == '[') {
                            const std::size_t end = consume_bracket_argument(buffer, pos);
                            if (end == std::string::npos) {
                                unterminated = true;
                                pos = size;
                            } else {
                                // Strip the delimiters.
                                std::size_t equals = 0;
                                while (pos + 1 + equals < size && buffer[pos + 1 + equals] == '=') {
                                    ++equals;
                                }
                                const std::size_t content_start = pos + 2 + equals;
                                const std::size_t content_end = end - (equals + 2);
                                argument.text = buffer.substr(content_start, content_end - content_start);
                                argument.quote_kind = QuoteKind::Bracket;
                                pos = end;
                            }
                        } else {
                            argument.quote_kind = QuoteKind::Raw;
                            std::string text;
                            while (pos < size) {
                                const char cur = buffer[pos];
                                if (cur == ' ' || cur == '\t' || cur == '\n' || cur == '\r' || cur == ')') {
                                    break;
                                }
                                if (cur == '\\' && pos + 1 < size) {
                                    text.push_back(buffer[pos + 1]);
                                    pos += 2;
                                    continue;
                                }
                                text.push_back(cur);
                                ++pos;
                            }
                            argument.text = std::move(text);
                        }

                        argument.span = SourceSpan{arg_start, pos, 1, 1, 1, 1};
                        command.arguments.push_back(argument);
                        ++argument_index;
                    }

                    if (pos < size && buffer[pos] == ')') {
                        ++pos;
                        command.span.end_offset = pos;
                    } else {
                        unterminated = true;
                        command.span.end_offset = size;
                        tree.set_has_errors(true);
                    }
                    if (unterminated) {
                        command.span.end_offset = size;
                        tree.set_has_errors(true);
                    }

                    StatementNode statement;
                    statement.is_command = true;
                    statement.command = std::move(command);
                    tree.statements().push_back(statement);
                    continue;
                }

                // Not a command; treat the identifier run as whitespace-like trivia.
                add_trivia(TriviaKind::Whitespace, SourceSpan{name_start, pos, 1, 1, 1, 1});
                continue;
            }

            // Unknown character: record as whitespace trivia to stay lossless.
            const std::size_t start = pos;
            ++pos;
            add_trivia(TriviaKind::Whitespace, SourceSpan{start, pos, 1, 1, 1, 1});
        }

        // Fill real line/column coordinates on every statement.
        for (StatementNode &statement : tree.statements()) {
            if (statement.is_command) {
                fill_coordinates(buffer, statement.command.name_span);
                fill_coordinates(buffer, statement.command.span);
                for (ArgumentNode &argument : statement.command.arguments) {
                    fill_coordinates(buffer, argument.span);
                }
            } else {
                fill_coordinates(buffer, statement.trivia.span);
            }
        }

        return tree;
    }

} // namespace xe::cmake::core