#ifndef XE_CMAKE_RULES_CHECK_RUNNER_H
#define XE_CMAKE_RULES_CHECK_RUNNER_H

#include "RuleRegistry.h"

#include "xe/cmake/analysis/DirectedDependencyGraph.h"
#include "xe/cmake/core/ConcreteSyntaxTree.h"
#include "xe/cmake/core/MutationEngine.h"

#include <string_view>
#include <vector>

namespace xe::cmake::rules {

    // Executes every registered rule against the parsed listfiles and the
    // dependency graph, collecting findings. DSL rules are applied per matched
    // node; script hooks are driven through the ScriptRuleLoader.
    class CheckRunner {
    public:
        explicit CheckRunner(const RuleRegistry &registry) : registry_(registry) {
        }

        // Runs the full check pipeline over the project files.
        std::vector<xe::cmake::core::Finding> run(const std::vector<xe::cmake::core::ConcreteSyntaxTree> &files, const xe::cmake::analysis::DirectedDependencyGraph &graph) const;

    private:
        std::vector<xe::cmake::core::Finding> run_dsl(const std::vector<xe::cmake::core::ConcreteSyntaxTree> &files) const;
        std::vector<xe::cmake::core::Finding>
        run_scripts(const std::vector<xe::cmake::core::ConcreteSyntaxTree> &files, const xe::cmake::analysis::DirectedDependencyGraph &graph) const;

        const RuleRegistry &registry_;
    };

} // namespace xe::cmake::rules

#endif // XE_CMAKE_RULES_CHECK_RUNNER_H