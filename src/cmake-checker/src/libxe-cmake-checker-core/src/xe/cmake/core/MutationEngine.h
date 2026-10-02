#ifndef XE_CMAKE_CORE_MUTATION_ENGINE_H
#define XE_CMAKE_CORE_MUTATION_ENGINE_H

#include "SourceSpan.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace xe::cmake::core {

    // A single atomic text mutation over a file buffer. The span refers to the
    // original buffer offsets; edits are sorted in reverse-offset order before
    // splicing so offsets stay valid.
    struct TextEdit {
        enum class Kind : uint8_t {
            Replace,
            InsertBefore,
            InsertAfter,
            Remove,
        };

        Kind kind = Kind::Replace;
        SourceSpan span;
        std::string new_text;

        static TextEdit replace(SourceSpan span, std::string_view new_text) {
            TextEdit edit;
            edit.kind = Kind::Replace;
            edit.span = span;
            edit.new_text = std::string(new_text);
            return edit;
        }

        static TextEdit insert_before(std::size_t offset, std::string_view text) {
            TextEdit edit;
            edit.kind = Kind::InsertBefore;
            edit.span = SourceSpan{offset, offset, 1, 1, 1, 1};
            edit.new_text = std::string(text);
            return edit;
        }

        static TextEdit insert_after(std::size_t offset, std::string_view text) {
            TextEdit edit;
            edit.kind = Kind::InsertAfter;
            edit.span = SourceSpan{offset, offset, 1, 1, 1, 1};
            edit.new_text = std::string(text);
            return edit;
        }

        static TextEdit remove(SourceSpan span) {
            TextEdit edit;
            edit.kind = Kind::Remove;
            edit.span = span;
            return edit;
        }

        std::size_t anchor_offset() const {
            return span.start_offset;
        }
    };

    // An atomic, multi-file transaction of text edits.
    struct WorkspaceEdit {
        struct FileEdit {
            std::string file_path;
            std::vector<TextEdit> edits;
        };

        std::vector<FileEdit> files;

        void add_edit(std::string file_path, TextEdit edit) {
            for (FileEdit &file : files) {
                if (file.file_path == file_path) {
                    file.edits.push_back(edit);
                    return;
                }
            }
            FileEdit file;
            file.file_path = std::move(file_path);
            file.edits.push_back(edit);
            files.push_back(std::move(file));
        }

        bool empty() const {
            for (const FileEdit &file : files) {
                if (!file.edits.empty()) {
                    return false;
                }
            }
            return true;
        }
    };

    // Splicer applies a WorkspaceEdit over an original buffer, producing the
    // mutated buffer. Edits are sorted in reverse-anchor order so that later
    // (higher-offset) edits are applied first, keeping original offsets valid.
    class TextSplicer {
    public:
        static std::string splice(const std::string &source, const std::vector<TextEdit> &edits);
    };

    // Automated fix attached to a Finding. Strictly optional.
    struct Fix {
        std::string description;
        std::vector<TextEdit> edits;

        Fix() = default;

        explicit Fix(std::string description_) : description(std::move(description_)) {
        }

        void add_edit(TextEdit edit) {
            edits.push_back(std::move(edit));
        }
    };

    // Diagnostic severity levels.
    enum class Severity : uint8_t {
        Info,
        Warn,
        Error,
    };

    // A single diagnostic produced by a rule check.
    struct Finding {
        std::string rule_id;
        Severity severity = Severity::Warn;
        std::string message;
        std::string file_path;
        SourceSpan span;
        std::optional<Fix> fix;

        bool has_fix() const {
            return fix.has_value();
        }
    };

    // A declarative rule severity override ("error" | "warn" | "info" | "off").
    enum class RuleSeverity : uint8_t {
        Error,
        Warn,
        Info,
        Off,
    };

} // namespace xe::cmake::core

#endif // XE_CMAKE_CORE_MUTATION_ENGINE_H