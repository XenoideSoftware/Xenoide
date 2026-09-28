#include "xe/cmake/io/BuildTreeReader.h"
#include "xe/cmake/io/FileSystem.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake::io {

    TEST_CASE("BuildTreeReader parses a trace file into graph edges") {
        InMemoryFileSystem fs;
        fs.write_file("/v/trace.json", R"([{"args":["target_link_libraries","app","PRIVATE","libxe-a"],"file":"/v/CMakeLists.txt"},{"args":["set","x","1"]}])");
        xe::cmake::analysis::DirectedDependencyGraph graph;
        const BuildTreeReader reader(fs);
        reader.read_trace("/v/trace.json", graph);
        REQUIRE(graph.has_target("app"));
        REQUIRE(graph.has_target("libxe-a"));
        REQUIRE(graph.has_edge("app", "libxe-a"));
    }

    TEST_CASE("BuildTreeReader ignores malformed trace entries") {
        InMemoryFileSystem fs;
        fs.write_file("/v/trace.json", "[]");
        xe::cmake::analysis::DirectedDependencyGraph graph;
        const BuildTreeReader reader(fs);
        reader.read_trace("/v/trace.json", graph);
        REQUIRE(graph.node_count() == 0);
    }

    TEST_CASE("BuildTreeReader returns cleanly for a missing trace") {
        InMemoryFileSystem fs;
        xe::cmake::analysis::DirectedDependencyGraph graph;
        const BuildTreeReader reader(fs);
        reader.read_trace("/missing/trace.json", graph);
        REQUIRE(graph.node_count() == 0);
    }
    TEST_CASE("BuildTreeReader parses a codemodel reply") {
        InMemoryFileSystem fs;
        fs.write_file("/v/.cmake/api/v1/reply/codemodel-v2.json", R"({"configurations":[{"targets":[{"name":"app","dependencies":[{"name":"libxe-a"}]}]}]})");
        xe::cmake::analysis::DirectedDependencyGraph graph;
        const BuildTreeReader reader(fs);
        reader.read_codemodel("/v/.cmake/api/v1/reply", graph);
        REQUIRE(graph.has_target("app"));
        REQUIRE(graph.has_edge("app", "libxe-a"));
    }

    TEST_CASE("BuildTreeReader codemodel skips dependency without id or name") {
        InMemoryFileSystem fs;
        fs.write_file("/v/.cmake/api/v1/reply/codemodel-v2.json", R"({"configurations":[{"targets":[{"name":"app","dependencies":[{"foo":"bar"}]}]}]})");
        xe::cmake::analysis::DirectedDependencyGraph graph;
        const BuildTreeReader reader(fs);
        reader.read_codemodel("/v/.cmake/api/v1/reply", graph);
        REQUIRE(graph.has_target("app"));
        REQUIRE(graph.node_count() == 1);
    }

    TEST_CASE("BuildTreeReader codemodel uses id fallback") {
        InMemoryFileSystem fs;
        fs.write_file("/v/.cmake/api/v1/reply/codemodel-v2.json", R"({"configurations":[{"targets":[{"name":"app","dependencies":[{"id":"libxe-b"}]}]}]})");
        xe::cmake::analysis::DirectedDependencyGraph graph;
        const BuildTreeReader reader(fs);
        reader.read_codemodel("/v/.cmake/api/v1/reply", graph);
        REQUIRE(graph.has_edge("app", "libxe-b"));
    }

    TEST_CASE("BuildTreeReader codemodel ignores malformed json") {
        InMemoryFileSystem fs;
        fs.write_file("/v/.cmake/api/v1/reply/codemodel-v2.json", "{not json");
        xe::cmake::analysis::DirectedDependencyGraph graph;
        const BuildTreeReader reader(fs);
        reader.read_codemodel("/v/.cmake/api/v1/reply", graph);
        REQUIRE(graph.node_count() == 0);
    }

    TEST_CASE("BuildTreeReader codemodel ignores missing configurations") {
        InMemoryFileSystem fs;
        fs.write_file("/v/.cmake/api/v1/reply/codemodel-v2.json", R"({"other": 1})");
        xe::cmake::analysis::DirectedDependencyGraph graph;
        const BuildTreeReader reader(fs);
        reader.read_codemodel("/v/.cmake/api/v1/reply", graph);
        REQUIRE(graph.node_count() == 0);
    }

    TEST_CASE("BuildTreeReader trace ignores non-array or empty") {
        InMemoryFileSystem fs;
        fs.write_file("/v/trace.json", R"({"not":"array"})");
        xe::cmake::analysis::DirectedDependencyGraph graph;
        const BuildTreeReader reader(fs);
        reader.read_trace("/v/trace.json", graph);
        REQUIRE(graph.node_count() == 0);
    }

    TEST_CASE("BuildTreeReader trace treats malformed trace as empty") {
        InMemoryFileSystem fs;
        fs.write_file("/v/trace.json", "not json");
        xe::cmake::analysis::DirectedDependencyGraph graph;
        const BuildTreeReader reader(fs);
        reader.read_trace("/v/trace.json", graph);
        REQUIRE(graph.node_count() == 0);
    }

} // namespace xe::cmake::io