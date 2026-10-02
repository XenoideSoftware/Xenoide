#ifndef XE_CMAKE_RULES_RULE_REGISTRY_H
#define XE_CMAKE_RULES_RULE_REGISTRY_H

#include "xe/cmake/dsl/DslRule.h"
#include "xe/cmake/script/ScriptRuleLoader.h"

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace xe::cmake::rules {

    // Unified rule table combining declarative DSL rules and ChaiScript hooks.
    // Also tracks per-rule severity overrides ("error" | "warn" | "info" | "off").
    class RuleRegistry {
    public:
        void add_dsl_rules(std::vector<xe::cmake::dsl::DslRule> rules);
        void add_script_loader(std::shared_ptr<xe::cmake::script::ScriptRuleLoader> loader);

        void set_severity_override(std::string rule_id, xe::cmake::core::RuleSeverity severity);

        const std::vector<xe::cmake::dsl::DslRule> &dsl_rules() const {
            return dsl_rules_;
        }

        const std::vector<std::shared_ptr<xe::cmake::script::ScriptRuleLoader>> &script_loaders() const {
            return script_loaders_;
        }

        // Returns the effective severity for a rule id, applying overrides.
        xe::cmake::core::RuleSeverity effective_severity(std::string_view rule_id, xe::cmake::core::RuleSeverity default_severity) const;

        std::size_t rule_count() const {
            return dsl_rules_.size() + script_loaders_.size();
        }

    private:
        std::vector<xe::cmake::dsl::DslRule> dsl_rules_;
        std::vector<std::shared_ptr<xe::cmake::script::ScriptRuleLoader>> script_loaders_;
        std::map<std::string, xe::cmake::core::RuleSeverity> overrides_;
    };

} // namespace xe::cmake::rules

#endif // XE_CMAKE_RULES_RULE_REGISTRY_H