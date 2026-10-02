#include "xe/cmake/rules/RuleRegistry.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake::rules {

    TEST_CASE("RuleRegistry registers DSL rules and applies overrides") {
        RuleRegistry registry;
        xe::cmake::dsl::DslRule rule;
        rule.id = "structure.single-target-per-folder";
        rule.severity = xe::cmake::core::RuleSeverity::Warn;
        std::vector<xe::cmake::dsl::DslRule> rules;
        rules.push_back(rule);
        registry.add_dsl_rules(rules);
        REQUIRE(registry.rule_count() == 1);
        REQUIRE(registry.effective_severity("structure.single-target-per-folder", xe::cmake::core::RuleSeverity::Warn) == xe::cmake::core::RuleSeverity::Warn);

        registry.set_severity_override("structure.single-target-per-folder", xe::cmake::core::RuleSeverity::Off);
        REQUIRE(registry.effective_severity("structure.single-target-per-folder", xe::cmake::core::RuleSeverity::Warn) == xe::cmake::core::RuleSeverity::Off);
    }

    TEST_CASE("RuleRegistry registers script loaders") {
        RuleRegistry registry;
        auto loader = std::make_shared<xe::cmake::script::ScriptRuleLoader>();
        registry.add_script_loader(loader);
        REQUIRE(registry.script_loaders().size() == 1);
        REQUIRE(registry.rule_count() == 1);
    }

    TEST_CASE("RuleRegistry default severity falls back for unknown rules") {
        RuleRegistry registry;
        REQUIRE(registry.effective_severity("unknown.rule", xe::cmake::core::RuleSeverity::Error) == xe::cmake::core::RuleSeverity::Error);
    }

} // namespace xe::cmake::rules