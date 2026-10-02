#include "xe/cmake/analysis/DirectedDependencyGraph.h"
#include "xe/cmake/testing/GraphSyntheticGenerator.h"
#include "xe/cmake/testing/PropertyAssertions.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake::analysis {

    TEST_CASE("DirectedDependencyGraph registers nodes and deduplicates edges") {
        DirectedDependencyGraph graph;
        graph.add_edge("a", "b");
        graph.add_edge("a", "b");
        graph.add_edge("b", "c");
        REQUIRE(graph.node_count() == 3);
        REQUIRE(graph.edge_count() == 2);
        REQUIRE(graph.has_target("a"));
        REQUIRE_FALSE(graph.has_target("z"));
    }

    TEST_CASE("DirectedDependencyGraph tracks incoming and outgoing edges") {
        DirectedDependencyGraph graph;
        graph.add_edge("app", "libxe-a");
        graph.add_edge("app", "libxe-b");
        graph.add_edge("libxe-b", "libxe-a");

        const std::vector<GraphEdge> outgoing = graph.outgoing_edges("app");
        REQUIRE(outgoing.size() == 2);
        const std::vector<GraphEdge> incoming = graph.incoming_edges("libxe-a");
        REQUIRE(incoming.size() == 2);
        REQUIRE(incoming[0].source == "app");
        REQUIRE(incoming[1].source == "libxe-b");
    }

    TEST_CASE("DirectedDependencyGraph edge attributes") {
        DirectedDependencyGraph graph;
        graph.add_edge("app", "libxe-a", "target_link");
        const std::vector<GraphEdge> edges = graph.outgoing_edges("app");
        REQUIRE(edges.size() == 1);
        REQUIRE(edges[0].attribute("kind") == "target_link");
        REQUIRE(edges[0].attribute("unknown") == "");
    }

    TEST_CASE("DirectedDependencyGraph detects a cycle") {
        DirectedDependencyGraph graph;
        graph.add_edge("a", "b");
        graph.add_edge("b", "c");
        graph.add_edge("c", "a");
        REQUIRE_FALSE(testing::requireGraphAcyclic(graph));
        REQUIRE_FALSE(graph.find_cycles().empty());
    }

    TEST_CASE("DirectedDependencyGraph is acyclic for a valid DAG") {
        DirectedDependencyGraph graph;
        graph.add_edge("a", "b");
        graph.add_edge("b", "c");
        graph.add_edge("a", "c");
        REQUIRE(testing::requireGraphAcyclic(graph));
    }

    TEST_CASE("DirectedDependencyGraph random DAG generator is acyclic") {
        for (std::uint32_t seed = 1; seed <= 10; ++seed) {
            const DirectedDependencyGraph graph = testing::generate_random_dag(20, seed);
            REQUIRE(testing::requireGraphAcyclic(graph));
        }
    }

    TEST_CASE("DirectedDependencyGraph self-loop is a cycle") {
        DirectedDependencyGraph graph;
        graph.add_edge("a", "a");
        REQUIRE_FALSE(graph.find_cycles().empty());
    }

} // namespace xe::cmake::analysis