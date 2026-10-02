#include "YamlRuleLoader.h"

#include <c4/std/string.hpp>
#include <ryml.hpp>
#include <ryml_std.hpp>

#include <fstream>
#include <sstream>

namespace xe::cmake::dsl {
    namespace {

        std::string node_value(ryml::ConstNodeRef node) {
            if (node.invalid() || !node.has_val()) {
                return std::string();
            }
            return std::string(node.val().str, node.val().len);
        }

        xe::cmake::core::RuleSeverity severity_from_string(const std::string &value) {
            if (value == "error") {
                return xe::cmake::core::RuleSeverity::Error;
            }
            if (value == "info") {
                return xe::cmake::core::RuleSeverity::Info;
            }
            if (value == "off") {
                return xe::cmake::core::RuleSeverity::Off;
            }
            return xe::cmake::core::RuleSeverity::Warn;
        }

        DslNodeMatch node_match_from_string(const std::string &value) {
            if (value == "file") {
                return DslNodeMatch::File;
            }
            if (value == "argument") {
                return DslNodeMatch::Argument;
            }
            if (value == "block") {
                return DslNodeMatch::Block;
            }
            return DslNodeMatch::Command;
        }

        DslRule parse_rule(ryml::ConstNodeRef node) {
            DslRule rule;
            if (node.has_child("id")) {
                rule.id = node_value(node["id"]);
            }
            if (node.has_child("severity")) {
                rule.severity = severity_from_string(node_value(node["severity"]));
            }
            if (node.has_child("description")) {
                rule.description = node_value(node["description"]);
            }
            if (node.has_child("when")) {
                rule.when = node_value(node["when"]);
            }
            if (node.has_child("message")) {
                rule.message = node_value(node["message"]);
            }
            if (node.has_child("match")) {
                const ryml::ConstNodeRef match = node["match"];
                if (match.has_child("node")) {
                    rule.match_node = node_match_from_string(node_value(match["node"]));
                }
                if (match.has_child("name")) {
                    rule.match_name = node_value(match["name"]);
                }
                if (match.has_child("pattern")) {
                    rule.match_pattern = node_value(match["pattern"]);
                }
            }
            if (node.has_child("fix")) {
                DslFixSpec fix;
                const ryml::ConstNodeRef fix_node = node["fix"];
                if (fix_node.has_child("template")) {
                    fix.template_name = node_value(fix_node["template"]);
                }
                if (fix_node.has_child("description")) {
                    fix.description = node_value(fix_node["description"]);
                }
                if (fix_node.has_child("edits") && fix_node["edits"].is_seq()) {
                    for (const ryml::ConstNodeRef edit : fix_node["edits"].children()) {
                        DslValue value = DslValue::null();
                        value.string_value = node_value(edit);
                        fix.edits.push_back(value);
                    }
                }
                rule.fix = fix;
            }
            return rule;
        }

    } // namespace

    std::vector<DslRule> YamlRuleLoader::load_from_string(std::string_view yaml) {
        std::vector<DslRule> rules;
        ryml::Tree tree;
        try {
            const std::string yaml_copy(yaml);
            tree = ryml::parse_in_arena(ryml::to_csubstr(yaml_copy));
        } catch (const std::exception &) {
            return rules;
        } catch (...) {
            // rapidyaml may throw its internal assertion type on malformed input.
            return rules;
        }
        const ryml::ConstNodeRef root = tree.crootref();
        if (root.invalid() || !root.has_child("rules") || !root["rules"].is_seq()) {
            return rules;
        }
        for (const ryml::ConstNodeRef rule_node : root["rules"].children()) {
            rules.push_back(parse_rule(rule_node));
        }
        return rules;
    }

    std::vector<DslRule> YamlRuleLoader::load_from_file(std::string_view path) {
        std::ifstream stream{std::string(path)};
        if (!stream) {
            return {};
        }
        std::ostringstream buffer;
        buffer << stream.rdbuf();
        return load_from_string(buffer.str());
    }

} // namespace xe::cmake::dsl