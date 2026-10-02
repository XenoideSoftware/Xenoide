#ifndef XE_CMAKE_CORE_CONCRETE_SYNTAX_TREE_H
#define XE_CMAKE_CORE_CONCRETE_SYNTAX_TREE_H

#include "SourceSpan.h"
#include "Trivia.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace xe::cmake::core {

    enum class QuoteKind : uint8_t {
        Raw,
        Quoted,
        Bracket,
    };

    struct ArgumentNode;

    // A single CMake command statement such as
    //   set (target "libxe-core")
    // The statement owns no memory; it references the owning listfile buffer via
    // byte spans.
    struct CommandNode {
        std::string name;
        SourceSpan name_span;
        SourceSpan span;
        std::string file_path;
        std::vector<ArgumentNode> arguments;
        const void *parent = nullptr;

        std::string_view name_view() const {
            return name;
        }

        std::size_t argument_count() const {
            return arguments.size();
        }

        const ArgumentNode &argument(std::size_t index) const;
    };

    struct ArgumentNode {
        QuoteKind quote_kind = QuoteKind::Raw;
        SourceSpan span;
        std::size_t index = 0;
        std::string text;

        bool is_quoted() const {
            return quote_kind == QuoteKind::Quoted;
        }

        bool is_raw() const {
            return quote_kind == QuoteKind::Raw;
        }

        bool is_bracket() const {
            return quote_kind == QuoteKind::Bracket;
        }
    };

    inline const ArgumentNode &CommandNode::argument(std::size_t index) const {
        return arguments[index];
    }

    // One statement of a listfile: either a command or pure trivia (comment /
    // whitespace / blank line).
    struct StatementNode {
        bool is_command = false;
        CommandNode command;
        TriviaNode trivia;
    };

    // A block is a command statement plus its nested statements (e.g. if/endif,
    // foreach/endforeach). In the minimal in-house parser, blocks are represented
    // as flat statements; this type is a view for future full parser integration.
    struct BlockNode {
        SourceSpan span;
        std::vector<StatementNode> statements;
    };

    // Lossless concrete syntax tree over a single listfile buffer. The buffer is
    // owned by the tree; every node references it through byte-exact spans.
    class ConcreteSyntaxTree {
    public:
        ConcreteSyntaxTree() = default;

        ConcreteSyntaxTree(std::string file_path, std::string source) : file_path_(std::move(file_path)), source_(std::move(source)) {
        }

        const std::string &file_path() const {
            return file_path_;
        }

        const std::string &source() const {
            return source_;
        }

        // Top-level statements in source order, including trivia.
        const std::vector<StatementNode> &statements() const {
            return statements_;
        }

        std::vector<StatementNode> &statements() {
            return statements_;
        }

        // All commands (top-level, in source order).
        std::vector<const CommandNode *> commands() const;

        // Whether the parse completed without syntax errors.
        bool has_errors() const {
            return has_errors_;
        }

        void set_has_errors(bool value) {
            has_errors_ = value;
        }

    private:
        std::string file_path_;
        std::string source_;
        std::vector<StatementNode> statements_;
        bool has_errors_ = false;
    };

} // namespace xe::cmake::core

#endif // XE_CMAKE_CORE_CONCRETE_SYNTAX_TREE_H