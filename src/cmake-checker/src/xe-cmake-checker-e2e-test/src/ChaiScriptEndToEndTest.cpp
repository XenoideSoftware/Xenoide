#include "SyntheticProjectGenerator.h"

#include "xe/cmake/io/DefaultCmakeParser.h"
#include "xe/cmake/io/ProjectLoader.h"
#include "xe/cmake/io/SyncWriter.h"
#include "xe/cmake/script/ScriptRuleLoader.h"
#include "xe/cmake/testing/PropertyAssertions.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake {

    using namespace xe::cmake;

    namespace {

        struct ScriptFixture {
            io::InMemoryFileSystem fs;
            io::DefaultCmakeParser parser;
            io::ProjectLoader loader;
            std::vector<core::ConcreteSyntaxTree> files;

            ScriptFixture() : parser(fs), loader(fs, parser) {
                testing::CMakeProjectFixtureBuilder().withExecutableCount(1).withLibraryCount(2).withTestCount(0).withDiverseCMakeSyntaxStyles().withSeed(7).build(fs);
                files = loader.load_project("/virtual_project");
            }
        };

        const char *alias_include_script = R"(
def check_file(ctx, file) {
    var has_library = false;
    var has_alias = false;
    var has_include = false;
    var lib_span = SourceSpan();
    var cmds = file.commands();
    for (var i = 0; i < cmds.size(); ++i) {
        var cmd = cmds[i];
        if (cmd.name() == "add_library") {
            if (cmd.argument_count() > 1 && cmd.argument(1).text() == "ALIAS") {
                has_alias = true;
            } else {
                has_library = true;
                lib_span = cmd.span();
            }
        } else if (cmd.name() == "target_include_directories") {
            has_include = true;
        }
    }
    if (has_library) {
        if (!has_alias) {
            ctx.report(Finding("library.alias-specification", Severity.Warn,
                               "Missing ALIAS target", lib_span));
        }
        if (!has_include) {
            ctx.report(Finding("library.include-directories-src", Severity.Warn,
                               "Missing target_include_directories", lib_span));
        }
    }
}
)";

    } // namespace

    // Scenario B: static library missing alias and include dirs.
    TEST_CASE("ChaiScriptEndToEnd: flag missing alias and include directories") {
        io::InMemoryFileSystem fs;
        fs.create_directories("/virtual_project/libxe-math");
        fs.write_file(
            "/virtual_project/libxe-math/CMakeLists.txt",
            "set (target \"libxe-math\")\n"
            "set (sources \"src/Math.cpp\")\n"
            "add_library(${target} ${sources})\n"
        );

        const io::DefaultCmakeParser parser(fs);
        const io::ProjectLoader loader(fs, parser);
        const auto files = loader.load_project("/virtual_project");

        script::ScriptRuleLoader script_loader;
        script_loader.load(alias_include_script);
        const std::vector<core::Finding> findings = script_loader.run_check_file(files);

        bool saw_alias = false;
        bool saw_include = false;
        for (const core::Finding &finding : findings) {
            if (finding.rule_id == "library.alias-specification") {
                saw_alias = true;
            }
            if (finding.rule_id == "library.include-directories-src") {
                saw_include = true;
            }
        }
        REQUIRE(saw_alias);
        REQUIRE(saw_include);
    }

    // Scenario A: graph-aware rename across two files.
    TEST_CASE("ChaiScriptEndToEnd: rename target across files via graph query") {
        io::InMemoryFileSystem fs;
        fs.create_directories("/virtual_project/libxe-alpha");
        fs.write_file(
            "/virtual_project/libxe-alpha/CMakeLists.txt",
            "set (target \"alpha_legacy_internal\")\n"
            "set (sources \"src/Alpha.cpp\")\n"
            "add_library(${target} ${sources})\n"
        );
        fs.create_directories("/virtual_project/xe-app");
        fs.write_file(
            "/virtual_project/xe-app/CMakeLists.txt",
            "set (target \"xe-app\")\n"
            "add_executable(${target} src/main.cpp)\n"
            "target_link_libraries(${target} PRIVATE alpha_legacy_internal)\n"
        );

        const io::DefaultCmakeParser parser(fs);
        const io::ProjectLoader loader(fs, parser);
        auto files = loader.load_project("/virtual_project");

        // Build the semantic graph from link commands, resolving ${target}.
        analysis::DirectedDependencyGraph graph;
        for (const core::ConcreteSyntaxTree &tree : files) {
            std::string target_name;
            for (const core::CommandNode *cmd : tree.commands()) {
                if (cmd->name == "set" && cmd->argument_count() >= 2 && cmd->argument(0).text == "target") {
                    target_name = cmd->argument(1).text;
                }
            }
            for (const core::CommandNode *cmd : tree.commands()) {
                if (cmd->name != "target_link_libraries" || cmd->argument_count() < 2) {
                    continue;
                }
                std::string source = cmd->argument(0).text;
                if (source == "${target}" && !target_name.empty()) {
                    source = target_name;
                }
                for (std::size_t i = 1; i < cmd->argument_count(); ++i) {
                    const std::string &dep = cmd->argument(i).text;
                    if (dep == "PUBLIC" || dep == "PRIVATE" || dep == "INTERFACE") {
                        continue;
                    }
                    graph.add_edge(source, dep);
                }
            }
        }
        REQUIRE(testing::requireGraphAcyclic(graph));
        REQUIRE(graph.has_edge("xe-app", "alpha_legacy_internal"));

        // Script that renames the declared target when it mismatches the folder.
        script::ScriptRuleLoader script_loader;
        script_loader.load(R"(
def check_file(ctx, file) {
    var folder = "";
    var parts = str_split(file.path(), "/");
    if (parts.size() >= 2) {
        folder = parts[parts.size() - 2];
    }
    var has_set_target = false;
    var target_arg_span = SourceSpan();
    var target_arg_text = "";
    var cmds = file.commands();
    for (var i = 0; i < cmds.size(); ++i) {
        var cmd = cmds[i];
        if (cmd.name() == "set" && cmd.argument_count() >= 2 && cmd.argument(0).text() == "target") {
            has_set_target = true;
            target_arg_span = cmd.argument(1).span();
            target_arg_text = cmd.argument(1).text();
        }
    }
    if (has_set_target) {
        if (target_arg_text != folder) {
            ctx.report(Finding("naming.target-matches-folder", Severity.Error,
                               "target '" + target_arg_text + "' != folder '" + folder + "'",
                               target_arg_span));
        }
    }
}
)");
        const std::vector<core::Finding> findings = script_loader.run_check_file(files);
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].rule_id == "naming.target-matches-folder");
    }

    // Scenario C: Catch2 test specification completeness.
    TEST_CASE("ChaiScriptEndToEnd: flag missing Catch2 declarations") {
        io::InMemoryFileSystem fs;
        fs.create_directories("/virtual_project/libxe-math-test");
        fs.write_file(
            "/virtual_project/libxe-math-test/CMakeLists.txt",
            "set (target \"libxe-math-test\")\n"
            "set (sources \"src/MathTest.cpp\")\n"
            "add_executable(${target} ${sources})\n"
        );

        const io::DefaultCmakeParser parser(fs);
        const io::ProjectLoader loader(fs, parser);
        const auto files = loader.load_project("/virtual_project");

        script::ScriptRuleLoader script_loader;
        script_loader.load(R"chai(
def check_file(ctx, file) {
    var find_catch2 = null;
    var discover = null;
    var cmds = file.commands();
    for (var i = 0; i < cmds.size(); ++i) {
        var cmd = cmds[i];
        if (cmd.name() == "find_package" && cmd.argument_count() >= 1 && cmd.argument(0).text() == "Catch2") {
            find_catch2 = cmd;
        }
        if (cmd.name() == "catch_discover_tests") {
            discover = cmd;
        }
    }
    if (find_catch2 == null) {
        ctx.report(Finding("testing.catch2-specification", Severity.Error,
                           "Missing find_package(Catch2 REQUIRED)", file.commands()[0].span()));
    }
    if (discover == null) {
        ctx.report(Finding("testing.catch2-specification", Severity.Error,
                           "Missing catch_discover_tests", file.commands()[file.commands().size() - 1].span()));
    }
}
)chai");
        const std::vector<core::Finding> findings = script_loader.run_check_file(files);
        REQUIRE(findings.size() == 2);
        for (const core::Finding &finding : findings) {
            REQUIRE(finding.rule_id == "testing.catch2-specification");
            REQUIRE(finding.severity == core::Severity::Error);
        }
    }

} // namespace xe::cmake