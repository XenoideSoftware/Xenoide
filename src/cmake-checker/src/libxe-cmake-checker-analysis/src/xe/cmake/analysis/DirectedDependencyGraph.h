#ifndef XE_CMAKE_ANALYSIS_DIRECTED_DEPENDENCY_GRAPH_H
#define XE_CMAKE_ANALYSIS_DIRECTED_DEPENDENCY_GRAPH_H

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace xe::cmake::analysis {

    // A generic directed edge with metadata attributes.
    struct GraphEdge {
        std::string source;
        std::string target;
        std::string kind;

        std::string_view attribute(std::string_view key) const {
            if (key == "kind") {
                return kind;
            }
            return std::string_view();
        }
    };

    // Generic directed dependency graph over named nodes. Tracks both outgoing
    // edges (dependencies) and incoming edges (consumers).
    class DirectedDependencyGraph {
    public:
        // Registers a node if not already present.
        void add_node(std::string node_id);

        // Adds a directed edge source -> target (deduplicated).
        void add_edge(std::string source, std::string target, std::string kind = "target_link");

        bool has_target(std::string_view node_id) const;
        bool has_edge(std::string_view source, std::string_view target) const;

        std::vector<std::string> node_ids() const;
        std::vector<GraphEdge> outgoing_edges(std::string_view node_id) const;
        std::vector<GraphEdge> incoming_edges(std::string_view node_id) const;
        std::vector<GraphEdge> edges() const;

        // Returns cycles as sequences of node ids (each of length >= 2). A
        // self-loop is reported as a single-element cycle.
        std::vector<std::vector<std::string>> find_cycles() const;

        std::size_t node_count() const {
            return nodes_.size();
        }

        std::size_t edge_count() const {
            return edges_.size();
        }

    private:
        std::vector<std::string> nodes_;
        std::vector<GraphEdge> edges_;
    };

} // namespace xe::cmake::analysis

#endif // XE_CMAKE_ANALYSIS_DIRECTED_DEPENDENCY_GRAPH_H