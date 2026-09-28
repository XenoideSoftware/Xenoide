#ifndef XE_CMAKE_DSL_DSL_VALUE_H
#define XE_CMAKE_DSL_DSL_VALUE_H

#include "xe/cmake/core/ConcreteSyntaxTree.h"
#include "xe/cmake/core/MutationEngine.h"

#include <string>
#include <vector>

namespace xe::cmake::dsl {

    // A reference to a node inside the evaluator. Exactly one pointer is active,
    // selected by the kind tag.
    struct DslNodeRef {
        enum class Kind : uint8_t {
            File,
            Command,
            Argument,
            Span,
        };

        Kind kind = Kind::File;
        const xe::cmake::core::ConcreteSyntaxTree *file = nullptr;
        const xe::cmake::core::CommandNode *command = nullptr;
        const xe::cmake::core::ArgumentNode *argument = nullptr;
        xe::cmake::core::SourceSpan span;
    };

    // Dynamic value produced by the DSL expression evaluator.
    struct DslValue {
        enum class Kind : uint8_t {
            Null,
            Bool,
            Int,
            String,
            List,
            Node,
        };

        Kind kind = Kind::Null;
        bool bool_value = false;
        long long int_value = 0;
        std::string string_value;
        std::vector<DslValue> list_value;
        DslNodeRef node_value;

        static DslValue null() {
            return DslValue();
        }

        static DslValue make_bool(bool value) {
            DslValue result;
            result.kind = Kind::Bool;
            result.bool_value = value;
            return result;
        }

        static DslValue make_int(long long value) {
            DslValue result;
            result.kind = Kind::Int;
            result.int_value = value;
            return result;
        }

        static DslValue make_string(std::string value) {
            DslValue result;
            result.kind = Kind::String;
            result.string_value = std::move(value);
            return result;
        }

        static DslValue make_list(std::vector<DslValue> value) {
            DslValue result;
            result.kind = Kind::List;
            result.list_value = std::move(value);
            return result;
        }

        static DslValue make_node(DslNodeRef value) {
            DslValue result;
            result.kind = Kind::Node;
            result.node_value = std::move(value);
            return result;
        }

        bool is_truthy() const {
            switch (kind) {
            case Kind::Bool:
                return bool_value;
            case Kind::Int:
                return int_value != 0;
            case Kind::String:
                return !string_value.empty();
            case Kind::List:
                return !list_value.empty();
            case Kind::Null:
                return false;
            case Kind::Node:
                return true;
            }
            return false;
        }
    };

} // namespace xe::cmake::dsl

#endif // XE_CMAKE_DSL_DSL_VALUE_H