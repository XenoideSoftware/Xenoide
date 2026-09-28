#include "DirectedDependencyGraph.h"

#include <algorithm>
#include <functional>

namespace xe::cmake::analysis {
    namespace {

        bool contains_edge(const std::vector<GraphEdge> &edges, std::string_view source, std::string_view target) {
            for (const GraphEdge &edge : edges) {
                if (edge.source == source && edge.target == target) {
                    return true;
                }
            }
            return false;
        }

    } // namespace

    void DirectedDependencyGraph::add_node(std::string node_id) {
        if (has_target(node_id)) {
            return;
        }
        nodes_.push_back(std::move(node_id));
    }

    void DirectedDependencyGraph::add_edge(std::string source, std::string target, std::string kind) {
        add_node(source);
        add_node(target);
        if (contains_edge(edges_, source, target)) {
            return;
        }
        GraphEdge edge;
        edge.source = std::move(source);
        edge.target = std::move(target);
        edge.kind = std::move(kind);
        edges_.push_back(std::move(edge));
    }

    bool DirectedDependencyGraph::has_target(std::string_view node_id) const {
        return std::find(nodes_.begin(), nodes_.end(), node_id) != nodes_.end();
    }

    bool DirectedDependencyGraph::has_edge(std::string_view source, std::string_view target) const {
        return contains_edge(edges_, source, target);
    }

    std::vector<std::string> DirectedDependencyGraph::node_ids() const {
        return nodes_;
    }

    std::vector<GraphEdge> DirectedDependencyGraph::outgoing_edges(std::string_view node_id) const {
        std::vector<GraphEdge> result;
        for (const GraphEdge &edge : edges_) {
            if (edge.source == node_id) {
                result.push_back(edge);
            }
        }
        return result;
    }

    std::vector<GraphEdge> DirectedDependencyGraph::incoming_edges(std::string_view node_id) const {
        std::vector<GraphEdge> result;
        for (const GraphEdge &edge : edges_) {
            if (edge.target == node_id) {
                result.push_back(edge);
            }
        }
        return result;
    }

    std::vector<GraphEdge> DirectedDependencyGraph::edges() const {
        return edges_;
    }

    std::vector<std::vector<std::string>> DirectedDependencyGraph::find_cycles() const {
        std::vector<std::vector<std::string>> cycles;
        const std::size_t node_count = nodes_.size();

        for (std::size_t start = 0; start < node_count; ++start) {
            std::vector<std::string> path;
            std::vector<std::size_t> visited;
            visited.push_back(start);

            std::function<void(std::size_t)> dfs = [&](std::size_t current) {
                path.push_back(nodes_[current]);
                for (const GraphEdge &edge : outgoing_edges(nodes_[current])) {
                    const std::size_t next_index = static_cast<std::size_t>(std::find(nodes_.begin(), nodes_.end(), edge.target) - nodes_.begin());
                    if (next_index >= node_count) {
                        continue;
                    }
                    if (next_index == start) {
                        cycles.push_back(path);
                    } else if (next_index > start && std::find(visited.begin(), visited.end(), next_index) == visited.end()) {
                        visited.push_back(next_index);
                        dfs(next_index);
                        visited.pop_back();
                    }
                }
                path.pop_back();
            };

            dfs(start);
        }

        return cycles;
    }

} // namespace xe::cmake::analysis