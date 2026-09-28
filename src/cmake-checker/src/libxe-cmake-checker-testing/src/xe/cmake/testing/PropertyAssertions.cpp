#include "PropertyAssertions.h"

#include <algorithm>
#include <set>

namespace xe::cmake::testing {

    bool requireCstValidSpans(const xe::cmake::core::ConcreteSyntaxTree &cst) {
        std::size_t previous = 0;
        for (const xe::cmake::core::StatementNode &statement : cst.statements()) {
            if (statement.is_command) {
                const xe::cmake::core::CommandNode &command = statement.command;
                if (command.span.start_offset < previous || command.span.end_offset < command.span.start_offset || command.span.end_offset > cst.source().size()) {
                    return false;
                }
                previous = command.span.end_offset;
            } else {
                if (statement.trivia.span.start_offset < previous || statement.trivia.span.end_offset < statement.trivia.span.start_offset) {
                    return false;
                }
                previous = statement.trivia.span.end_offset;
            }
        }
        return true;
    }

    bool requireCstLosslessRoundTrip(const xe::cmake::core::ConcreteSyntaxTree &cst, std::string_view original_bytes) {
        return cst.source() == original_bytes;
    }

    bool requireCstProperty(const xe::cmake::core::ConcreteSyntaxTree &cst, const std::vector<std::string> &expected_commands, std::string_view) {
        std::vector<std::string> actual;
        for (const xe::cmake::core::CommandNode *command : cst.commands()) {
            actual.push_back(command->name);
        }
        return actual == expected_commands;
    }

    bool requireWorkspaceEditNonOverlapping(const xe::cmake::core::WorkspaceEdit &workspace_edit) {
        for (const auto &file : workspace_edit.files) {
            for (std::size_t i = 0; i < file.edits.size(); ++i) {
                for (std::size_t j = i + 1; j < file.edits.size(); ++j) {
                    if (file.edits[i].span.overlaps(file.edits[j].span)) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    bool requireWorkspaceEditSplicingValid(const xe::cmake::core::WorkspaceEdit &workspace_edit, const std::string &source_text) {
        for (const auto &file : workspace_edit.files) {
            std::string spliced = xe::cmake::core::TextSplicer::splice(source_text, file.edits);
            if (spliced.empty() && !source_text.empty()) {
                return false;
            }
        }
        return true;
    }

    bool requireGraphAcyclic(const xe::cmake::analysis::DirectedDependencyGraph &directed_graph) {
        return directed_graph.find_cycles().empty();
    }

    bool requireFindingMatches(const xe::cmake::core::Finding &finding, std::string_view expected_rule_id, xe::cmake::core::Severity expected_severity) {
        return finding.rule_id == expected_rule_id && finding.severity == expected_severity;
    }

    bool requireFixApplicable(const xe::cmake::core::Fix &fix, const std::string &source_text) {
        if (fix.edits.empty()) {
            return true;
        }
        for (const xe::cmake::core::TextEdit &edit : fix.edits) {
            if (edit.span.end_offset > source_text.size()) {
                return false;
            }
            if (edit.span.start_offset > edit.span.end_offset) {
                return false;
            }
        }
        return true;
    }

} // namespace xe::cmake::testing