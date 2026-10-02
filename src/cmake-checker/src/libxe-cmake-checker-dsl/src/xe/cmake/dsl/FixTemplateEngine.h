#ifndef XE_CMAKE_DSL_FIX_TEMPLATE_ENGINE_H
#define XE_CMAKE_DSL_FIX_TEMPLATE_ENGINE_H

#include "DslRule.h"

#include "xe/cmake/core/MutationEngine.h"

#include <optional>
#include <string>
#include <vector>

namespace xe::cmake::dsl {

    // Applies the built-in declarative fix templates. A template inspects the
    // matched node and produces TextEdits, or returns an empty optional when the
    // template is not applicable to the node.
    class FixTemplateEngine {
    public:
        static std::optional<xe::cmake::core::Fix> apply_template(std::string_view template_name, const DslContext &context);
    };

} // namespace xe::cmake::dsl

#endif // XE_CMAKE_DSL_FIX_TEMPLATE_ENGINE_H