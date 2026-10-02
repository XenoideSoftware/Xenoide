#include "xe/cmake/script/ScriptRuleLoader.h"
#include "xe/cmake/testing/CstSyntheticGenerator.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake::script {

    TEST_CASE("ScriptRuleLoader runs check_file hooks") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_file(ctx, file) {
    if (file.path() == "/v/bad/CMakeLists.txt") {
        ctx.report(Finding("custom.bad-path", Severity.Warn, "bad path", file.commands()[0].span()));
    }
}
)");
        const auto good = xe::cmake::testing::parse_cst("/v/good/CMakeLists.txt", "set (a 1)");
        const auto bad = xe::cmake::testing::parse_cst("/v/bad/CMakeLists.txt", "set (a 1)");
        const std::vector<xe::cmake::core::ConcreteSyntaxTree> files = {good, bad};
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_file(files);
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].rule_id == "custom.bad-path");
        REQUIRE(findings[0].file_path == "/v/bad/CMakeLists.txt");
    }

    TEST_CASE("ScriptRuleLoader runs check_command hooks") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_command(ctx, cmd) {
    if (cmd.name() == "add_executable") {
        ctx.report(Finding("custom.executable", Severity.Error, "no exe", cmd.span()));
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_executable(app main.cpp)");
        const std::vector<xe::cmake::core::ConcreteSyntaxTree> files = {cst};
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_command(files);
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].rule_id == "custom.executable");
        REQUIRE(findings[0].severity == xe::cmake::core::Severity::Error);
    }

    TEST_CASE("ScriptRuleLoader runs check_project hooks with the graph") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_project(ctx, project, graph) {
    if (graph.has_target("app") && graph.has_target("libxe-a")) {
        ctx.report(Finding("custom.project", Severity.Info, "project ok", SourceSpan()));
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)");
        xe::cmake::analysis::DirectedDependencyGraph graph;
        graph.add_edge("app", "libxe-a");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_project({cst}, graph);
        REQUIRE(findings.size() == 1);
    }

    TEST_CASE("ScriptRuleLoader handles scripts without hooks") {
        ScriptRuleLoader loader;
        loader.load("var unrelated = 1;");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)");
        REQUIRE(loader.run_check_file({cst}).empty());
        REQUIRE(loader.run_check_command({cst}).empty());
        REQUIRE(loader.run_check_project({cst}, {}).empty());
    }

    TEST_CASE("ScriptRuleLoader supports reporting with attached fixes") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_command(ctx, cmd) {
    if (cmd.name() == "set") {
        var fix = Fix("quote");
        fix.add_edit(TextEdit.replace(cmd.argument(1).span(), "\"quoted\""));
        ctx.report(Finding("custom.fix", Severity.Warn, "quote it", cmd.argument(1).span(), fix));
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (target libxe-a)");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_command({cst});
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].has_fix());
        REQUIRE(findings[0].fix->edits.size() == 1);
        REQUIRE(findings[0].fix->edits[0].new_text == "\"quoted\"");
    }

    TEST_CASE("ScriptBindings proxy methods cover quote kinds and indexes") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_command(ctx, cmd) {
    if (cmd.name() == "set") {
        var a0 = cmd.argument(0);
        var a1 = cmd.argument(1);
        if (a0.text() == "target" && a1.quote_kind() == "quoted") {
            ctx.report(Finding("proxy.quoted", Severity.Info, "ok", a1.span()));
        }
        if (a0.index() == 0) {
            ctx.report(Finding("proxy.index", Severity.Info, "idx", a0.span()));
        }
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (target \"libxe-a\")");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_command({cst});
        REQUIRE(findings.size() == 2);
    }

    TEST_CASE("ScriptBindings finding accessors") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_command(ctx, cmd) {
    var finding = Finding("custom.accessor", Severity.Warn, "msg", cmd.span());
    if (finding.rule_id() == "custom.accessor" && finding.has_fix() == false) {
        ctx.report(Finding("proxy.ruleid", Severity.Warn, finding.message(), cmd.span()));
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(lib)");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_command({cst});
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].rule_id == "proxy.ruleid");
        REQUIRE(findings[0].message == "msg");
    }

    TEST_CASE("ScriptBindings TextEdit factories produce all kinds") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_command(ctx, cmd) {
    var fix = Fix("multi");
    fix.add_edit(TextEdit.insert_before(0, "a"));
    fix.add_edit(TextEdit.insert_after(1, "b"));
    fix.add_edit(TextEdit.remove(cmd.span()));
    ctx.report(Finding("proxy.edits", Severity.Warn, "edits", cmd.span(), fix));
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (x 1)");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_command({cst});
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].has_fix());
        REQUIRE(findings[0].fix->edits.size() == 3);
    }

    TEST_CASE("ScriptBindings graph proxy methods") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_project(ctx, project, graph) {
    var edges = graph.outgoing_edges("app");
    for (var i = 0; i < edges.size(); ++i) {
        var e = edges[i];
        if (e.source() == "app" && e.target() == "libxe-a" && e.attribute("kind") == "target_link") {
            ctx.report(Finding("proxy.graph", Severity.Info, "edge", SourceSpan()));
        }
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)");
        xe::cmake::analysis::DirectedDependencyGraph graph;
        graph.add_edge("app", "libxe-a", "target_link");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_project({cst}, graph);
        REQUIRE(findings.size() == 1);
    }

    TEST_CASE("ScriptBindings span accessors") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_command(ctx, cmd) {
    var span = cmd.span();
    if (span.start_line() >= 1 && span.end_offset() >= span.start_offset()) {
        ctx.report(Finding("proxy.span", Severity.Info, "span", span));
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (x 1)");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_command({cst});
        REQUIRE(findings.size() == 1);
    }

    TEST_CASE("ScriptRuleLoader uses check_file and check_project hooks together") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_file(ctx, file) {
    ctx.report(Finding("proxy.file", Severity.Warn, "file hook", SourceSpan()));
}
def check_project(ctx, project, graph) {
    ctx.report(Finding("proxy.project", Severity.Warn, "project hook", SourceSpan()));
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)");
        std::vector<xe::cmake::core::Finding> findings = loader.run_check_file({cst});
        REQUIRE(findings.size() == 1);
        findings = loader.run_check_project({cst}, {});
        REQUIRE(findings.size() == 1);
    }

    TEST_CASE("ScriptBindings find_commands filters by name") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_file(ctx, file) {
    var libs = file.find_commands("add_library");
    if (libs.size() == 2) {
        ctx.report(Finding("proxy.find_commands", Severity.Info, "found", libs[0].span()));
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(a)\nadd_executable(b)\nadd_library(c)\n");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_file({cst});
        REQUIRE(findings.size() == 1);
    }

    TEST_CASE("ScriptBindings missing graph methods are safe") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_project(ctx, project, graph) {
    var ids = graph.node_ids();
    ctx.report(Finding("proxy.emptygraph", Severity.Info, "n=" + to_string(ids.size()), SourceSpan()));
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_project({cst}, {});
        REQUIRE(findings.size() == 1);
    }

    TEST_CASE("ScriptBindings quote kinds via script") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_command(ctx, cmd) {
    if (cmd.name() == "set") {
        var raw = cmd.argument(1);
        var quoted = cmd.argument(2);
        if (raw.quote_kind() == "raw" && quoted.quote_kind() == "quoted") {
            ctx.report(Finding("proxy.quotekinds", Severity.Info, "kinds", raw.span()));
        }
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (target libxe-a \"quoted\")");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_command({cst});
        REQUIRE(findings.size() == 1);
    }

    TEST_CASE("ScriptBindings bracket quote kind via script") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_command(ctx, cmd) {
    if (cmd.name() == "set") {
        var b = cmd.argument(1);
        if (b.quote_kind() == "bracket") {
            ctx.report(Finding("proxy.bracket", Severity.Info, "bracket", b.span()));
        }
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (banner [=[hello]=])");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_command({cst});
        REQUIRE(findings.size() == 1);
    }

    TEST_CASE("ScriptBindings out-of-range argument returns empty proxy") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_command(ctx, cmd) {
    var missing = cmd.argument(99);
    if (missing.text() == "" && missing.index() == 0) {
        ctx.report(Finding("proxy.missingarg", Severity.Info, "empty", cmd.span()));
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_command({cst});
        REQUIRE(findings.size() == 1);
    }

    TEST_CASE("ScriptBindings file path and command iteration") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_file(ctx, file) {
    var cmds = file.commands();
    var names = "";
    for (var i = 0; i < cmds.size(); ++i) {
        names = names + cmds[i].name() + ",";
    }
    if (names == "set,add_library,") {
        ctx.report(Finding("proxy.names", Severity.Info, file.path(), SourceSpan()));
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)\nadd_library(x)\n");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_file({cst});
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].message == "/v/CMakeLists.txt");
    }

    TEST_CASE("ScriptBindings finding with fix accessor round-trips") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_command(ctx, cmd) {
    var fix = Fix("desc");
    fix.add_edit(TextEdit.replace(cmd.span(), "replacement"));
    var finding = Finding("proxy.fixacc", Severity.Warn, "m", cmd.span(), fix);
    if (finding.has_fix() && finding.fix().description() == "desc") {
        var edits = finding.fix().edits();
        ctx.report(Finding("proxy.fixacc2", Severity.Warn, "n=" + to_string(edits.size()), cmd.span()));
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_command({cst});
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].rule_id == "proxy.fixacc2");
        REQUIRE(findings[0].message == "n=1");
    }

    TEST_CASE("ScriptBindings ChaiScript ranged-for over commands") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_file(ctx, file) {
    var count = 0;
    for (cmd : file.commands()) {
        count = count + 1;
    }
    if (count == 3) {
        ctx.report(Finding("proxy.rangedfor", Severity.Info, "count=" + to_string(count), SourceSpan()));
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)\nadd_library(x)\nadd_executable(y)\n");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_file({cst});
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].message == "count=3");
    }

    TEST_CASE("ScriptBindings ChaiScript ranged-for over string list") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_project(ctx, project, graph) {
    var ids = graph.node_ids();
    var joined = "";
    for (id : ids) {
        joined = joined + id + ",";
    }
    if (joined == "app,libxe-a,") {
        ctx.report(Finding("proxy.rangedforids", Severity.Info, joined, SourceSpan()));
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)");
        xe::cmake::analysis::DirectedDependencyGraph graph;
        graph.add_edge("app", "libxe-a");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_project({cst}, graph);
        REQUIRE(findings.size() == 1);
    }

    TEST_CASE("ScriptBindings string primitives via script") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_file(ctx, file) {
    var p = file.path();
    if (regex_match("libxe-core", "^libxe-.*") &&
        regex_search(p, "CMakeLists") &&
        str_contains(p, "libxe") &&
        str_starts_with(p, "/v/") &&
        str_ends_with(p, "CMakeLists.txt")) {
        var parts = str_split(p, "/");
        if (parts.size() == 4) {
            ctx.report(Finding("proxy.strprims", Severity.Info, "ok", SourceSpan()));
        }
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/libxe-core/CMakeLists.txt", "set (a 1)");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_file({cst});
        REQUIRE(findings.size() == 1);
    }

    TEST_CASE("ScriptBindings string primitives negative cases") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_file(ctx, file) {
    var p = file.path();
    if (!regex_match(p, "^libxe-.*") &&
        !regex_search(p, "notpresent") &&
        !str_contains(p, "zzz") &&
        !str_starts_with(p, "engine/") &&
        !str_ends_with(p, ".cpp")) {
        ctx.report(Finding("proxy.strprimneg", Severity.Info, "ok", SourceSpan()));
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/libxe-core/CMakeLists.txt", "set (a 1)");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_file({cst});
        REQUIRE(findings.size() == 1);
    }

    TEST_CASE("ScriptBindings graph cycle detection via script") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_project(ctx, project, graph) {
    if (graph.has_target("a") && graph.has_target("b") && graph.has_edge("a", "b")) {
        ctx.report(Finding("proxy.graphedge", Severity.Info, "edge", SourceSpan()));
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)");
        xe::cmake::analysis::DirectedDependencyGraph graph;
        graph.add_edge("a", "b");
        graph.add_edge("b", "c");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_project({cst}, graph);
        REQUIRE(findings.size() == 1);
    }

    TEST_CASE("ScriptBindings split and join string segments") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_project(ctx, project, graph) {
    var ids = graph.node_ids();
    if (ids.size() == 3) {
        var joined = "";
        for (id : ids) {
            joined = joined + id + ";";
        }
        if (joined == "a;b;c;") {
            ctx.report(Finding("proxy.join", Severity.Info, joined, SourceSpan()));
        }
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)");
        xe::cmake::analysis::DirectedDependencyGraph graph;
        graph.add_edge("a", "b");
        graph.add_edge("b", "c");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_project({cst}, graph);
        REQUIRE(findings.size() == 1);
    }

    TEST_CASE("ScriptBindings to_string conversions") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_command(ctx, cmd) {
    var n = cmd.argument_count();
    var s = to_string(n);
    if (s == "2") {
        ctx.report(Finding("proxy.tostring", Severity.Info, s, cmd.span()));
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a b)");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_command({cst});
        REQUIRE(findings.size() == 1);
    }

    TEST_CASE("ScriptBindings kitchen-sink CMake parse exercises the Lexer") {
        const auto cst = xe::cmake::testing::parse_cst(
            "/v/CMakeLists.txt",
            "  \t# leading comment\n"
            "cmake_minimum_required (VERSION 3.25)\r\n"
            "project (Kitchen)\n"
            "set (target \"libxe-alpha\")\n"
            "set (sources src/A.cpp src/B.cpp)\n"
            "add_library(${target} ${sources})\n"
            "#[=[ block\ncomment ]=]\n"
            "set (banner [=[hello\nworld]=])\n"
            "set (escaped a\\ value \"quo\\\"ted\")\n"
            "target_link_libraries(${target} PRIVATE libxe-beta)\n"
        );
        REQUIRE_FALSE(cst.has_errors());
        REQUIRE(cst.commands().size() == 8);

        ScriptRuleLoader loader;
        loader.load(R"(
def check_file(ctx, file) {
    var total_args = 0;
    var cmds = file.commands();
    for (var i = 0; i < cmds.size(); ++i) {
        total_args = total_args + cmds[i].argument_count();
    }
    if (total_args == 18) {
        ctx.report(Finding("proxy.kitchen", Severity.Info, "count=" + to_string(total_args), SourceSpan()));
    }
}
)");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_file({cst});
        REQUIRE(findings.size() == 1);
    }

    TEST_CASE("ScriptBindings escaped and quoted argument round-trips") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (escaped a\\ value \"quo\\\"ted\")\n");
        REQUIRE_FALSE(cst.has_errors());
        REQUIRE(cst.commands()[0]->argument_count() == 3);
        REQUIRE(cst.commands()[0]->argument(1).text == "a value");
        REQUIRE(cst.commands()[0]->argument(2).text == "quo\"ted");
        REQUIRE(cst.commands()[0]->argument(2).is_quoted());
        REQUIRE(cst.commands()[0]->argument(1).is_raw());
    }

    TEST_CASE("ScriptBindings bracket argument round-trips") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (banner [=[hello\nworld]=])\n");
        REQUIRE_FALSE(cst.has_errors());
        REQUIRE(cst.commands()[0]->argument(1).is_bracket());
        REQUIRE(cst.commands()[0]->argument(1).text == "hello\nworld");
    }

    TEST_CASE("ScriptBindings str_split all segments") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_project(ctx, project, graph) {
    var parts = str_split("a;b;c;d", ";");
    if (parts.size() == 4 && parts[1] == "b" && parts[2] == "c" && parts[3] == "d" && parts[0] == "a") {
        ctx.report(Finding("proxy.splitall", Severity.Info, "ok", SourceSpan()));
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_project({cst}, {});
        REQUIRE(findings.size() == 1);
    }

    TEST_CASE("ScriptBindings str_split repeated delimiter") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_project(ctx, project, graph) {
    var parts = str_split("x;;y", ";");
    if (parts.size() == 3 && parts[0] == "x" && parts[1] == "" && parts[2] == "y") {
        ctx.report(Finding("proxy.splitrepeat", Severity.Info, "ok", SourceSpan()));
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_project({cst}, {});
        REQUIRE(findings.size() == 1);
    }

    TEST_CASE("ScriptBindings str_split trailing delimiter") {
        ScriptRuleLoader loader;
        loader.load(R"(
def check_project(ctx, project, graph) {
    var parts = str_split("a;b;", ";");
    if (parts.size() == 3 && parts[0] == "a" && parts[1] == "b" && parts[2] == "") {
        ctx.report(Finding("proxy.splittrail", Severity.Info, "ok", SourceSpan()));
    }
}
)");
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_project({cst}, {});
        REQUIRE(findings.size() == 1);
    }

    TEST_CASE("ScriptBindings argument collection loops") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (target \"lib\" src/A.cpp src/B.cpp)\n");
        REQUIRE_FALSE(cst.has_errors());

        ScriptRuleLoader loader;
        loader.load(R"(
def check_command(ctx, cmd) {
    var args = cmd.arguments();
    var names = "";
    for (var i = 0; i < args.size(); ++i) {
        names = names + args[i].text() + ";";
    }
    if (names == "target;lib;src/A.cpp;src/B.cpp;") {
        ctx.report(Finding("proxy.argloop", Severity.Info, names, cmd.span()));
    }
}
)");
        const std::vector<xe::cmake::core::Finding> findings = loader.run_check_command({cst});
        REQUIRE(findings.size() == 1);
    }

} // namespace xe::cmake::script