
#include "xe/math/Range.h"
#include <catch2/catch_all.hpp>
#include <iostream>

TEST_CASE("Range default constructor should initialize range with values 0 and 1", "[math][range]") {
    xe::TRange<float> subject;
    REQUIRE(subject.max >= subject.min);
    REQUIRE(subject.min == 0.0f);
    REQUIRE(subject.max == 1.0f);
}

TEST_CASE("Range two parameter constructor should initialize range with values correctly set", "[math][range]") {
    xe::TRange<float> subject = xe::TRange<float>{1.0f, -1.0f};

    REQUIRE(subject.max >= subject.min);
    REQUIRE(subject.min == -1.0f);
    REQUIRE(subject.max == 1.0f);
}

TEST_CASE("Range expand should not mutate the range if supplied value is inside range", "[math][range]") {
    xe::TRange<float> subject;

    const float values[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};

    for (const float value : values) {
        subject.expand(value);
        REQUIRE(subject.max >= subject.min);
        REQUIRE(subject.min == 0.0f);
        REQUIRE(subject.max == 1.0f);
    }
}

TEST_CASE("Range expand should mutate the range if supplied value is outside range", "[math][range]") {
    xe::TRange<float> subject;

    subject.expand(-1.0f);
    REQUIRE(subject.max >= subject.min);
    REQUIRE(subject.min == -1.0f);
    REQUIRE(subject.max == 1.0f);

    subject.expand(2.0f);
    REQUIRE(subject.max >= subject.min);
    REQUIRE(subject.min == -1.0f);
    REQUIRE(subject.max == 2.0f);

    subject.expand(-2.0f);
    REQUIRE(subject.max >= subject.min);
    REQUIRE(subject.min == -2.0f);
    REQUIRE(subject.max == 2.0f);
}

TEST_CASE("Range partialOverlap should return true when overlapping with supplied range", "[math][range]") {
    xe::TRange<float> subject;

    const xe::TRange<float> ranges[] = {
        xe::TRange<float>{0.5f, 1.5f},
        xe::TRange<float>{-1.5f, 0.5f},
        xe::TRange<float>{0.25f, 1.5f},
        xe::TRange<float>{-1.5f, 0.75f},
        xe::TRange<float>{-10.5f, 0.1f},
        xe::TRange<float>{-1.5f, 2.0f}
    };

    for (const auto &range : ranges) {
        REQUIRE(subject.partialOverlap(range));
    }
}

TEST_CASE("Range overlap should return true when current range overlaps with supplied range", "[math][range]") {
    xe::TRange<float> subject;

    const xe::TRange<float> ranges[] = {
        xe::TRange<float>{0.5f, 1.5f},
        xe::TRange<float>{-1.5f, 0.5f},
        xe::TRange<float>{0.25f, 1.5f},
        xe::TRange<float>{-1.5f, 0.75f},
        xe::TRange<float>{-10.5f, 0.1f},
        xe::TRange<float>{-1.5f, 2.0f},
        xe::TRange<float>{-1.0f, 1.0f},
        xe::TRange<float>{0.0f, 1.0f},
        xe::TRange<float>{-2.0f, 2.0f},
    };

    for (const auto &range : ranges) {
        REQUIRE(subject.overlap(range));
    }
}

TEST_CASE("Range overlap should return false when using tightly positioned ranges", "[math][range]") {
    for (int i = -10; i < 10; i++) {
        for (float offset = 0.0f; offset < 1.0f; offset += 0.125f) {
            const float width = 1.0f;

            const xe::TRange<float> subject1{(i + 0) * width + offset, (i + 1) * width + offset};

            const xe::TRange<float> subject2{(i + 1) * width + offset, (i + 2) * width + offset};

            REQUIRE_FALSE(subject1.partialOverlap(subject2));
        }
    }
}

