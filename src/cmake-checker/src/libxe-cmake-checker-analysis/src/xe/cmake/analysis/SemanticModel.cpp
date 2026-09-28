#include "SemanticModel.h"

#include <algorithm>

namespace xe::cmake::analysis {

    void SemanticModel::add_target(TargetDeclaration target) {
        targets_.push_back(std::move(target));
    }

    void SemanticModel::add_source(std::string, std::string, xe::cmake::core::SourceSpan) {
    }

    void SemanticModel::add_include_directory(std::string, std::string, xe::cmake::core::SourceSpan) {
    }

    void SemanticModel::add_link_command(std::string target_name, std::string dependency, std::string, xe::cmake::core::SourceSpan) {
        link_commands_.push_back(std::make_pair(std::move(target_name), std::move(dependency)));
    }

    void SemanticModel::add_alias(std::string alias_name, std::string target_name) {
        aliases_.push_back(std::make_pair(std::move(alias_name), std::move(target_name)));
    }

    void SemanticModel::add_file(xe::cmake::core::ConcreteSyntaxTree tree) {
        files_.push_back(tree.file_path());
    }

    std::vector<TargetDeclaration> SemanticModel::find_target(std::string_view name) const {
        std::vector<TargetDeclaration> result;
        for (const TargetDeclaration &target : targets_) {
            if (target.name == name) {
                result.push_back(target);
            }
        }
        return result;
    }

    DirectedDependencyGraph SemanticModel::build_graph() const {
        DirectedDependencyGraph graph;
        for (const TargetDeclaration &target : targets_) {
            graph.add_node(target.name);
        }
        for (const auto &pair : aliases_) {
            graph.add_node(pair.first);
        }
        for (const auto &link : link_commands_) {
            graph.add_edge(link.first, link.second);
        }
        return graph;
    }

} // namespace xe::cmake::analysis