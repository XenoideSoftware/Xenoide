#include "GraphSyntheticGenerator.h"

namespace xe::cmake::testing {

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    xe::cmake::analysis::DirectedDependencyGraph generate_random_dag(std::size_t node_count, std::uint32_t seed) {
        xe::cmake::analysis::DirectedDependencyGraph graph;
        std::mt19937 rng(seed);

        for (std::size_t i = 0; i < node_count; ++i) {
            graph.add_node("target_" + std::to_string(i));
        }
        // Connect each node i to some nodes with a larger index, producing a DAG.
        std::uniform_int_distribution<int> count_dist(0, 3);
        for (std::size_t i = 0; i + 1 < node_count; ++i) {
            const int edge_count = count_dist(rng);
            std::uniform_int_distribution<std::size_t> target_dist(i + 1, node_count - 1);
            for (int e = 0; e < edge_count; ++e) {
                const std::size_t target = target_dist(rng);
                graph.add_edge("target_" + std::to_string(i), "target_" + std::to_string(target));
            }
        }
        return graph;
    }

} // namespace xe::cmake::testing