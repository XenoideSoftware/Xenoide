#include "xe/cmake/core/SourceSpan.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake::core {

    TEST_CASE("SourceSpan invalid is not valid") {
        const SourceSpan span = SourceSpan::invalid();
        REQUIRE_FALSE(span.is_valid());
    }

    TEST_CASE("SourceSpan validity requires monotonic offsets") {
        const SourceSpan valid{5, 10, 2, 3, 2, 8};
        REQUIRE(valid.is_valid());
        const SourceSpan inverted{10, 5, 2, 3, 2, 8};
        REQUIRE_FALSE(inverted.is_valid());
    }

    TEST_CASE("SourceSpan contains tests nested spans") {
        const SourceSpan outer{0, 20, 1, 1, 2, 5};
        const SourceSpan inner{5, 10, 1, 6, 1, 11};
        REQUIRE(outer.contains(inner));
        REQUIRE_FALSE(inner.contains(outer));
        REQUIRE_FALSE(outer.contains(SourceSpan{15, 25, 1, 1, 1, 1}));
    }

    TEST_CASE("SourceSpan overlaps detects intersection") {
        const SourceSpan left{0, 10, 1, 1, 1, 1};
        const SourceSpan right{8, 20, 1, 1, 1, 1};
        const SourceSpan disjoint{20, 30, 1, 1, 1, 1};
        REQUIRE(left.overlaps(right));
        REQUIRE(right.overlaps(left));
        REQUIRE_FALSE(left.overlaps(disjoint));
    }

    TEST_CASE("SourceSpan equality compares offsets only") {
        REQUIRE(SourceSpan{0, 5, 1, 1, 1, 5} == SourceSpan{0, 5, 2, 3, 2, 7});
        REQUIRE(SourceSpan{0, 5, 1, 1, 1, 5} != SourceSpan{0, 6, 1, 1, 1, 6});
        REQUIRE(SourceSpan{1, 5, 1, 1, 1, 5} != SourceSpan{0, 5, 1, 1, 1, 5});
    }

    TEST_CASE("SourceSpan default construction yields valid empty span") {
        const SourceSpan span;
        REQUIRE(span.start_offset == 0);
        REQUIRE(span.end_offset == 0);
        REQUIRE(span.is_valid());
    }

} // namespace xe::cmake::core