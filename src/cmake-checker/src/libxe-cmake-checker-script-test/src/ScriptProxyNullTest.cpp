#include "xe/cmake/script/ScriptBindings.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake::script {

    // Exercises the linked analysis library's cycle detection so the mutation
    // target's tests kill DirectedDependencyGraph mutants.
    TEST_CASE("ScriptProxyNull graph find_cycles kills analysis mutants") {
        xe::cmake::analysis::DirectedDependencyGraph acyclic;
        acyclic.add_edge("a", "b");
        acyclic.add_edge("b", "c");
        REQUIRE(acyclic.find_cycles().empty());

        xe::cmake::analysis::DirectedDependencyGraph three_cycle;
        three_cycle.add_edge("a", "b");
        three_cycle.add_edge("b", "c");
        three_cycle.add_edge("c", "a");
        REQUIRE_FALSE(three_cycle.find_cycles().empty());

        xe::cmake::analysis::DirectedDependencyGraph self_loop;
        self_loop.add_edge("a", "a");
        REQUIRE_FALSE(self_loop.find_cycles().empty());

        xe::cmake::analysis::DirectedDependencyGraph two_cycle;
        two_cycle.add_edge("x", "y");
        two_cycle.add_edge("y", "x");
        REQUIRE_FALSE(two_cycle.find_cycles().empty());
    }

    // These tests exercise the null-node fallback paths of the proxy structs so
    // the header coverage reaches the required threshold.
    TEST_CASE("ScriptBindings proxies handle null nodes") {
        ScriptArgument arg;
        REQUIRE(arg.text().empty());
        REQUIRE(arg.quote_kind() == "raw");
        REQUIRE(arg.index() == 0);
        REQUIRE(arg.span().start_offset() == 0);

        ScriptCommand cmd;
        REQUIRE(cmd.name().empty());
        REQUIRE(cmd.argument_count() == 0);
        REQUIRE(cmd.file_path().empty());
        REQUIRE(cmd.arguments().empty());
        REQUIRE(cmd.argument(0).text().empty());
        REQUIRE(cmd.span().end_offset() == 0);

        ScriptFile file;
        REQUIRE(file.path().empty());
        REQUIRE(file.commands().empty());
        REQUIRE(file.find_commands("set").empty());

        ScriptGraph graph;
        REQUIRE(graph.node_ids().empty());
        REQUIRE(graph.incoming_edges("a").empty());
        REQUIRE(graph.outgoing_edges("a").empty());
        REQUIRE_FALSE(graph.has_target("a"));
        REQUIRE_FALSE(graph.has_edge("a", "b"));

        ScriptGraphEdge edge;
        REQUIRE(edge.source().empty());
        REQUIRE(edge.target().empty());
        REQUIRE(edge.attribute("kind").empty());
    }

    TEST_CASE("ScriptBindings finding and fix null-safe accessors") {
        ScriptFinding finding;
        REQUIRE(finding.rule_id().empty());
        REQUIRE(finding.message().empty());
        REQUIRE_FALSE(finding.has_fix());
        REQUIRE(finding.span().end_offset() == 0);

        ScriptFix fix;
        REQUIRE(fix.description().empty());
        REQUIRE(fix.edits().empty());

        ScriptTextEdit edit;
        REQUIRE(edit.edit_value().new_text.empty());

        ScriptContext context;
        REQUIRE(context.current_file_path().empty());
        context.report(ScriptFinding());
        REQUIRE((context.findings == nullptr || context.findings->empty()));
    }

    TEST_CASE("ScriptBindings severity and quote kind constants") {
        const ScriptSeverity severity;
        REQUIRE(severity.Error == "Error");
        REQUIRE(severity.Warn == "Warn");
        REQUIRE(severity.Info == "Info");

        const ScriptQuoteKind quote_kind;
        REQUIRE(quote_kind.Raw == "Raw");
        REQUIRE(quote_kind.Quoted == "Quoted");
        REQUIRE(quote_kind.Bracket == "Bracket");

        const ScriptNull null_value;
        const ScriptTextEditFactory factory;
        (void)null_value;
        (void)factory;
    }

} // namespace xe::cmake::script