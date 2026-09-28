#include "xe/cmake/io/DefaultCmakeParser.h"
#include "xe/cmake/io/FileSystem.h"
#include "xe/cmake/io/ProjectLoader.h"
#include "xe/cmake/io/SyncWriter.h"
#include "xe/cmake/rules/CheckRunner.h"
#include "xe/cmake/rules/FixConflictResolver.h"
#include "xe/cmake/rules/RuleRegistry.h"
#include "xe/cmake/dsl/YamlRuleLoader.h"
#include "xe/cmake/script/ScriptBindings.h"
#include "xe/cmake/testing/CstSyntheticGenerator.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

namespace xe::cmake {

    using namespace xe::cmake;

    namespace {

        rules::RuleRegistry quote_and_split_registry() {
            rules::RuleRegistry registry;
            dsl::DslRule quote;
            quote.id = "formatting.quote-source-paths";
            quote.severity = core::RuleSeverity::Warn;
            quote.match_node = dsl::DslNodeMatch::Argument;
            quote.when = "cmd.name == 'set' && cmd.argument_count > 1 && cmd.argument(0).text == 'sources' && arg.index > 0 && !arg.is_quoted";
            quote.message = "Source file '${arg.text}' must be quoted";
            quote.fix = dsl::DslFixSpec{std::string("quote_argument"), std::string(), {}};
            registry.add_dsl_rules({quote});

            dsl::DslRule split;
            split.id = "formatting.target-link-single-dependency";
            split.severity = core::RuleSeverity::Warn;
            split.match_node = dsl::DslNodeMatch::Command;
            split.match_name = "target_link_libraries";
            split.when = "count(cmd.arguments, a -> a.index > 1 && a.text not in ['PUBLIC', 'PRIVATE', 'INTERFACE']) > 1";
            split.message = "one dependency per line";
            split.fix = dsl::DslFixSpec{std::string("split_target_link_libraries_per_line"), std::string(), {}};
            registry.add_dsl_rules({split});
            return registry;
        }

    } // namespace

    TEST_CASE("RuleEngineIntegration full pipeline with fixes and second pass") {
        io::InMemoryFileSystem fs;
        fs.create_directories("/v/libxe-alpha");
        fs.write_file(
            "/v/libxe-alpha/CMakeLists.txt",
            "set (target \"libxe-alpha\")\n"
            "set (sources src/A.cpp src/B.cpp)\n"
            "add_library(${target} ${sources})\n"
        );
        fs.create_directories("/v/xe-app");
        fs.write_file(
            "/v/xe-app/CMakeLists.txt",
            "set (target \"xe-app\")\n"
            "add_executable(${target} src/main.cpp)\n"
            "target_link_libraries(${target} PRIVATE libxe-alpha libxe-beta PUBLIC libxe-gamma)\n"
        );

        const io::DefaultCmakeParser parser(fs);
        const io::ProjectLoader loader(fs, parser);
        auto files = loader.load_project("/v");
        REQUIRE(files.size() == 2);

        rules::RuleRegistry registry = quote_and_split_registry();
        const rules::CheckRunner runner(registry);
        std::vector<core::Finding> findings = runner.run(files, {});
        REQUIRE(findings.size() == 3);

        const rules::FixConflictResolver resolver;
        const rules::FixConflictResolver::Result result = resolver.resolve(findings);
        REQUIRE(result.applied_indices.size() == 3);

        const io::SyncWriter writer(fs);
        writer.apply(result.workspace);

        std::string alpha_content;
        REQUIRE(fs.read_file("/v/libxe-alpha/CMakeLists.txt", alpha_content));
        REQUIRE(alpha_content.find("\"src/A.cpp\"") != std::string::npos);
        REQUIRE(alpha_content.find("\"src/B.cpp\"") != std::string::npos);

        std::string app_content;
        REQUIRE(fs.read_file("/v/xe-app/CMakeLists.txt", app_content));
        REQUIRE(app_content.find("PRIVATE libxe-alpha") != std::string::npos);
        REQUIRE(app_content.find("PRIVATE libxe-beta") != std::string::npos);
        REQUIRE(app_content.find("PUBLIC libxe-gamma") != std::string::npos);

        // Second pass must be clean.
        const auto reparsed = loader.load_project("/v");
        const std::vector<core::Finding> recheck = runner.run(reparsed, {});
        REQUIRE(recheck.empty());
    }

    TEST_CASE("RuleEngineIntegration graph-driven rules") {
        io::InMemoryFileSystem fs;
        fs.create_directories("/v/a");
        fs.write_file("/v/a/CMakeLists.txt", "set (target \"a\")\nadd_library(${target} src/A.cpp)\n");
        fs.create_directories("/v/b");
        fs.write_file("/v/b/CMakeLists.txt", "set (target \"b\")\nadd_library(${target} src/B.cpp)\n");
        fs.create_directories("/v/app");
        fs.write_file(
            "/v/app/CMakeLists.txt",
            "set (target \"app\")\nadd_executable(${target} src/main.cpp)\n"
            "target_link_libraries(${target} PRIVATE a b)\n"
        );

        const io::DefaultCmakeParser parser(fs);
        const io::ProjectLoader loader(fs, parser);
        const auto files = loader.load_project("/v");

        // Build a semantic graph from the link commands.
        analysis::DirectedDependencyGraph graph;
        for (const core::ConcreteSyntaxTree &tree : files) {
            std::string target_name;
            for (const core::CommandNode *cmd : tree.commands()) {
                if (cmd->name == "set" && cmd->argument_count() >= 2 && cmd->argument(0).text == "target") {
                    target_name = cmd->argument(1).text;
                }
            }
            for (const core::CommandNode *cmd : tree.commands()) {
                if (cmd->name == "target_link_libraries" && cmd->argument_count() >= 2) {
                    std::string source = cmd->argument(0).text;
                    if (source == "${target}") {
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
        }
        REQUIRE(graph.has_edge("app", "a"));
        REQUIRE(graph.has_edge("app", "b"));
        REQUIRE(graph.find_cycles().empty());

        // A ChaiScript rule that inspects the graph.
        rules::RuleRegistry registry;
        auto loader_ptr = std::make_shared<script::ScriptRuleLoader>();
        loader_ptr->load(R"(
def check_project(ctx, project, graph) {
    if (graph.has_edge("app", "a") && graph.has_target("b")) {
        var incoming = graph.incoming_edges("a");
        if (incoming.size() == 1) {
            ctx.report(Finding("proxy.graphcheck", Severity.Info, "ok", SourceSpan()));
        }
    }
}
)");
        registry.add_script_loader(loader_ptr);

        const rules::CheckRunner runner(registry);
        const std::vector<core::Finding> findings = runner.run(files, graph);
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].rule_id == "proxy.graphcheck");
    }

    TEST_CASE("RuleEngineIntegration file-level rule over empty and rich files") {
        rules::RuleRegistry registry;
        dsl::DslRule file_rule;
        file_rule.id = "structure.has-target";
        file_rule.severity = core::RuleSeverity::Error;
        file_rule.match_node = dsl::DslNodeMatch::File;
        file_rule.when = "count(file.commands, c -> c.name in ['add_library', 'add_executable']) > 0";
        file_rule.message = "found ${len(file.commands)} commands";
        registry.add_dsl_rules({file_rule});

        const auto rich = testing::parse_cst("/v/CMakeLists.txt", "set (a 1)\nadd_library(lib)\nadd_executable(app)\n");
        const auto empty = testing::parse_cst("/v/empty/CMakeLists.txt", "");
        const auto sparse = testing::parse_cst("/v/sparse/CMakeLists.txt", "set (a 1)\n");

        const rules::CheckRunner runner(registry);
        const std::vector<core::Finding> findings = runner.run({rich, empty, sparse}, {});
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].file_path == "/v/CMakeLists.txt");
    }

    TEST_CASE("RuleEngineIntegration multiline comments and bracket args parse") {
        const auto cst = testing::parse_cst(
            "/v/CMakeLists.txt",
            "# header\n"
            "set (target \"lib\")\n"
            "#[=[ block\ncomment ]=]\n"
            "set (sources src/A.cpp \"src/B.cpp\")\n"
            "add_library(${target} ${sources})\n"
        );
        REQUIRE_FALSE(cst.has_errors());
        REQUIRE(cst.commands().size() == 3);

        rules::RuleRegistry registry;
        dsl::DslRule quote_rule;
        quote_rule.id = "test.quote";
        quote_rule.severity = core::RuleSeverity::Warn;
        quote_rule.match_node = dsl::DslNodeMatch::Argument;
        quote_rule.when = "cmd.name == 'set' && arg.index > 0 && !arg.is_quoted && arg.index == 1";
        quote_rule.message = "quote it";
        quote_rule.fix = dsl::DslFixSpec{std::string("quote_argument"), std::string(), {}};
        registry.add_dsl_rules({quote_rule});

        const rules::CheckRunner runner(registry);
        const std::vector<core::Finding> findings = runner.run({cst}, {});
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].span.start_line == 5);
    }

    TEST_CASE("RuleEngineIntegration overlapping fixes require manual intervention") {
        std::vector<core::Finding> findings;
        core::Finding a;
        a.file_path = "/v/a.txt";
        a.rule_id = "overlap.a";
        core::Fix fix_a("a");
        fix_a.add_edit(core::TextEdit::replace(core::SourceSpan{0, 20, 1, 1, 1, 1}, "X"));
        a.fix = fix_a;
        findings.push_back(a);

        core::Finding b;
        b.file_path = "/v/a.txt";
        b.rule_id = "overlap.b";
        core::Fix fix_b("b");
        fix_b.add_edit(core::TextEdit::replace(core::SourceSpan{10, 30, 1, 1, 1, 1}, "Y"));
        b.fix = fix_b;
        findings.push_back(b);

        const rules::FixConflictResolver resolver;
        const rules::FixConflictResolver::Result result = resolver.resolve(findings);
        REQUIRE(result.applied_indices.size() == 1);
        REQUIRE(result.manual_indices.size() == 1);
    }

    TEST_CASE("RuleEngineIntegration severity overrides suppress rules") {
        rules::RuleRegistry registry;
        dsl::DslRule rule;
        rule.id = "test.suppress";
        rule.severity = core::RuleSeverity::Warn;
        rule.match_node = dsl::DslNodeMatch::Command;
        rule.when = "cmd.name == 'add_library'";
        rule.message = "m";
        registry.add_dsl_rules({rule});
        registry.set_severity_override("test.suppress", core::RuleSeverity::Off);

        const auto cst = testing::parse_cst("/v/CMakeLists.txt", "add_library(lib)");
        const rules::CheckRunner runner(registry);
        REQUIRE(runner.run({cst}, {}).empty());
    }

    TEST_CASE("RuleEngineIntegration mixed DSL and script rules combine") {
        rules::RuleRegistry registry;
        dsl::DslRule dsl_rule;
        dsl_rule.id = "test.dsl";
        dsl_rule.severity = core::RuleSeverity::Warn;
        dsl_rule.match_node = dsl::DslNodeMatch::Command;
        dsl_rule.when = "cmd.name == 'set'";
        dsl_rule.message = "dsl hit";
        registry.add_dsl_rules({dsl_rule});

        auto script_loader = std::make_shared<script::ScriptRuleLoader>();
        script_loader->load(R"(
def check_command(ctx, cmd) {
    if (cmd.name() == "add_library") {
        ctx.report(Finding("test.script", Severity.Warn, "script hit", cmd.span()));
    }
}
)");
        registry.add_script_loader(script_loader);

        const auto cst = testing::parse_cst("/v/CMakeLists.txt", "set (a 1)\nadd_library(lib)\n");
        const rules::CheckRunner runner(registry);
        const std::vector<core::Finding> findings = runner.run({cst}, {});
        REQUIRE(findings.size() == 2);
    }

    // Runs the materialized guidelines YAML through the full rule engine over a
    // diverse synthetic project, exercising the DSL evaluator, Lexer, Fix
    // templates, and CheckRunner in one integrated pass.
    TEST_CASE("RuleEngineIntegration materialized guidelines over synthetic project") {
        const std::string yaml = R"yaml(
rules:
  - id: target.declaration-uses-variable
    severity: error
    match:
      node: command
    when: "cmd.name in ['add_library', 'add_executable'] && cmd.argument_count > 0 && cmd.argument(0).text != '${target}'"
    message: "Target declaration must use ${target}"
  - id: formatting.quote-source-paths
    severity: warn
    match:
      node: argument
    when: "cmd.name == 'set' && cmd.argument_count > 1 && cmd.argument(0).text == 'sources' && arg.index > 0 && !arg.is_quoted"
    message: "Source must be quoted"
    fix:
      template: quote_argument
  - id: formatting.target-link-single-dependency
    severity: warn
    match:
      node: command
      name: "target_link_libraries"
    when: "count(cmd.arguments, a -> a.index > 1 && a.text not in ['PUBLIC', 'PRIVATE', 'INTERFACE']) > 1"
    message: "One line per dependency"
    fix:
      template: split_target_link_libraries_per_line
  - id: target.variable-definition
    severity: error
    match:
      node: file
    when: "!file.has_command('set') || count(file.commands, c -> c.name == 'set' && c.argument_count >= 2 && c.argument(0).text == 'target') == 0"
    message: "Missing set(target)"
)yaml";
        const std::vector<dsl::DslRule> rules = dsl::YamlRuleLoader::load_from_string(yaml);
        REQUIRE(rules.size() == 4);

        rules::RuleRegistry registry;
        registry.add_dsl_rules(rules);

        io::InMemoryFileSystem fs;
        fs.create_directories("/v/alpha");
        fs.write_file("/v/alpha/CMakeLists.txt", "set (target \"alpha\")\nset (sources src/A.cpp src/B.cpp)\nadd_library(${target} ${sources})\n");
        fs.create_directories("/v/bad");
        fs.write_file(
            "/v/bad/CMakeLists.txt",
            "add_library(my_lib src/X.cpp)\n"
            "target_link_libraries(my_lib PRIVATE alpha beta)\n"
        );

        const io::DefaultCmakeParser parser(fs);
        const io::ProjectLoader loader(fs, parser);
        const auto files = loader.load_project("/v");

        const rules::CheckRunner runner(registry);
        std::vector<core::Finding> findings = runner.run(files, {});

        // alpha: 2 unquoted sources. bad: 1 direct declaration, 1 split link,
        // 1 missing set(target).
        REQUIRE(findings.size() == 5);

        const rules::FixConflictResolver resolver;
        const rules::FixConflictResolver::Result result = resolver.resolve(findings);
        REQUIRE(result.applied_indices.size() == 3);

        const io::SyncWriter writer(fs);
        writer.apply(result.workspace);

        const auto reparsed = loader.load_project("/v");
        const std::vector<core::Finding> recheck = runner.run(reparsed, {});
        // The two unquoted sources and the split link are repaired; the direct
        // declaration and missing set(target) have no automated fix.
        REQUIRE(recheck.size() == 2);
    }

    TEST_CASE("RuleEngineIntegration DSL evaluator rich predicates") {
        rules::RuleRegistry registry;
        dsl::DslRule count_rule;
        count_rule.id = "test.count";
        count_rule.severity = core::RuleSeverity::Warn;
        count_rule.match_node = dsl::DslNodeMatch::Command;
        count_rule.when = "cmd.name == 'target_link_libraries' && len(filter(cmd.arguments, a -> a.index > 1 && a.text not in ['PUBLIC','PRIVATE','INTERFACE'])) >= 2";
        count_rule.message = "many deps";
        registry.add_dsl_rules({count_rule});

        dsl::DslRule path_rule;
        path_rule.id = "test.path";
        path_rule.severity = core::RuleSeverity::Info;
        path_rule.match_node = dsl::DslNodeMatch::File;
        path_rule.when = "regex_search(file.path, 'libxe-.*') && contains(file.path, 'cmake')";
        path_rule.message = "path ${file.path}";
        registry.add_dsl_rules({path_rule});

        const auto cst = testing::parse_cst(
            "/v/libxe-cmake-core/CMakeLists.txt",
            "set (target \"libxe-core\")\n"
            "add_library(${target} ${sources})\n"
            "target_link_libraries(${target} PRIVATE a b c)\n"
        );
        const rules::CheckRunner runner(registry);
        const std::vector<core::Finding> findings = runner.run({cst}, {});
        REQUIRE(findings.size() == 2);
    }

    // Exercises the Lexer's diverse-syntax branches and the DSL evaluator's
    // operators via the rule engine, killing transitively-linked mutants.
    TEST_CASE("RuleEngineIntegration lexer and evaluator edge coverage") {
        const auto cst = testing::parse_cst(
            "/v/CMakeLists.txt",
            "  # header\n"
            "set (target \"libxe-core\")\r\n"
            "set (sources src/A.cpp src/B.cpp)\n"
            "add_library(${target} ${sources})\n"
            "add_library(xe::core ALIAS ${target})\n"
            "target_link_libraries(${target} PRIVATE libxe-a libxe-b PUBLIC libxe-c)\n"
            "#[=[ block ]=]\n"
            "set (banner [=[hello\nworld]=])\n"
            "set (escaped a\\ value)\n"
        );
        REQUIRE_FALSE(cst.has_errors());
        REQUIRE(cst.commands().size() == 7);

        rules::RuleRegistry registry;
        dsl::DslRule bracket_rule;
        bracket_rule.id = "test.bracket";
        bracket_rule.severity = core::RuleSeverity::Info;
        bracket_rule.match_node = dsl::DslNodeMatch::Command;
        bracket_rule.when = "cmd.argument_count > 1 && cmd.argument(1).is_bracket && cmd.argument(1).text.length == 11";
        bracket_rule.message = "bracket";
        registry.add_dsl_rules({bracket_rule});

        dsl::DslRule split_rule;
        split_rule.id = "test.split";
        split_rule.severity = core::RuleSeverity::Warn;
        split_rule.match_node = dsl::DslNodeMatch::Command;
        split_rule.when = "cmd.name == 'target_link_libraries' && 'libxe-c' in cmd.arguments";
        split_rule.message = "has c";
        registry.add_dsl_rules({split_rule});

        dsl::DslRule compare_rule;
        compare_rule.id = "test.compare";
        compare_rule.severity = core::RuleSeverity::Warn;
        compare_rule.match_node = dsl::DslNodeMatch::Command;
        compare_rule.when = "cmd.name <= 'zzz' && cmd.name >= 'aaa' && cmd.name != 'setx' && cmd.name == 'set' && cmd.argument_count == 2 && cmd.argument(0).text == 'target'";
        compare_rule.message = "compare";
        registry.add_dsl_rules({compare_rule});

        dsl::DslRule count_rule;
        count_rule.id = "test.count";
        count_rule.severity = core::RuleSeverity::Warn;
        count_rule.match_node = dsl::DslNodeMatch::Command;
        count_rule.when = "count(cmd.arguments, a -> a.index > 1 && a.text not in ['PUBLIC', 'PRIVATE', 'INTERFACE']) == 3";
        count_rule.message = "three deps";
        registry.add_dsl_rules({count_rule});

        const rules::CheckRunner runner(registry);
        const std::vector<core::Finding> findings = runner.run({cst}, {});
        REQUIRE(findings.size() == 3);
    }

    TEST_CASE("RuleEngineIntegration quote_kind and index properties") {
        const auto cst = testing::parse_cst("/v/CMakeLists.txt", "set (a \"quoted\" raw [=[bracket]=])\n");
        REQUIRE_FALSE(cst.has_errors());

        rules::RuleRegistry registry;
        dsl::DslRule quoted_rule;
        quoted_rule.id = "test.quoted";
        quoted_rule.severity = core::RuleSeverity::Info;
        quoted_rule.match_node = dsl::DslNodeMatch::Argument;
        quoted_rule.when = "arg.is_quoted && arg.quote_kind == 'quoted' && arg.index == 1";
        quoted_rule.message = "quoted";
        registry.add_dsl_rules({quoted_rule});

        dsl::DslRule raw_rule;
        raw_rule.id = "test.raw";
        raw_rule.severity = core::RuleSeverity::Info;
        raw_rule.match_node = dsl::DslNodeMatch::Argument;
        raw_rule.when = "arg.is_raw && arg.index == 2";
        raw_rule.message = "raw";
        registry.add_dsl_rules({raw_rule});

        dsl::DslRule bracket_rule;
        bracket_rule.id = "test.bracketkind";
        bracket_rule.severity = core::RuleSeverity::Info;
        bracket_rule.match_node = dsl::DslNodeMatch::Argument;
        bracket_rule.when = "arg.is_bracket && arg.index == 3";
        bracket_rule.message = "bracket";
        registry.add_dsl_rules({bracket_rule});

        const rules::CheckRunner runner(registry);
        const std::vector<core::Finding> findings = runner.run({cst}, {});
        REQUIRE(findings.size() == 3);
    }

    // Exercises the DSL evaluator's higher-order functions, path utilities and
    // comparison operators through DSL rule execution, killing the evaluator's
    // transitively-linked mutants in the rule-engine gate.
    TEST_CASE("RuleEngineIntegration DSL evaluator higher-order coverage") {
        const auto cst = testing::parse_cst(
            "/v/libxe-core/CMakeLists.txt",
            "set (target \"libxe-core\")\n"
            "set (sources src/A.cpp src/B.cpp src/C.cpp)\n"
            "add_library(${target} ${sources})\n"
            "add_executable(app src/main.cpp)\n"
            "target_link_libraries(${target} PRIVATE libxe-a libxe-b PUBLIC libxe-c)\n"
        );
        REQUIRE_FALSE(cst.has_errors());

        rules::RuleRegistry registry;
        dsl::DslRule all_rule;
        all_rule.id = "test.all";
        all_rule.severity = core::RuleSeverity::Info;
        all_rule.match_node = dsl::DslNodeMatch::Command;
        all_rule.when = "cmd.name == 'set' && all(cmd.arguments, a -> a.index <= 1 || a.text != 'missing')";
        all_rule.message = "all ok";
        registry.add_dsl_rules({all_rule});

        dsl::DslRule exists_rule;
        exists_rule.id = "test.exists";
        exists_rule.severity = core::RuleSeverity::Info;
        exists_rule.match_node = dsl::DslNodeMatch::File;
        exists_rule.when = "exists(file.commands, c -> c.name == 'add_executable')";
        exists_rule.message = "has exe";
        registry.add_dsl_rules({exists_rule});

        dsl::DslRule filter_rule;
        filter_rule.id = "test.filter";
        filter_rule.severity = core::RuleSeverity::Info;
        filter_rule.match_node = dsl::DslNodeMatch::Command;
        filter_rule.when = "len(filter(cmd.arguments, a -> a.index > 1 && a.text not in ['PUBLIC','PRIVATE','INTERFACE'])) == 3";
        filter_rule.message = "filtered";
        registry.add_dsl_rules({filter_rule});

        dsl::DslRule first_rule;
        first_rule.id = "test.first";
        first_rule.severity = core::RuleSeverity::Info;
        first_rule.match_node = dsl::DslNodeMatch::Command;
        first_rule.when = "cmd.name == 'set' && cmd.first_arg.text == 'target' && cmd.last_arg.text == 'libxe-core'";
        first_rule.message = "first last";
        registry.add_dsl_rules({first_rule});

        dsl::DslRule path_rule;
        path_rule.id = "test.pathutils";
        path_rule.severity = core::RuleSeverity::Info;
        path_rule.match_node = dsl::DslNodeMatch::File;
        path_rule.when = "path_basename(file.path) == 'CMakeLists.txt' && path_dirname(file.path) == '/v/libxe-core' && file.folder_name == 'libxe-core'";
        path_rule.message = "paths";
        registry.add_dsl_rules({path_rule});

        dsl::DslRule regex_rule;
        regex_rule.id = "test.regex";
        regex_rule.severity = core::RuleSeverity::Info;
        regex_rule.match_node = dsl::DslNodeMatch::Argument;
        regex_rule.when = "regex_match(arg.text, 'libxe-.*') && regex_search(file.path, 'libxe-core')";
        regex_rule.message = "regex";
        registry.add_dsl_rules({regex_rule});

        dsl::DslRule split_rule;
        split_rule.id = "test.splitrule";
        split_rule.severity = core::RuleSeverity::Info;
        split_rule.match_node = dsl::DslNodeMatch::File;
        split_rule.when = "len(split(file.path, '/')) == 3 && to_lower(file.folder_name) == 'libxe-core'";
        split_rule.message = "split";
        registry.add_dsl_rules({split_rule});

        const rules::CheckRunner runner(registry);
        const std::vector<core::Finding> findings = runner.run({cst}, {});
        // all x2 (set), exists x1, filter x1 (tll), first x1 (set target),
        // path x1, regex x2 (libxe-a, libxe-b, libxe-c args), split x1
        REQUIRE(findings.size() == 10);
    }

    TEST_CASE("RuleEngineIntegration InMemoryFileSystem edge cases") {
        io::InMemoryFileSystem fs;
        // Root-level keys without leading slash and nested prefixes.
        fs.write_file("CMakeLists.txt", "set (a 1)\n");
        fs.create_directories("/v");
        fs.write_file("/v/libxe-a/CMakeLists.txt", "set (target \"a\")\n");
        fs.write_file("/v/libxe-a/src/A.cpp", "int x;");

        REQUIRE(fs.exists("CMakeLists.txt"));
        REQUIRE(fs.is_file("CMakeLists.txt"));
        REQUIRE_FALSE(fs.is_directory("CMakeLists.txt"));
        REQUIRE(fs.is_directory("/v"));
        REQUIRE(fs.is_directory("/v/libxe-a"));
        REQUIRE(fs.is_file("/v/libxe-a/CMakeLists.txt"));
        REQUIRE_FALSE(fs.exists("/v/nope"));

        // create_directories idempotency and pre-existing entries.
        REQUIRE(fs.create_directories("/v"));
        REQUIRE(fs.create_directories("/v/libxe-a"));

        // list_directory includes files and nested dirs.
        const std::vector<std::string> root_entries = fs.list_directory("/");
        REQUIRE(!root_entries.empty());
        const std::vector<std::string> v_entries = fs.list_directory("/v");
        REQUIRE(v_entries.size() == 1);
        REQUIRE(v_entries[0] == "libxe-a");
        const std::vector<std::string> lib_entries = fs.list_directory("/v/libxe-a");
        REQUIRE(lib_entries.size() == 2);

        std::string content;
        REQUIRE(fs.read_file("/v/libxe-a/src/A.cpp", content));
        REQUIRE(content == "int x;");
    }

    TEST_CASE("RuleEngineIntegration DSL tokenizer and evaluator battery") {
        const auto cst = testing::parse_cst(
            "/v/CMakeLists.txt",
            "set (target \"libxe-core\")\n"
            "set (count 5)\n"
            "target_link_libraries(${target} PRIVATE a b)\n"
        );
        REQUIRE_FALSE(cst.has_errors());

        rules::RuleRegistry registry;
        const std::vector<std::pair<std::string, std::string>> cases = {
            // double-quoted strings, !=, <=, >=, == operators
            {"t1", "cmd.name == \"set\" && cmd.argument(0).text != \"sources\""},
            {"t2", "cmd.argument_count <= 3 && cmd.argument_count >= 2 && cmd.argument_count == 2"},
            {"t3", "cmd.name == \"set\" && cmd.argument(1).text == \"target\""},
            // lambda arrow and comments
            {"t4", "count(cmd.arguments, a -> a.index > 0 && a.text != 'x') -- comment\n>= 1"},
            {"t5", "len(cmd.arguments) >= 1 && !(cmd.argument_count == 0)"},
            // in / not in lists
            {"t6", "cmd.name in ['set', 'add_library'] && 'target' in cmd.arguments"},
            {"t7", "cmd.name not in ['add_executable'] && 'b' not in ['x', 'y']"},
            // numeric comparisons and regex
            {"t8", "regex_search(cmd.argument(0).text, '^tar') || regex_match(cmd.name, '^[a-z_]+$')"},
            // string length and casing
            {"t9", "to_upper(cmd.argument(0).text) == 'TARGET' && cmd.argument(0).text.length == 6"},
            // path utilities
            {"t10", "ends_with(file.path, '.txt') && starts_with(file.path, '/v/') && contains(file.path, 'CMake')"},
        };
        for (const auto &entry : cases) {
            dsl::DslRule rule;
            rule.id = "battery." + entry.first;
            rule.severity = core::RuleSeverity::Info;
            rule.match_node = dsl::DslNodeMatch::Command;
            rule.when = entry.second;
            rule.message = "hit " + entry.first;
            registry.add_dsl_rules({rule});
        }

        const rules::CheckRunner runner(registry);
        const std::vector<core::Finding> findings = runner.run({cst}, {});
        REQUIRE(findings.size() == 20);
    }

    TEST_CASE("RuleEngineIntegration DSL tokenizer boundary paths") {
        const auto cst = testing::parse_cst("/v/CMakeLists.txt", "set (target \"libxe-core\")\nset (a 1)\n");
        REQUIRE_FALSE(cst.has_errors());

        rules::RuleRegistry registry;
        // `->` at non-zero position, `--` comment at line start, `!=`, and
        // parenthesized negation exercise the tokenizer's char-boundary paths.
        const std::vector<std::pair<std::string, std::string>> cases = {
            {"c1", "cmd.argument_count == 2 || cmd.name == 'x'"},
            {"c2", "cmd.name != 'x' -- trailing\n&& cmd.argument_count >= 1"},
            {"c3", "! (cmd.name == 'y') && cmd.name == 'set'"},
            {"c4", "cmd.argument(0).text in ['target'] || cmd.name == 'missing'"},
            {"c5", "len(cmd.arguments) == 2 && cmd.argument(0).index == 0"},
            {"c6", "cmd.argument(1).text == 'libxe-core' && cmd.argument(0).text == 'target'"},
            {"c7", "starts_with(cmd.argument(0).text, 'ta') && ends_with(cmd.argument(0).text, 'get')"},
            {"c8", "to_lower(to_upper(cmd.name)) == 'set'"},
            {"c9", "'x' in split('x,y', ',') || 'y' in split('x,y', ',')"},
            {"c10", "count(cmd.arguments, a -> a.text == 'target') >= 1 && len(cmd.arguments) < 3"},
        };
        for (const auto &entry : cases) {
            dsl::DslRule rule;
            rule.id = "boundary." + entry.first;
            rule.severity = core::RuleSeverity::Info;
            rule.match_node = dsl::DslNodeMatch::Command;
            rule.when = entry.second;
            rule.message = "b " + entry.first;
            registry.add_dsl_rules({rule});
        }

        const rules::CheckRunner runner(registry);
        const std::vector<core::Finding> findings = runner.run({cst}, {});
        REQUIRE(!findings.empty());
    }

    TEST_CASE("RuleEngineIntegration diverse project exercises Lexer and DSL") {
        io::InMemoryFileSystem fs;
        fs.create_directories("/p/libxe-a");
        fs.write_file(
            "/p/libxe-a/CMakeLists.txt",
            "# comment\n"
            "set (target \"libxe-a\")\n"
            "set (sources src/A.cpp \"src/B.cpp\" src/C.cpp)\n"
            "add_library(${target} ${sources})\n"
            "add_library(xe::a ALIAS ${target})\n"
            "target_include_directories(${target} PUBLIC \"src\")\n"
            "target_link_libraries(${target} PRIVATE libxe-b)\n"
        );
        fs.create_directories("/p/libxe-b");
        fs.write_file(
            "/p/libxe-b/CMakeLists.txt",
            "set (target \"libxe-b\")\n"
            "add_library(${target} src/B.cpp)\n"
            "target_link_libraries(${target} PRIVATE libxe-c)\n"
        );
        fs.create_directories("/p/xe-app");
        fs.write_file(
            "/p/xe-app/CMakeLists.txt",
            "set (target \"xe-app\")\n"
            "add_executable(${target} src/main.cpp)\n"
            "target_link_libraries(${target} PRIVATE libxe-a libxe-b)\n"
        );
        fs.create_directories("/p/weird");
        fs.write_file(
            "/p/weird/CMakeLists.txt",
            "cmake_minimum_required (VERSION 3.25)\r\n"
            "project (Weird)\n"
            "#[=[ block\ncomment ]=]\n"
            "set (banner [=[hello\nworld]=])\n"
            "set (escaped a\\ value \"quo\\\"ted\")\n"
        );

        const io::DefaultCmakeParser parser(fs);
        const io::ProjectLoader loader(fs, parser);
        const auto files = loader.load_project("/p");
        REQUIRE(files.size() == 4);
        for (const auto &tree : files) {
            REQUIRE_FALSE(tree.has_errors());
        }

        rules::RuleRegistry registry;
        dsl::DslRule quote_rule;
        quote_rule.id = "integ.quote";
        quote_rule.severity = core::RuleSeverity::Warn;
        quote_rule.match_node = dsl::DslNodeMatch::Argument;
        quote_rule.when = "cmd.name == 'set' && cmd.argument(0).text == 'sources' && arg.index > 0 && !arg.is_quoted";
        quote_rule.message = "quote";
        quote_rule.fix = dsl::DslFixSpec{std::string("quote_argument"), std::string(), {}};
        registry.add_dsl_rules({quote_rule});

        dsl::DslRule bracket_rule;
        bracket_rule.id = "integ.bracket";
        bracket_rule.severity = core::RuleSeverity::Info;
        bracket_rule.match_node = dsl::DslNodeMatch::Command;
        bracket_rule.when = "cmd.name == 'set' && cmd.argument_count > 1 && cmd.argument(1).is_bracket";
        bracket_rule.message = "bracket";
        registry.add_dsl_rules({bracket_rule});

        dsl::DslRule target_rule;
        target_rule.id = "integ.target";
        target_rule.severity = core::RuleSeverity::Error;
        target_rule.match_node = dsl::DslNodeMatch::Command;
        target_rule.when = "cmd.name in ['add_library', 'add_executable'] && cmd.argument(0).text != '${target}'";
        target_rule.message = "target var";
        registry.add_dsl_rules({target_rule});

        dsl::DslRule file_rule;
        file_rule.id = "integ.file";
        file_rule.severity = core::RuleSeverity::Info;
        file_rule.match_node = dsl::DslNodeMatch::File;
        file_rule.when = "len(file.commands) >= 3 && file.has_command('add_library')";
        file_rule.message = "file";
        registry.add_dsl_rules({file_rule});

        const rules::CheckRunner runner(registry);
        const std::vector<core::Finding> findings = runner.run(files, {});
        REQUIRE(findings.size() >= 5);
    }

    TEST_CASE("RuleEngineIntegration evaluator value comparisons") {
        const auto cst = testing::parse_cst("/v/CMakeLists.txt", "set (a 1)\nadd_library(lib)\n");
        REQUIRE_FALSE(cst.has_errors());

        rules::RuleRegistry registry;
        const std::vector<std::pair<std::string, std::string>> cases = {
            // bool equality and logical negation
            {"b1", "cmd.name == 'set' && (true == true) && (false == false)"},
            {"b2", "cmd.name == 'set' && !false && !(cmd.name == 'nope')"},
            // null comparisons
            {"b3", "cmd.argument(5) == null && cmd.first_arg != null && cmd.last_arg != null"},
            // int and string comparisons through values_equal
            {"b4", "cmd.argument_count == 2 && cmd.argument(0).text == 'a'"},
            {"b5", "cmd.argument(1).text != 'zzz' && cmd.argument(0).text != 'b'"},
            // comparison operators on strings
            {"b6", "cmd.name < 'zzz' && cmd.name > 'aaa'"},
            // in on strings and lists
            {"b7", "'a' in cmd.arguments && 'set' in ['set', 'x']"},
            {"b8", "'z' not in cmd.arguments && 'q' not in ['a', 'b']"},
            // mixed and/or
            {"b9", "(cmd.name == 'set' && cmd.argument_count == 2) || (cmd.name == 'x' && false)"},
            {"b10", "(cmd.name == 'nope' && true) || (cmd.argument_count > 0 && cmd.argument_count < 5)"},
        };
        for (const auto &entry : cases) {
            dsl::DslRule rule;
            rule.id = "values." + entry.first;
            rule.severity = core::RuleSeverity::Info;
            rule.match_node = dsl::DslNodeMatch::Command;
            rule.when = entry.second;
            rule.message = "v " + entry.first;
            registry.add_dsl_rules({rule});
        }

        const rules::CheckRunner runner(registry);
        const std::vector<core::Finding> findings = runner.run({cst}, {});
        // Rules match across commands; the exact count is not the focus here.
        REQUIRE(findings.size() >= 10);
    }

    TEST_CASE("RuleEngineIntegration message interpolation paths") {
        const auto cst = testing::parse_cst("/v/libxe-core/CMakeLists.txt", "set (target \"libxe-core\")\nadd_library(${target} src/A.cpp)\n");
        REQUIRE_FALSE(cst.has_errors());

        rules::RuleRegistry registry;
        dsl::DslRule int_rule;
        int_rule.id = "interp.int";
        int_rule.severity = core::RuleSeverity::Info;
        int_rule.match_node = dsl::DslNodeMatch::Command;
        int_rule.when = "cmd.name == 'add_library'";
        int_rule.message = "argc=${cmd.argument_count} name=${cmd.name}";
        registry.add_dsl_rules({int_rule});

        dsl::DslRule bool_rule;
        bool_rule.id = "interp.bool";
        bool_rule.severity = core::RuleSeverity::Info;
        bool_rule.match_node = dsl::DslNodeMatch::Command;
        bool_rule.when = "cmd.name == 'add_library'";
        bool_rule.message = "islib=${cmd.name == 'add_library'}";
        registry.add_dsl_rules({bool_rule});

        dsl::DslRule literal_rule;
        literal_rule.id = "interp.literal";
        literal_rule.severity = core::RuleSeverity::Info;
        literal_rule.match_node = dsl::DslNodeMatch::Command;
        literal_rule.when = "cmd.name == 'add_library'";
        literal_rule.message = "escape=${unknown_expr} tail";
        registry.add_dsl_rules({literal_rule});

        const rules::CheckRunner runner(registry);
        const std::vector<core::Finding> findings = runner.run({cst}, {});
        REQUIRE(findings.size() == 3);
        REQUIRE(findings[0].message == "argc=2 name=add_library");
        REQUIRE(findings[1].message == "islib=true");
        REQUIRE(findings[2].message == "escape=${unknown_expr} tail");
    }

    TEST_CASE("RuleEngineIntegration script proxy null-path coverage") {
        script::ScriptArgument arg;
        REQUIRE(arg.text().empty());
        REQUIRE(arg.quote_kind() == "raw");
        REQUIRE(arg.index() == 0);
        REQUIRE(arg.span().start_offset() == 0);

        script::ScriptCommand cmd;
        REQUIRE(cmd.name().empty());
        REQUIRE(cmd.argument_count() == 0);
        REQUIRE(cmd.file_path().empty());
        REQUIRE(cmd.arguments().empty());
        REQUIRE(cmd.argument(0).text().empty());
        REQUIRE(cmd.span().end_offset() == 0);

        script::ScriptFile file;
        REQUIRE(file.path().empty());
        REQUIRE(file.commands().empty());
        REQUIRE(file.find_commands("set").empty());

        script::ScriptGraph graph;
        REQUIRE(graph.node_ids().empty());
        REQUIRE(graph.incoming_edges("a").empty());
        REQUIRE(graph.outgoing_edges("a").empty());
        REQUIRE_FALSE(graph.has_target("a"));
        REQUIRE_FALSE(graph.has_edge("a", "b"));

        script::ScriptGraphEdge edge;
        REQUIRE(edge.source().empty());
        REQUIRE(edge.target().empty());
        REQUIRE(edge.attribute("kind").empty());

        script::ScriptFinding finding;
        REQUIRE(finding.rule_id().empty());
        REQUIRE(finding.message().empty());
        REQUIRE_FALSE(finding.has_fix());

        script::ScriptFix fix;
        REQUIRE(fix.description().empty());
        REQUIRE(fix.edits().empty());

        script::ScriptContext context;
        REQUIRE(context.current_file_path().empty());

        // Live nodes exercise the non-null branches.
        const auto cst = testing::parse_cst("/v/CMakeLists.txt", "set (target \"lib\")");
        script::ScriptCommand live;
        live.node = cst.commands()[0];
        REQUIRE(live.name() == "set");
        REQUIRE(live.argument_count() == 2);
        REQUIRE(live.argument(1).text() == "lib");
        REQUIRE(live.arguments().size() == 2);

        script::ScriptFile live_file;
        live_file.tree = &cst;
        REQUIRE(live_file.path() == "/v/CMakeLists.txt");
        REQUIRE(live_file.commands().size() == 1);
    }

} // namespace xe::cmake