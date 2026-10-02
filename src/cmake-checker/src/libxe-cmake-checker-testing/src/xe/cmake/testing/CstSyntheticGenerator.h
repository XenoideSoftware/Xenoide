#ifndef XE_CMAKE_TESTING_CST_SYNTHETIC_GENERATOR_H
#define XE_CMAKE_TESTING_CST_SYNTHETIC_GENERATOR_H

#include "xe/cmake/core/ConcreteSyntaxTree.h"

#include <string>
#include <string_view>
#include <vector>

namespace xe::cmake::testing {

    // Parametric builder for synthetic concrete syntax trees.
    class CstBuilder {
    public:
        CstBuilder &withPath(std::string path) {
            path_ = std::move(path);
            return *this;
        }

        CstBuilder &withCommand(std::string name, std::vector<std::string> arguments) {
            xe::cmake::core::CommandNode command;
            command.name = std::move(name);
            command.file_path = path_;
            for (std::size_t i = 0; i < arguments.size(); ++i) {
                xe::cmake::core::ArgumentNode argument;
                argument.text = std::move(arguments[i]);
                argument.index = i;
                argument.quote_kind = xe::cmake::core::QuoteKind::Raw;
                command.arguments.push_back(std::move(argument));
            }
            xe::cmake::core::StatementNode statement;
            statement.is_command = true;
            statement.command = std::move(command);
            statements_.push_back(std::move(statement));
            return *this;
        }

        CstBuilder &withQuotedCommand(std::string name, std::vector<std::string> arguments) {
            xe::cmake::core::CommandNode command;
            command.name = std::move(name);
            command.file_path = path_;
            for (std::size_t i = 0; i < arguments.size(); ++i) {
                xe::cmake::core::ArgumentNode argument;
                argument.text = std::move(arguments[i]);
                argument.index = i;
                argument.quote_kind = xe::cmake::core::QuoteKind::Quoted;
                command.arguments.push_back(std::move(argument));
            }
            xe::cmake::core::StatementNode statement;
            statement.is_command = true;
            statement.command = std::move(command);
            statements_.push_back(std::move(statement));
            return *this;
        }

        CstBuilder &withTrivia(std::string comment) {
            (void)comment;
            xe::cmake::core::TriviaNode trivia;
            trivia.kind = xe::cmake::core::TriviaKind::Comment;
            xe::cmake::core::StatementNode statement;
            statement.is_command = false;
            statement.trivia = trivia;
            statements_.push_back(std::move(statement));
            return *this;
        }

        xe::cmake::core::ConcreteSyntaxTree build() const {
            xe::cmake::core::ConcreteSyntaxTree tree(path_, source_);
            for (const xe::cmake::core::StatementNode &statement : statements_) {
                tree.statements().push_back(statement);
            }
            return tree;
        }

    private:
        std::string path_ = "/virtual/CMakeLists.txt";
        std::string source_;
        std::vector<xe::cmake::core::StatementNode> statements_;
    };

    // Convenience: builds a CST from a real listfile buffer.
    xe::cmake::core::ConcreteSyntaxTree parse_cst(std::string_view path, std::string_view source);

} // namespace xe::cmake::testing

#endif // XE_CMAKE_TESTING_CST_SYNTHETIC_GENERATOR_H