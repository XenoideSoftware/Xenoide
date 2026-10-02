#include "xe/cmake/analysis/SemanticModel.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake::analysis {

    TEST_CASE("SemanticModel stores targets and finds by name") {
        SemanticModel model;
        TargetDeclaration lib;
        lib.name = "libxe-a";
        lib.kind = "library";
        lib.file_path = "/v/libxe-a/CMakeLists.txt";
        model.add_target(lib);
        TargetDeclaration app;
        app.name = "app";
        app.kind = "executable";
        app.file_path = "/v/app/CMakeLists.txt";
        model.add_target(app);
        REQUIRE(model.target_count() == 2);
        REQUIRE(model.find_target("libxe-a").size() == 1);
        REQUIRE(model.find_target("missing").empty());
    }

    TEST_CASE("SemanticModel build_graph reflects link commands") {
        SemanticModel model;
        TargetDeclaration app;
        app.name = "app";
        model.add_target(app);
        TargetDeclaration lib;
        lib.name = "libxe-a";
        model.add_target(lib);
        model.add_link_command("app", "libxe-a", "PRIVATE", {});
        const DirectedDependencyGraph graph = model.build_graph();
        REQUIRE(graph.has_target("app"));
        REQUIRE(graph.has_edge("app", "libxe-a"));
    }

    TEST_CASE("SemanticModel aliases add nodes to the graph") {
        SemanticModel model;
        TargetDeclaration lib;
        lib.name = "libxe-a";
        model.add_target(lib);
        model.add_alias("xe::a", "libxe-a");
        const DirectedDependencyGraph graph = model.build_graph();
        REQUIRE(graph.has_target("xe::a"));
    }

    TEST_CASE("SemanticModel records files") {
        SemanticModel model;
        model.add_file(xe::cmake::core::ConcreteSyntaxTree("/v/a/CMakeLists.txt", ""));
        model.add_file(xe::cmake::core::ConcreteSyntaxTree("/v/b/CMakeLists.txt", ""));
        REQUIRE(model.files().size() == 2);
    }

    TEST_CASE("SemanticModel stores source/include/link facts without throwing") {
        SemanticModel model;
        TargetDeclaration lib;
        lib.name = "libxe-a";
        lib.kind = "library";
        lib.file_path = "/v/libxe-a/CMakeLists.txt";
        model.add_target(lib);
        model.add_source("libxe-a", "src/A.cpp", {});
        model.add_include_directory("libxe-a", "src", {});
        model.add_link_command("libxe-a", "libxe-b", "PRIVATE", {});
        REQUIRE(model.target_count() == 1);
        REQUIRE(model.find_target("libxe-a").size() == 1);
        REQUIRE(model.find_target("libxe-b").empty());
    }

    TEST_CASE("SemanticModel graph includes inter-library links") {
        SemanticModel model;
        TargetDeclaration a;
        a.name = "libxe-a";
        model.add_target(a);
        TargetDeclaration b;
        b.name = "libxe-b";
        model.add_target(b);
        TargetDeclaration app;
        app.name = "app";
        model.add_target(app);
        model.add_link_command("app", "libxe-a", "PRIVATE", {});
        model.add_link_command("libxe-a", "libxe-b", "PRIVATE", {});
        const DirectedDependencyGraph graph = model.build_graph();
        REQUIRE(graph.has_edge("app", "libxe-a"));
        REQUIRE(graph.has_edge("libxe-a", "libxe-b"));
        REQUIRE(graph.node_count() == 3);
    }

    TEST_CASE("SemanticModel accessors expose targets and files") {
        SemanticModel model;
        TargetDeclaration a;
        a.name = "libxe-a";
        a.kind = "library";
        a.file_path = "/v/libxe-a/CMakeLists.txt";
        model.add_target(a);
        model.add_file(xe::cmake::core::ConcreteSyntaxTree("/v/libxe-a/CMakeLists.txt", ""));

        REQUIRE(model.target_count() == 1);
        REQUIRE(model.targets().size() == 1);
        REQUIRE(model.targets()[0].name == "libxe-a");
        REQUIRE(model.targets()[0].kind == "library");
        REQUIRE(model.targets()[0].file_path == "/v/libxe-a/CMakeLists.txt");
        REQUIRE(model.files().size() == 1);
        REQUIRE(model.files()[0] == "/v/libxe-a/CMakeLists.txt");
    }

} // namespace xe::cmake::analysis