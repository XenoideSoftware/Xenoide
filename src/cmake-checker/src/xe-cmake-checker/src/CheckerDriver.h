#ifndef XE_CMAKE_CHECKER_DRIVER_H
#define XE_CMAKE_CHECKER_DRIVER_H

#include "CliOptionsParser.h"

#include "xe/cmake/analysis/SemanticModel.h"
#include "xe/cmake/io/FileSystem.h"
#include "xe/cmake/io/ProjectLoader.h"
#include "xe/cmake/rules/RuleRegistry.h"

#include <memory>
#include <string>
#include <vector>

namespace xe::cmake {

    // Orchestrates the whole checking pipeline: loads the project, builds the
    // graph, runs rules, and optionally applies fixes.
    class CheckerDriver {
    public:
        CheckerDriver(const CliOptions &options, const xe::cmake::io::IFileSystem &filesystem, xe::cmake::rules::RuleRegistry &registry);

        // Runs the check. Returns 0 on success, non-zero exit code otherwise.
        int run() const;

    private:
        std::vector<xe::cmake::core::Finding>
        collect_findings(const std::vector<xe::cmake::core::ConcreteSyntaxTree> &files, const xe::cmake::analysis::DirectedDependencyGraph &graph) const;

        const CliOptions &options_;
        const xe::cmake::io::IFileSystem &filesystem_;
        xe::cmake::rules::RuleRegistry &registry_;
    };

} // namespace xe::cmake

#endif // XE_CMAKE_CHECKER_DRIVER_H