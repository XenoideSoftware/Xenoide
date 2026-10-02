#ifndef XE_CMAKE_TESTING_GRAPH_SYNTHETIC_GENERATOR_H
#define XE_CMAKE_TESTING_GRAPH_SYNTHETIC_GENERATOR_H

#include "xe/cmake/analysis/DirectedDependencyGraph.h"

#include <random>
#include <string>
#include <vector>

namespace xe::cmake::testing {

    // Parametric builder for synthetic dependency graphs.
    class GraphBuilder {
    public:
        GraphBuilder &withNode(std::string node) {
            graph_.add_node(std::move(node));
            return *this;
        }

        GraphBuilder &withEdge(std::string source, std::string target) {
            graph_.add_edge(std::move(source), std::move(target));
            return *this;
        }

        xe::cmake::analysis::DirectedDependencyGraph build() const {
            return graph_;
        }

    private:
        xe::cmake::analysis::DirectedDependencyGraph graph_;
    };

    // Generates a random DAG with the given node count, seeded for reproducibility.
    xe::cmake::analysis::DirectedDependencyGraph generate_random_dag(std::size_t node_count, std::uint32_t seed);

} // namespace xe::cmake::testing

#endif // XE_CMAKE_TESTING_GRAPH_SYNTHETIC_GENERATOR_H