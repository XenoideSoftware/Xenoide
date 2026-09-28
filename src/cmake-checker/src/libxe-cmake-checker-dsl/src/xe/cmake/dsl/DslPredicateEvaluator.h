#ifndef XE_CMAKE_DSL_DSL_PREDICATE_EVALUATOR_H
#define XE_CMAKE_DSL_DSL_PREDICATE_EVALUATOR_H

#include "DslRule.h"

#include <string>
#include <string_view>

namespace xe::cmake::dsl {

    // Evaluates declarative `when:` predicate expressions against CST nodes.
    //
    // The supported expression language is a small, side-effect-free subset:
    //   - literals: 'string', 123, true/false, [a, b, c]
    //   - property access: cmd.name, cmd.argument_count, arg.text, file.path
    //   - method calls: cmd.argument(0), file.has_command('set')
    //   - higher-order sequences: count, exists, all, filter, first, len
    //   - string/regex primitives: regex_match, regex_search, starts_with,
    //     ends_with, contains, split, to_lower, to_upper
    //   - path utilities: path_basename, path_dirname, path_stem, path_extension
    //   - comparison & logical operators: ==, !=, <, <=, >, >=, in, not in, &&, ||, !
    class DslPredicateEvaluator {
    public:
        // Evaluates the predicate against the given context. Returns false for any
        // parse or evaluation error so that malformed rules degrade to no match.
        bool evaluate(std::string_view expression, const DslContext &context) const;

        // Interpolates ${expr} occurrences inside a message using the context.
        std::string interpolate(std::string_view message, const DslContext &context) const;
    };

} // namespace xe::cmake::dsl

#endif // XE_CMAKE_DSL_DSL_PREDICATE_EVALUATOR_H