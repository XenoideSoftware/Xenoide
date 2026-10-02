#ifndef XE_CMAKE_TESTING_PROPERTY_ASSERTIONS_H
#define XE_CMAKE_TESTING_PROPERTY_ASSERTIONS_H

#include "xe/cmake/analysis/DirectedDependencyGraph.h"
#include "xe/cmake/core/ConcreteSyntaxTree.h"
#include "xe/cmake/core/MutationEngine.h"

#include <string>
#include <string_view>
#include <vector>

namespace xe::cmake::testing {

    // Reusable property assertions complying with docs/TESTING.md. Each helper
    // embeds its target entity in the function name and returns true when the
    // property holds so tests can REQUIRE() the result.

    // All token/node spans are monotonic and within buffer bounds.
    bool requireCstValidSpans(const xe::cmake::core::ConcreteSyntaxTree &cst);

    // Byte-for-byte serialization matches the original source.
    bool requireCstLosslessRoundTrip(const xe::cmake::core::ConcreteSyntaxTree &cst, std::string_view original_bytes);

    // Structural property evaluated against a predicate.
    bool requireCstProperty(const xe::cmake::core::ConcreteSyntaxTree &cst, const std::vector<std::string> &expected_commands, std::string_view description);

    // Multi-file transaction contains zero overlapping intervals per file.
    bool requireWorkspaceEditNonOverlapping(const xe::cmake::core::WorkspaceEdit &workspace_edit);

    // Edits splice cleanly without corrupting boundaries.
    bool requireWorkspaceEditSplicingValid(const xe::cmake::core::WorkspaceEdit &workspace_edit, const std::string &source_text);

    // Dependency graph contains no circular references.
    bool requireGraphAcyclic(const xe::cmake::analysis::DirectedDependencyGraph &directed_graph);

    // Finding carries the expected rule id and severity.
    bool requireFindingMatches(const xe::cmake::core::Finding &finding, std::string_view expected_rule_id, xe::cmake::core::Severity expected_severity);

    // Automated fixits cleanly apply to the target syntax tree.
    bool requireFixApplicable(const xe::cmake::core::Fix &fix, const std::string &source_text);

} // namespace xe::cmake::testing

#endif // XE_CMAKE_TESTING_PROPERTY_ASSERTIONS_H