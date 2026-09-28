#include "RefactoringPrimitives.h"

#include <set>

namespace xe::cmake::core {

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    WorkspaceEdit RenameSymbolRefactoring::rename_target(const ConcreteSyntaxTree &cst, std::string_view old_name, std::string_view new_name) {
        WorkspaceEdit workspace;
        for (const CommandNode *command : cst.commands()) {
            const bool is_target_declaration = command->name == "add_library" || command->name == "add_executable";
            if (is_target_declaration && command->argument_count() > 0 && command->argument(0).text == old_name) {
                workspace.add_edit(command->file_path, TextEdit::replace(command->argument(0).span, new_name));
            }
            if (command->name == "target_link_libraries") {
                for (const ArgumentNode &argument : command->arguments) {
                    if (argument.index > 0 && argument.text == old_name) {
                        workspace.add_edit(command->file_path, TextEdit::replace(argument.span, new_name));
                    }
                }
            }
            if (command->name == "set" && command->argument_count() >= 2 && command->argument(0).text == "target" && command->argument(1).text == old_name) {
                workspace.add_edit(command->file_path, TextEdit::replace(command->argument(1).span, "\"" + std::string(new_name) + "\""));
            }
        }
        return workspace;
    }

    std::vector<TextEdit>
    ExtractFunctionRefactoring::extract_statements(const ConcreteSyntaxTree &cst, const std::vector<const CommandNode *> &statements, std::string_view function_name) {
        (void)cst;
        std::vector<TextEdit> edits;
        if (statements.empty()) {
            return edits;
        }
        std::size_t first = statements.front()->span.start_offset;
        std::size_t last = statements.back()->span.end_offset;
        edits.push_back(TextEdit::replace(SourceSpan{first, last, 1, 1, 1, 1}, "function(" + std::string(function_name) + ")\nendfunction()\n"));
        return edits;
    }

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    std::vector<TextEdit> InlineFunctionRefactoring::inline_function(const ConcreteSyntaxTree &cst, std::string_view function_name, std::string_view body) {
        (void)cst;
        std::vector<TextEdit> edits;
        for (const CommandNode *command : cst.commands()) {
            if (command->name == function_name) {
                edits.push_back(TextEdit::replace(command->span, std::string(body)));
            }
        }
        return edits;
    }

} // namespace xe::cmake::core