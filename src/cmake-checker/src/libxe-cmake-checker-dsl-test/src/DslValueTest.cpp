#include "xe/cmake/dsl/DslValue.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake::dsl {

    TEST_CASE("DslValue factories produce the expected kinds") {
        const DslValue null_value = DslValue::null();
        REQUIRE(null_value.kind == DslValue::Kind::Null);
        REQUIRE_FALSE(null_value.is_truthy());

        const DslValue bool_true = DslValue::make_bool(true);
        REQUIRE(bool_true.kind == DslValue::Kind::Bool);
        REQUIRE(bool_true.is_truthy());

        const DslValue bool_false = DslValue::make_bool(false);
        REQUIRE_FALSE(bool_false.is_truthy());

        const DslValue int_zero = DslValue::make_int(0);
        REQUIRE(int_zero.kind == DslValue::Kind::Int);
        REQUIRE_FALSE(int_zero.is_truthy());

        const DslValue int_positive = DslValue::make_int(5);
        REQUIRE(int_positive.is_truthy());

        const DslValue empty_string = DslValue::make_string("");
        REQUIRE_FALSE(empty_string.is_truthy());

        const DslValue non_empty_string = DslValue::make_string("x");
        REQUIRE(non_empty_string.is_truthy());

        const DslValue empty_list = DslValue::make_list({});
        REQUIRE_FALSE(empty_list.is_truthy());

        const DslValue non_empty_list = DslValue::make_list({DslValue::make_int(1)});
        REQUIRE(non_empty_list.is_truthy());

        const DslValue node_value = DslValue::make_node(DslNodeRef());
        REQUIRE(node_value.kind == DslValue::Kind::Node);
        REQUIRE(node_value.is_truthy());
    }

} // namespace xe::cmake::dsl