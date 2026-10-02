#ifndef XE_CMAKE_RULES_FIX_CONFLICT_RESOLVER_H
#define XE_CMAKE_RULES_FIX_CONFLICT_RESOLVER_H

#include "xe/cmake/core/MutationEngine.h"

#include <optional>
#include <vector>

namespace xe::cmake::rules {

    // Resolves the set of fixable findings into a single multi-file WorkspaceEdit.
    // Findings are processed per file; edits are validated for overlapping spans
    // and sorted in reverse-offset order to preserve coordinate validity. Findings
    // whose fix cannot be applied cleanly are reported for manual intervention.
    class FixConflictResolver {
    public:
        // Builds a WorkspaceEdit from the findings. Returns the indices of the
        // findings whose fixes were applied; the remaining findings require manual
        // intervention.
        struct Result {
            xe::cmake::core::WorkspaceEdit workspace;
            std::vector<std::size_t> applied_indices;
            std::vector<std::size_t> manual_indices;
        };

        Result resolve(const std::vector<xe::cmake::core::Finding> &findings) const;
    };

} // namespace xe::cmake::rules

#endif // XE_CMAKE_RULES_FIX_CONFLICT_RESOLVER_H