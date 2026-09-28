#ifndef XE_CMAKE_CORE_REFACTORING_PRIMITIVES_H
#define XE_CMAKE_CORE_REFACTORING_PRIMITIVES_H

#include "ConcreteSyntaxTree.h"
#include "MutationEngine.h"

#include <string>
#include <string_view>
#include <vector>

namespace xe::cmake::core {

    // C++ refactoring building blocks. These primitives compute WorkspaceEdits
    // that a future xe-cmake-refactor CLI orchestrates through ChaiScript. They
    // never mutate a buffer directly; they only produce edits.
    class RenameSymbolRefactoring {
    public:
        // Produces edits that rename every occurrence of `old_name` used as a
        // target identifier (the first argument of add_library/add_executable, and
        // any argument of target_link_libraries matching the old name).
        static WorkspaceEdit rename_target(const ConcreteSyntaxTree &cst, std::string_view old_name, std::string_view new_name);
    };

    class ExtractFunctionRefactoring {
    public:
        // Produces an edit that replaces a contiguous statement range with a
        // function(...) definition plus a call site. Placeholder for the future
        // refactoring CLI.
        static std::vector<TextEdit> extract_statements(const ConcreteSyntaxTree &cst, const std::vector<const CommandNode *> &statements, std::string_view function_name);
    };

    class InlineFunctionRefactoring {
    public:
        // Produces edits that inline a function body into its call sites with
        // parameter substitution. Placeholder for the future refactoring CLI.
        static std::vector<TextEdit> inline_function(const ConcreteSyntaxTree &cst, std::string_view function_name, std::string_view body);
    };

} // namespace xe::cmake::core

#endif // XE_CMAKE_CORE_REFACTORING_PRIMITIVES_H