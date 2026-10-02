#ifndef XE_CMAKE_DSL_DSL_RULE_H
#define XE_CMAKE_DSL_DSL_RULE_H

#include "DslValue.h"

#include "xe/cmake/core/MutationEngine.h"

#include <optional>
#include <string>
#include <vector>

namespace xe::cmake::dsl {

    enum class DslNodeMatch : uint8_t {
        File,
        Command,
        Argument,
        Block,
    };

    // The node context against which a DSL predicate is evaluated.
    struct DslContext {
        const xe::cmake::core::ConcreteSyntaxTree *file = nullptr;
        const xe::cmake::core::CommandNode *command = nullptr;
        const xe::cmake::core::ArgumentNode *argument = nullptr;
    };

    // A declarative fix: either a named built-in template or explicit edits.
    struct DslFixSpec {
        std::optional<std::string> template_name;
        std::string description;
        std::vector<DslValue> edits;
    };

    // A single declarative YAML rule.
    struct DslRule {
        std::string id;
        xe::cmake::core::RuleSeverity severity = xe::cmake::core::RuleSeverity::Warn;
        std::string description;
        DslNodeMatch match_node = DslNodeMatch::Command;
        std::string match_name;
        std::string match_pattern;
        std::string when;
        std::string message;
        std::optional<DslFixSpec> fix;
    };

} // namespace xe::cmake::dsl

#endif // XE_CMAKE_DSL_DSL_RULE_H