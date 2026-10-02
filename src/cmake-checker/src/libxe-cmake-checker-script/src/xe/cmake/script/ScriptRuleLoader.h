#ifndef XE_CMAKE_SCRIPT_SCRIPT_RULE_LOADER_H
#define XE_CMAKE_SCRIPT_SCRIPT_RULE_LOADER_H

#include "ScriptBindings.h"
#include "ScriptEngineFacade.h"

#include "xe/cmake/analysis/DirectedDependencyGraph.h"
#include "xe/cmake/core/ConcreteSyntaxTree.h"
#include "xe/cmake/core/MutationEngine.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace xe::cmake::script {

    // Discovers and loads custom *.chai rule files, then drives the standard
    // hooks (check_file, check_command, check_project) against parsed listfiles
    // and the dependency graph. Script hooks report findings through the context.
    class ScriptRuleLoader {
    public:
        ScriptRuleLoader();

        // Loads a script source and makes its hooks available.
        void load(std::string_view source);

        // Runs all loaded scripts' check_file hook against every file.
        std::vector<xe::cmake::core::Finding> run_check_file(const std::vector<xe::cmake::core::ConcreteSyntaxTree> &files) const;

        // Runs all loaded scripts' check_command hook against every command.
        std::vector<xe::cmake::core::Finding> run_check_command(const std::vector<xe::cmake::core::ConcreteSyntaxTree> &files) const;

        // Runs all loaded scripts' check_project hook once with the full graph.
        std::vector<xe::cmake::core::Finding>
        run_check_project(const std::vector<xe::cmake::core::ConcreteSyntaxTree> &files, const xe::cmake::analysis::DirectedDependencyGraph &graph) const;

    private:
        struct HookSet {
            bool has_check_file = false;
            bool has_check_command = false;
            bool has_check_project = false;
        };

        // Evaluates a script and records which hooks it defines.
        void load_script(std::string_view source);

        std::vector<HookSet> hooks_;
        ScriptEngineFacade facade_;
    };

} // namespace xe::cmake::script

#endif // XE_CMAKE_SCRIPT_SCRIPT_RULE_LOADER_H