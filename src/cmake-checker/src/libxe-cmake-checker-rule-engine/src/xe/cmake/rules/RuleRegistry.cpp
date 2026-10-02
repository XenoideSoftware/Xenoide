#include "RuleRegistry.h"

namespace xe::cmake::rules {

    void RuleRegistry::add_dsl_rules(std::vector<xe::cmake::dsl::DslRule> rules) {
        for (xe::cmake::dsl::DslRule &rule : rules) {
            dsl_rules_.push_back(std::move(rule));
        }
    }

    void RuleRegistry::add_script_loader(std::shared_ptr<xe::cmake::script::ScriptRuleLoader> loader) {
        script_loaders_.push_back(std::move(loader));
    }

    void RuleRegistry::set_severity_override(std::string rule_id, xe::cmake::core::RuleSeverity severity) {
        overrides_[std::move(rule_id)] = severity;
    }

    xe::cmake::core::RuleSeverity RuleRegistry::effective_severity(std::string_view rule_id, xe::cmake::core::RuleSeverity default_severity) const {
        const auto it = overrides_.find(std::string(rule_id));
        if (it != overrides_.end()) {
            return it->second;
        }
        return default_severity;
    }

} // namespace xe::cmake::rules