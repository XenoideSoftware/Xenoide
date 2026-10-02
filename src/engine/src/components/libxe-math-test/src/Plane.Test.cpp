
#include "xe/math/Plane.h"
#include <catch2/catch_all.hpp>

TEST_CASE("Plane default constructor makes XZ plane pointing plus Y axis", "[math][plane]") {
    xe::Plane subject;

    REQUIRE(subject.a == 0.0f);
    REQUIRE(subject.b == 1.0f);
    REQUIRE(subject.c == 0.0f);
    REQUIRE(subject.d == 0.0f);
}

TEST_CASE("Plane four parameter constructor makes a plane with specific equation terms", "[math][plane]") {
    xe::Plane subject{1.0f, 2.0f, 3.0f, 4.0f};

    REQUIRE(subject.a == 1.0f);
    REQUIRE(subject.b == 2.0f);
    REQUIRE(subject.c == 3.0f);
    REQUIRE(subject.d == 4.0f);
}

TEST_CASE("Plane const data returns a pointer to internal array", "[math][plane]") {
    const xe::Plane subject{1.0f, 2.0f, 3.0f, 4.0f};

    const float *values = subject.data();

    REQUIRE(values[0] == subject.a);
    REQUIRE(values[1] == subject.b);
    REQUIRE(values[2] == subject.c);
    REQUIRE(values[3] == subject.d);

    REQUIRE(values[0] == 1.0f);
    REQUIRE(values[1] == 2.0f);
    REQUIRE(values[2] == 3.0f);
    REQUIRE(values[3] == 4.0f);
}

TEST_CASE("Plane data returns a pointer to internal array", "[math][plane]") {
    xe::Plane subject{1.0f, 2.0f, 3.0f, 4.0f};

    const float *values = subject.data();

    REQUIRE(values[0] == subject.a);
    REQUIRE(values[1] == subject.b);
    REQUIRE(values[2] == subject.c);
    REQUIRE(values[3] == subject.d);

    REQUIRE(values[0] == 1.0f);
    REQUIRE(values[1] == 2.0f);
    REQUIRE(values[2] == 3.0f);
    REQUIRE(values[3] == 4.0f);
}

struct TestCase {
    xe::Plane input{};
    bool output{false};
};

TEST_CASE("Plane operator== checks for equality for each term", "[math][plane]") {
    xe::Plane subject{1.0f, 2.0f, 3.0f, 4.0f};

    const TestCase testCases[] = {
        TestCase{xe::Plane{1.0f, 2.0f, 3.0f, 4.0f}, true},
        TestCase{xe::Plane{-1.0f, 2.0f, 3.0f, 4.0f}, false},
        TestCase{xe::Plane{1.0f, -2.0f, 3.0f, 4.0f}, false},
        TestCase{xe::Plane{1.0f, 2.0f, -3.0f, 4.0f}, false},
        TestCase{xe::Plane{1.0f, 2.0f, 3.0f, -4.0f}, false}
    };

    for (const auto &testCase : testCases) {
        REQUIRE(testCase.output == (subject == testCase.input));
    }
}

TEST_CASE("Plane operator!= checks for inequality for each term", "[math][plane]") {
    xe::Plane subject{1.0f, 2.0f, 3.0f, 4.0f};

    const TestCase testCases[] = {
        TestCase{xe::Plane{1.0f, 2.0f, 3.0f, 4.0f}, false},
        TestCase{xe::Plane{-1.0f, 2.0f, 3.0f, 4.0f}, true},
        TestCase{xe::Plane{1.0f, -2.0f, 3.0f, 4.0f}, true},
        TestCase{xe::Plane{1.0f, 2.0f, -3.0f, 4.0f}, true},
        TestCase{xe::Plane{1.0f, 2.0f, 3.0f, -4.0f}, true}
    };

    for (const auto &testCase : testCases) {
        REQUIRE(testCase.output == (subject != testCase.input));
    }
}

TEST_CASE("Plane vectorial factory method generates a plane from a specific point and normal vector", "[math][plane]") {
    struct VectorialInput {
        xe::Vector3 normal;
        xe::Vector3 position;
    };

    struct VectorialTestCase {
        VectorialInput input;
        xe::Plane output;
    };

    const VectorialTestCase testCases[] = {
        {VectorialInput{xe::Vector3{1.0f, 0.0f, 0.0f}, xe::Vector3{0.0f, 0.0f, 0.0f}}, xe::Plane{1.0f, 0.0f, 0.0f, 0.0f}},
        {VectorialInput{xe::Vector3{0.0f, 1.0f, 0.0f}, xe::Vector3{0.0f, 0.0f, 0.0f}}, xe::Plane{0.0f, 1.0f, 0.0f, 0.0f}},
        {VectorialInput{xe::Vector3{0.0f, 0.0f, 1.0f}, xe::Vector3{0.0f, 0.0f, 0.0f}}, xe::Plane{0.0f, 0.0f, 1.0f, 0.0f}},
        {VectorialInput{xe::Vector3{1.0f, 0.0f, 0.0f}, xe::Vector3{1.0f, 1.0f, 1.0f}}, xe::Plane{1.0f, 0.0f, 0.0f, 1.0f}},
        {VectorialInput{xe::Vector3{0.0f, 1.0f, 0.0f}, xe::Vector3{1.0f, 1.0f, 1.0f}}, xe::Plane{0.0f, 1.0f, 0.0f, 1.0f}},
        {VectorialInput{xe::Vector3{0.0f, 0.0f, 1.0f}, xe::Vector3{1.0f, 1.0f, 1.0f}}, xe::Plane{0.0f, 0.0f, 1.0f, 1.0f}},
    };

    for (const auto &testCase : testCases) {
        const auto &input = testCase.input;
        const auto output = xe::planeVectorial(input.normal, input.position);

        REQUIRE(testCase.output == output);
        REQUIRE(xe::norm2(output.normal()) == Catch::Approx(1.0f));
    }
}

TEST_CASE("Plane evaluate returns result scalar of evaluating point against plane equation", "[math][plane]") {
    using xe::Plane;

    Plane plane{-1.0f, 2.0f, -3.0f, 1.0f};

    REQUIRE(plane.evaluate({0.0f, 0.0f, 0.0f}) == -1.0f);
    REQUIRE(plane.evaluate({1.0f, 1.0f, 1.0f}) == -3.0f);
    REQUIRE(plane.evaluate({-1.0f, 0.0f, 0.0f}) == 0.0f);
    REQUIRE(plane.evaluate({-1.0f, 1.0f, 0.0f}) == 2.0f);
}

TEST_CASE("Plane intersect checks if two planes intersect", "[math][plane]") {
    using xe::Plane;

    Plane plane1{1.0f, 0.0f, 0.0f, 1.0f};
    Plane plane2{0.0f, 1.0f, 0.0f, 1.0f};
    Plane plane3{0.0f, 0.0f, 1.0f, 1.0f};

    REQUIRE(plane1.intersect(plane2));
    REQUIRE(plane2.intersect(plane3));
    REQUIRE(plane3.intersect(plane1));
}

TEST_CASE("Plane test checks how a point relates to a plane", "[math][plane]") {
    using xe::Plane;
    using xe::PlaneSide;

    Plane plane{0.0f, 1.0f, 0.0f, 1.0f};

    REQUIRE(plane.test({0.0f, 2.0f, 0.0f}) == PlaneSide::Front);
    REQUIRE(plane.test({0.0f, -2.0f, 0.0f}) == PlaneSide::Back);
    REQUIRE(plane.test({0.0f, 1.0f, 0.0f}) == PlaneSide::Inside);
}

TEST_CASE("Plane serialization generates a non-empty string", "[math][plane]") {
    xe::Plane subject;

    std::stringstream ss;
    ss << subject;

    REQUIRE(ss.str() != "");
}

TEST_CASE("Plane serialization generates different strings for different planes", "[math][plane]") {
    std::stringstream ss1;
    ss1 << xe::planeXY();

    std::stringstream ss2;
    ss2 << xe::planeYZ();

    REQUIRE(ss1.str() != ss2.str());
}
