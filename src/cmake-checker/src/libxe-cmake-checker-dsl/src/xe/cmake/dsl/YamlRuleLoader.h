#ifndef XE_CMAKE_DSL_YAML_RULE_LOADER_H
#define XE_CMAKE_DSL_YAML_RULE_LOADER_H

#include "DslRule.h"

#include <string>
#include <string_view>
#include <vector>

namespace xe::cmake::dsl {

    // Loads declarative YAML rules from a rules file (or an in-memory buffer)
    // using rapidyaml.
    class YamlRuleLoader {
    public:
        // Parses the given YAML document into a list of rules.
        static std::vector<DslRule> load_from_string(std::string_view yaml);

        // Loads rules from a file on disk.
        static std::vector<DslRule> load_from_file(std::string_view path);
    };

} // namespace xe::cmake::dsl

#endif // XE_CMAKE_DSL_YAML_RULE_LOADER_H