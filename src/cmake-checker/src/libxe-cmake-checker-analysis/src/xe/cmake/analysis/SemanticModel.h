#ifndef XE_CMAKE_ANALYSIS_SEMANTIC_MODEL_H
#define XE_CMAKE_ANALYSIS_SEMANTIC_MODEL_H

#include "DirectedDependencyGraph.h"

#include "xe/cmake/core/ConcreteSyntaxTree.h"

#include <string>
#include <string_view>
#include <vector>

namespace xe::cmake::analysis {

    // A target declaration extracted from a listfile.
    struct TargetDeclaration {
        std::string name;
        std::string kind; // "library" | "executable"
        std::string file_path;
        xe::cmake::core::SourceSpan span;
    };

    // Semantic model mapping parsed listfiles to the facts the checkers need:
    // target declarations, source sets, include directories and link commands.
    class SemanticModel {
    public:
        void add_target(TargetDeclaration target);
        void add_source(std::string target_name, std::string file_path, xe::cmake::core::SourceSpan span);
        void add_include_directory(std::string target_name, std::string directory, xe::cmake::core::SourceSpan span);
        void add_link_command(std::string target_name, std::string dependency, std::string scope, xe::cmake::core::SourceSpan span);
        void add_alias(std::string alias_name, std::string target_name);
        void add_file(xe::cmake::core::ConcreteSyntaxTree tree);

        const std::vector<TargetDeclaration> &targets() const {
            return targets_;
        }

        const std::vector<std::string> &files() const {
            return files_;
        }

        std::vector<TargetDeclaration> find_target(std::string_view name) const;

        // Builds a directed dependency graph from the recorded link commands.
        DirectedDependencyGraph build_graph() const;

        std::size_t target_count() const {
            return targets_.size();
        }

    private:
        std::vector<TargetDeclaration> targets_;
        std::vector<std::string> files_;
        std::vector<std::pair<std::string, std::string>> link_commands_;
        std::vector<std::pair<std::string, std::string>> aliases_;
    };

} // namespace xe::cmake::analysis

#endif // XE_CMAKE_ANALYSIS_SEMANTIC_MODEL_H