#include "xe/cmake/rules/CheckRunner.h"
#include "xe/cmake/rules/RuleRegistry.h"
#include "xe/cmake/testing/CstSyntheticGenerator.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake::rules {

    TEST_CASE("CheckRunner executes DSL rules across command nodes") {
        RuleRegistry registry;
        xe::cmake::dsl::DslRule rule;
        rule.id = "test.unquoted-source";
        rule.severity = xe::cmake::core::RuleSeverity::Warn;
        rule.match_node = xe::cmake::dsl::DslNodeMatch::Command;
        rule.when = "cmd.name == 'set' && cmd.argument_count > 1 && cmd.argument(0).text == 'sources'";
        rule.message = "unquoted sources present";
        std::vector<xe::cmake::dsl::DslRule> rules;
        rules.push_back(rule);
        registry.add_dsl_rules(rules);

        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (target \"lib\")\nset (sources src/A.cpp src/B.cpp)\n");
        const CheckRunner runner(registry);
        const std::vector<xe::cmake::core::Finding> findings = runner.run({cst}, {});
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].rule_id == "test.unquoted-source");
    }

    TEST_CASE("CheckRunner applies severity overrides to findings") {
        RuleRegistry registry;
        xe::cmake::dsl::DslRule rule;
        rule.id = "test.warn-rule";
        rule.severity = xe::cmake::core::RuleSeverity::Warn;
        rule.match_node = xe::cmake::dsl::DslNodeMatch::Command;
        rule.when = "cmd.name == 'add_library'";
        rule.message = "m";
        std::vector<xe::cmake::dsl::DslRule> rules;
        rules.push_back(rule);
        registry.add_dsl_rules(rules);
        registry.set_severity_override("test.warn-rule", xe::cmake::core::RuleSeverity::Error);

        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(lib)");
        const CheckRunner runner(registry);
        const std::vector<xe::cmake::core::Finding> findings = runner.run({cst}, {});
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].severity == xe::cmake::core::Severity::Error);
    }

    TEST_CASE("CheckRunner suppresses off rules") {
        RuleRegistry registry;
        xe::cmake::dsl::DslRule rule;
        rule.id = "test.off-rule";
        rule.severity = xe::cmake::core::RuleSeverity::Warn;
        rule.match_node = xe::cmake::dsl::DslNodeMatch::Command;
        rule.when = "true";
        rule.message = "m";
        std::vector<xe::cmake::dsl::DslRule> rules;
        rules.push_back(rule);
        registry.add_dsl_rules(rules);
        registry.set_severity_override("test.off-rule", xe::cmake::core::RuleSeverity::Off);

        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(lib)");
        const CheckRunner runner(registry);
        REQUIRE(runner.run({cst}, {}).empty());
    }

    TEST_CASE("CheckRunner runs script hooks through the registry") {
        RuleRegistry registry;
        auto loader = std::make_shared<xe::cmake::script::ScriptRuleLoader>();
        loader->load(R"(
def check_command(ctx, cmd) {
    if (cmd.name() == "add_library") {
        ctx.report(Finding("custom.library", Severity.Warn, "lib", cmd.span()));
    }
}
)");
        registry.add_script_loader(loader);

        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(lib)");
        const CheckRunner runner(registry);
        const std::vector<xe::cmake::core::Finding> findings = runner.run({cst}, {});
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].rule_id == "custom.library");
    }

    TEST_CASE("CheckRunner file-level rules match the whole listfile") {
        RuleRegistry registry;
        xe::cmake::dsl::DslRule rule;
        rule.id = "test.file-rule";
        rule.severity = xe::cmake::core::RuleSeverity::Warn;
        rule.match_node = xe::cmake::dsl::DslNodeMatch::File;
        rule.when = "file.has_command('add_library') && file.count_commands('add_library') == 1";
        rule.message = "found a library";
        std::vector<xe::cmake::dsl::DslRule> rules;
        rules.push_back(rule);
        registry.add_dsl_rules(rules);

        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)\nadd_library(lib)\n");
        const CheckRunner runner(registry);
        const std::vector<xe::cmake::core::Finding> findings = runner.run({cst}, {});
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].rule_id == "test.file-rule");
        REQUIRE(findings[0].file_path == "/v/CMakeLists.txt");
    }

    TEST_CASE("CheckRunner file-level rule does not match empty file") {
        RuleRegistry registry;
        xe::cmake::dsl::DslRule rule;
        rule.id = "test.file-empty";
        rule.severity = xe::cmake::core::RuleSeverity::Info;
        rule.match_node = xe::cmake::dsl::DslNodeMatch::File;
        rule.when = "file.has_command('set')";
        rule.message = "m";
        std::vector<xe::cmake::dsl::DslRule> rules;
        rules.push_back(rule);
        registry.add_dsl_rules(rules);

        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "");
        const CheckRunner runner(registry);
        REQUIRE(runner.run({cst}, {}).empty());
    }

    TEST_CASE("CheckRunner argument-level rules match per-argument") {
        RuleRegistry registry;
        xe::cmake::dsl::DslRule rule;
        rule.id = "test.argument-rule";
        rule.severity = xe::cmake::core::RuleSeverity::Warn;
        rule.match_node = xe::cmake::dsl::DslNodeMatch::Argument;
        rule.match_name = "set";
        rule.match_pattern = "src/.*";
        rule.when = "arg.index > 0 && !arg.is_quoted";
        rule.message = "unquoted source ${arg.text}";
        std::vector<xe::cmake::dsl::DslRule> rules;
        rules.push_back(rule);
        registry.add_dsl_rules(rules);

        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (sources src/A.cpp src/B.cpp)\n");
        const CheckRunner runner(registry);
        const std::vector<xe::cmake::core::Finding> findings = runner.run({cst}, {});
        REQUIRE(findings.size() == 2);
        REQUIRE(findings[0].rule_id == "test.argument-rule");
        REQUIRE(findings[1].span.start_offset > findings[0].span.start_offset);
    }

    TEST_CASE("CheckRunner argument rule attaches a fix template") {
        RuleRegistry registry;
        xe::cmake::dsl::DslRule rule;
        rule.id = "test.quote-arg";
        rule.severity = xe::cmake::core::RuleSeverity::Warn;
        rule.match_node = xe::cmake::dsl::DslNodeMatch::Argument;
        rule.match_name = "set";
        rule.when = "arg.index > 0 && !arg.is_quoted";
        rule.message = "quote it";
        rule.fix = xe::cmake::dsl::DslFixSpec{std::string("quote_argument"), std::string(), {}};
        std::vector<xe::cmake::dsl::DslRule> rules;
        rules.push_back(rule);
        registry.add_dsl_rules(rules);

        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (sources src/A.cpp)\n");
        const CheckRunner runner(registry);
        const std::vector<xe::cmake::core::Finding> findings = runner.run({cst}, {});
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].has_fix());
        REQUIRE(findings[0].fix->edits.size() == 1);
    }

    TEST_CASE("CheckRunner command rule with message interpolation") {
        RuleRegistry registry;
        xe::cmake::dsl::DslRule rule;
        rule.id = "test.interpolate";
        rule.severity = xe::cmake::core::RuleSeverity::Error;
        rule.match_node = xe::cmake::dsl::DslNodeMatch::Command;
        rule.when = "cmd.name == 'add_library'";
        rule.message = "command ${cmd.name} at ${file.path}";
        std::vector<xe::cmake::dsl::DslRule> rules;
        rules.push_back(rule);
        registry.add_dsl_rules(rules);

        const auto cst = xe::cmake::testing::parse_cst("/v/lib/CMakeLists.txt", "add_library(lib)");
        const CheckRunner runner(registry);
        const std::vector<xe::cmake::core::Finding> findings = runner.run({cst}, {});
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].message == "command add_library at /v/lib/CMakeLists.txt");
        REQUIRE(findings[0].severity == xe::cmake::core::Severity::Error);
    }

} // namespace xe::cmake::rules