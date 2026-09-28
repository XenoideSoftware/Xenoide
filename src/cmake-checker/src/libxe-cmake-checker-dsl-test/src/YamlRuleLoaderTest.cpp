#include "xe/cmake/dsl/YamlRuleLoader.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

namespace xe::cmake::dsl {

    TEST_CASE("YamlRuleLoader parses a check-only rule") {
        const std::string yaml = R"(
rules:
  - id: structure.single-target-per-folder
    severity: error
    description: "A single CMake target per folder"
    match:
      node: file
    when: "count(file.commands, c -> c.name in ['add_library', 'add_executable']) > 1"
    message: "A single CMake target should be stored in a given folder"
)";
        const std::vector<DslRule> rules = YamlRuleLoader::load_from_string(yaml);
        REQUIRE(rules.size() == 1);
        REQUIRE(rules[0].id == "structure.single-target-per-folder");
        REQUIRE(rules[0].severity == xe::cmake::core::RuleSeverity::Error);
        REQUIRE(rules[0].match_node == DslNodeMatch::File);
        REQUIRE_FALSE(rules[0].fix.has_value());
        REQUIRE(rules[0].when.find("count(file.commands") != std::string::npos);
    }

    TEST_CASE("YamlRuleLoader parses a rule with a fix template") {
        const std::string yaml = R"(
rules:
  - id: formatting.quote-source-paths
    severity: warn
    match:
      node: argument
    when: "!arg.is_quoted"
    message: "Source must be quoted"
    fix:
      template: quote_argument
)";
        const std::vector<DslRule> rules = YamlRuleLoader::load_from_string(yaml);
        REQUIRE(rules.size() == 1);
        REQUIRE(rules[0].match_node == DslNodeMatch::Argument);
        REQUIRE(rules[0].fix.has_value());
        REQUIRE(rules[0].fix->template_name.has_value());
        REQUIRE(*rules[0].fix->template_name == "quote_argument");
    }

    TEST_CASE("YamlRuleLoader parses custom declarative edits") {
        const std::string yaml = R"(
rules:
  - id: target.declaration-uses-variable
    severity: error
    match:
      node: command
    when: "cmd.argument_count > 0 && cmd.argument(0).text != '${target}'"
    message: "Must use ${target}"
    fix:
      description: "Replace target identifier with ${target}"
      edits:
        - action: replace
          span: cmd.argument(0).span
          content: "${target}"
)";
        const std::vector<DslRule> rules = YamlRuleLoader::load_from_string(yaml);
        REQUIRE(rules.size() == 1);
        REQUIRE(rules[0].fix.has_value());
        REQUIRE(rules[0].fix->edits.size() == 1);
    }

    TEST_CASE("YamlRuleLoader returns empty for invalid YAML") {
        const std::vector<DslRule> rules = YamlRuleLoader::load_from_string("this: is: not: yaml: :");
        REQUIRE(rules.empty());
    }

    TEST_CASE("YamlRuleLoader defaults match node to command") {
        const std::string yaml = R"(
rules:
  - id: test.rule
    severity: warn
    when: "true"
    message: "x"
)";
        const std::vector<DslRule> rules = YamlRuleLoader::load_from_string(yaml);
        REQUIRE(rules.size() == 1);
        REQUIRE(rules[0].match_node == DslNodeMatch::Command);
        REQUIRE(rules[0].severity == xe::cmake::core::RuleSeverity::Warn);
    }

    TEST_CASE("YamlRuleLoader parses info and off severities") {
        const std::string yaml = R"(
rules:
  - id: a.info
    severity: info
    when: "true"
    message: "x"
  - id: b.off
    severity: off
    when: "true"
    message: "y"
)";
        const std::vector<DslRule> rules = YamlRuleLoader::load_from_string(yaml);
        REQUIRE(rules.size() == 2);
        REQUIRE(rules[0].severity == xe::cmake::core::RuleSeverity::Info);
        REQUIRE(rules[1].severity == xe::cmake::core::RuleSeverity::Off);
    }

    TEST_CASE("YamlRuleLoader parses block match and name/pattern filters") {
        const std::string yaml = R"(
rules:
  - id: structure.block
    severity: error
    match:
      node: block
    when: "true"
    message: "x"
  - id: formatting.named
    severity: warn
    match:
      node: command
      name: "target_link_libraries"
      pattern: "libxe-.*"
    when: "cmd.argument_count > 0"
    message: "y"
)";
        const std::vector<DslRule> rules = YamlRuleLoader::load_from_string(yaml);
        REQUIRE(rules.size() == 2);
        REQUIRE(rules[0].match_node == DslNodeMatch::Block);
        REQUIRE(rules[0].severity == xe::cmake::core::RuleSeverity::Error);
        REQUIRE(rules[1].match_name == "target_link_libraries");
        REQUIRE(rules[1].match_pattern == "libxe-.*");
    }

    TEST_CASE("YamlRuleLoader parses fix description without template") {
        const std::string yaml = R"(
rules:
  - id: fix.desc-only
    severity: warn
    when: "true"
    message: "x"
    fix:
      description: "manual remediation"
)";
        const std::vector<DslRule> rules = YamlRuleLoader::load_from_string(yaml);
        REQUIRE(rules.size() == 1);
        REQUIRE(rules[0].fix.has_value());
        REQUIRE_FALSE(rules[0].fix->template_name.has_value());
        REQUIRE(rules[0].fix->description == "manual remediation");
    }

    TEST_CASE("YamlRuleLoader loads a real file") {
        const std::string yaml = "rules:\n  - id: file.rule\n    when: \"true\"\n    message: \"m\"\n";
        const std::string path = "/tmp/xe-cmake-rules-test.yaml";
        std::ofstream out(path, std::ios::trunc);
        out << yaml;
        out.close();
        const std::vector<DslRule> rules = YamlRuleLoader::load_from_file(path);
        REQUIRE(rules.size() == 1);
        REQUIRE(rules[0].id == "file.rule");
        std::filesystem::remove(path);
    }

    TEST_CASE("YamlRuleLoader returns empty for a missing file") {
        REQUIRE(YamlRuleLoader::load_from_file("/missing/xe-rules.yaml").empty());
    }

} // namespace xe::cmake::dsl