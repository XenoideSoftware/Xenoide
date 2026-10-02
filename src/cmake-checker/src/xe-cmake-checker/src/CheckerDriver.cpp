#include "CheckerDriver.h"

#include "DiagnosticsReporter.h"

#include "xe/cmake/io/BuildTreeReader.h"
#include "xe/cmake/io/DefaultCmakeParser.h"
#include "xe/cmake/io/SyncWriter.h"
#include "xe/cmake/rules/CheckRunner.h"
#include "xe/cmake/rules/FixConflictResolver.h"

#include <iostream>

namespace xe::cmake {

    CheckerDriver::CheckerDriver(const CliOptions &options, const xe::cmake::io::IFileSystem &filesystem, xe::cmake::rules::RuleRegistry &registry)
        : options_(options),
          filesystem_(filesystem),
          registry_(registry) {
    }

    std::vector<xe::cmake::core::Finding>
    CheckerDriver::collect_findings(const std::vector<xe::cmake::core::ConcreteSyntaxTree> &files, const xe::cmake::analysis::DirectedDependencyGraph &graph) const {
        const xe::cmake::rules::CheckRunner runner(registry_);
        return runner.run(files, graph);
    }

    int CheckerDriver::run() const {
        const xe::cmake::io::DefaultCmakeParser parser(filesystem_);
        const xe::cmake::io::ProjectLoader loader(filesystem_, parser);

        const std::vector<xe::cmake::core::ConcreteSyntaxTree> files = loader.load_project(options_.project);
        if (files.empty()) {
            std::cerr << "xe-cmake-checker: error: no listfiles found under " << options_.project << "\n";
            return 2;
        }

        xe::cmake::analysis::DirectedDependencyGraph graph;
        if (!options_.project_build_dir.empty()) {
            const xe::cmake::io::BuildTreeReader reader(filesystem_);
            reader.read_trace(options_.project_build_dir + "/trace.json", graph);
            reader.read_codemodel(options_.project_build_dir + "/.cmake/api/v1/reply", graph);
        }

        const std::vector<xe::cmake::core::Finding> findings = collect_findings(files, graph);

        DiagnosticsReporter reporter(std::cout);
        if (options_.fix) {
            const xe::cmake::rules::FixConflictResolver resolver;
            const xe::cmake::rules::FixConflictResolver::Result result = resolver.resolve(findings);
            const xe::cmake::io::SyncWriter writer(filesystem_);
            const std::vector<std::string> written = writer.apply(result.workspace);
            for (const std::string &path : written) {
                std::cout << "xe-cmake-checker: fixed " << path << "\n";
            }
            std::vector<xe::cmake::core::Finding> remaining;
            for (const std::size_t index : result.manual_indices) {
                remaining.push_back(findings[index]);
            }
            reporter.report(remaining);
            return reporter.exit_code(remaining, options_.werror);
        }

        if (options_.diff) {
            const xe::cmake::rules::FixConflictResolver resolver;
            const xe::cmake::rules::FixConflictResolver::Result result = resolver.resolve(findings);
            std::vector<xe::cmake::core::Finding> fixable;
            for (const std::size_t index : result.applied_indices) {
                fixable.push_back(findings[index]);
            }
            reporter.report(fixable);
            std::cout << "--- diff mode: " << result.applied_indices.size() << " fixable, " << result.manual_indices.size() << " manual ---\n";
            return reporter.exit_code(findings, options_.werror);
        }

        reporter.report(findings);
        return reporter.exit_code(findings, options_.werror);
    }

} // namespace xe::cmake