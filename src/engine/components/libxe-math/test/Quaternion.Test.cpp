#include <catch2/catch_all.hpp>
#include <cmath>
#include "xe/math/Quaternion.h"
#include "xe/math/Common.h"

TEST_CASE("Quaternion vector scalar constructor initializes vector and scalar part", "[math][quaternion]") {
    const auto q2 = xe::Quat{{0.0f, 1.0f, 0.0f}, 1.0f};

    REQUIRE(q2.V.x == Catch::Approx(0.0f));
    REQUIRE(q2.V.y == Catch::Approx(1.0f));
    REQUIRE(q2.V.z == Catch::Approx(0.0f));
    REQUIRE(q2.W == Catch::Approx(1.0f));
}

TEST_CASE("Quaternion four scalar constructor initializes vector and scalar part", "[math][quaternion]") {
    const auto q = xe::Quat{0.0f, 1.0f, 0.0f, 1.0f};

    REQUIRE(q.V.x == Catch::Approx(0.0f));
    REQUIRE(q.V.y == Catch::Approx(1.0f));
    REQUIRE(q.V.z == Catch::Approx(0.0f));
    REQUIRE(q.W == Catch::Approx(1.0f));
}

TEST_CASE("Quaternion one scalar constructor initializes scalar part", "[math][quaternion]") {
    const auto q2 = xe::Quat{1.0f};

    REQUIRE(q2.V.x == Catch::Approx(0.0f));
    REQUIRE(q2.V.y == Catch::Approx(0.0f));
    REQUIRE(q2.V.z == Catch::Approx(0.0f));
    REQUIRE(q2.W == Catch::Approx(1.0f));
}

TEST_CASE("Quaternion vector3 constructor initializes vector part", "[math][quaternion]") {
    const auto q = xe::Quat{{0.0f, 1.0f, 0.0f}};

    REQUIRE(q.V.x == Catch::Approx(0.0f));
    REQUIRE(q.V.y == Catch::Approx(1.0f));
    REQUIRE(q.V.z == Catch::Approx(0.0f));
    REQUIRE(q.W == Catch::Approx(0.0f));
}

TEST_CASE("Quaternion vector4 constructor initializes vector part", "[math][quaternion]") {
    const auto v = xe::Vector4{0.0f, 1.0f, 0.0f, 10.0f};
    const auto q = xe::Quat{v};

    REQUIRE(q.V.x == Catch::Approx(0.0f));
    REQUIRE(q.V.y == Catch::Approx(1.0f));
    REQUIRE(q.V.z == Catch::Approx(0.0f));
    REQUIRE(q.W == Catch::Approx(10.0f));
}

TEST_CASE("Quaternion three scalar constructor initializes vector part", "[math][quaternion]") {
    const auto q = xe::Quat{0.0f, 1.0f, 0.0f};

    REQUIRE(q.V.x == Catch::Approx(0.0f));
    REQUIRE(q.V.y == Catch::Approx(1.0f));
    REQUIRE(q.V.z == Catch::Approx(0.0f));
    REQUIRE(q.W == Catch::Approx(0.0f));
}

TEST_CASE("Quaternion pointer constructor initializes vector and scalar part", "[math][quaternion]") {
    const float values[] = {4.0f, 2.0f, 3.0f, 1.0f};

    const auto q = xe::Quat{values};

    REQUIRE(q.V.x == Catch::Approx(4.0f));
    REQUIRE(q.V.y == Catch::Approx(2.0f));
    REQUIRE(q.V.z == Catch::Approx(3.0f));
    REQUIRE(q.W == Catch::Approx(1.0f));
}

TEST_CASE("Quaternion copy constructor initializes vector and scalar part", "[math][quaternion]") {
    const auto q = xe::Quat{xe::Quat{1.0f, 2.0f, 3.0f, 1.0f}};

    REQUIRE(q.V.x == Catch::Approx(1.0f));
    REQUIRE(q.V.y == Catch::Approx(2.0f));
    REQUIRE(q.V.z == Catch::Approx(3.0f));
    REQUIRE(q.W == Catch::Approx(1.0f));
}

TEST_CASE("Quaternion data should return a pointer to the first element", "[math][quaternion]") {
    const auto quat = xe::Quat{0.0f, 1.0f, 2.0f, 3.0f};
    const float *data = quat.data();

    REQUIRE(data[0] == 0.0f);
    REQUIRE(data[1] == 1.0f);
    REQUIRE(data[2] == 2.0f);
    REQUIRE(data[3] == 3.0f);
}

TEST_CASE("Quaternion size always returns four", "[math][quaternion]") {
    const auto quat = xe::Quat{0.0f, 1.0f, 2.0f, 3.0f};
    REQUIRE(quat.size() == 4);
}

TEST_CASE("Quaternion operator[] should enable access to each element", "[math][quaternion]") {
    auto quat = xe::Quat{0.0f, 1.0f, 2.0f, 3.0f};
    REQUIRE(quat[0] == 0.0f);
    REQUIRE(quat[1] == 1.0f);
    REQUIRE(quat[2] == 2.0f);
    REQUIRE(quat[3] == 3.0f);

    quat[3] = 10.0f;
    REQUIRE(quat[3] == 10.0f);

    const auto quat2 = xe::Quat{10.0f, -1.0f, -2.0f, -3.0f};
    REQUIRE(quat2[0] == 10.0f);
    REQUIRE(quat2[1] == -1.0f);
    REQUIRE(quat2[2] == -2.0f);
    REQUIRE(quat2[3] == -3.0f);
}

TEST_CASE("Quaternion operator== does element-wise comparison via FP tolerance", "[math][quaternion]") {
    const auto qa_1 = xe::Quat{{1.0f, 2.0f, 3.0f}, 1.0f};
    const auto qa_2 = xe::Quat{1.0f, 2.0f, 3.0f, 1.0f};
    const auto qb_1 = xe::Quat{{1.0f, 2.0f, 3.0f}};
    const auto qb_2 = xe::Quat{1.0f, 2.0f, 3.0f};

    REQUIRE(qa_1 == qa_2);
    REQUIRE(qb_1 == qb_2);
    REQUIRE(qa_2 == qa_1);
    REQUIRE(qb_2 == qb_1);
}

TEST_CASE("Quaternion operator!= does element-wise comparison via FP tolerance", "[math][quaternion]") {
    const auto qa_1 = xe::Quat{{1.0f, 2.0f, 3.0f}, 1.0f};
    const auto qa_2 = xe::Quat{1.0f, 2.0f, 3.0f, 1.0f};
    const auto qb_1 = xe::Quat{{1.0f, 2.0f, 3.0f}};
    const auto qb_2 = xe::Quat{1.0f, 2.0f, 3.0f};

    REQUIRE(qa_1 != qb_2);
    REQUIRE(qa_2 != qb_1);

    REQUIRE(qb_1 != qa_2);
    REQUIRE(qb_2 != qa_1);
}

TEST_CASE("Quaternion zero factory method initializes scalar and vector part to zeroes", "[math][quaternion]") {
    const auto q = xe::quatZero<float>();

    REQUIRE(q.V.x == Catch::Approx(0.0f));
    REQUIRE(q.V.y == Catch::Approx(0.0f));
    REQUIRE(q.V.z == Catch::Approx(0.0f));
    REQUIRE(q.W == Catch::Approx(0.0f));
}

TEST_CASE("Quaternion identity factory method initializes scalar part to one", "[math][quaternion]") {
    const auto q = xe::quatId<float>();

    REQUIRE(q.V.x == Catch::Approx(0.0f));
    REQUIRE(q.V.y == Catch::Approx(0.0f));
    REQUIRE(q.V.z == Catch::Approx(0.0f));
    REQUIRE(q.W == Catch::Approx(1.0f));
}

TEST_CASE("Quaternion rotationRH factory method initializes normalized quaternion with half-angle cosine", "[math][quaternion]") {
    const float radians = xe::pi<float>;
    const xe::Vector3 axis{0.0f, 1.0f, 0.0f};

    const auto subject = xe::quatRotationRH<float>(axis, radians);
    const auto correct = xe::normalize(xe::Quat{axis * std::sin(radians * 0.5f), std::cos(radians * 0.5f)});

    REQUIRE(xe::norm(subject) == Catch::Approx(1.0f));
    REQUIRE(subject.V.x == Catch::Approx(correct.V.x));
    REQUIRE(subject.V.y == Catch::Approx(correct.V.y));
    REQUIRE(subject.V.z == Catch::Approx(correct.V.z));
    REQUIRE(subject.W == Catch::Approx(correct.W));
}

TEST_CASE("Quaternion rotationLH factory method initializes normalized quaternion with half-angle cosine", "[math][quaternion]") {
    const float radians = xe::pi<float>;
    const xe::Vector3 axis{0.0f, 1.0f, 0.0f};

    const auto subject = xe::quatRotationLH<float>(axis, radians);
    const auto correct = xe::normalize(xe::Quat{axis * std::sin(radians * 0.5f), std::cos(radians * 0.5f)});

    REQUIRE(subject.V.x == Catch::Approx(-correct.V.x));
    REQUIRE(subject.V.y == Catch::Approx(-correct.V.y));
    REQUIRE(subject.V.z == Catch::Approx(-correct.V.z));
    REQUIRE(subject.W == Catch::Approx(correct.W));
}

TEST_CASE("Quaternion norm2 should compute the squared magnitude", "[math][quaternion]") {
    const auto q = xe::Quat{2.0f, 3.0f, 4.0f, 5.0f};
    REQUIRE(norm2(q) == 54.0f);
}

TEST_CASE("Quaternion norm should compute the magnitude", "[math][quaternion]") {
    const auto q = xe::Quat{0.0f, 3.0f, 0.0f, 4.0f};
    REQUIRE(norm(q) == 5.0f);
}

TEST_CASE("Quaternion conjugate should negate the vector part", "[math][quaternion]") {
    const auto q4 = xe::conjugate(xe::Quat{{-1.0f, 1.0f, -1.0f}, 1.0f});

    REQUIRE(q4.V.x == 1.0f);
    REQUIRE(q4.V.y == -1.0f);
    REQUIRE(q4.V.z == 1.0f);
    REQUIRE(q4.W == 1.0f);
}

TEST_CASE("Quaternion normalize should scale a quaternion to unit length", "[math][quaternion]") {
    const auto q1 = xe::normalize(xe::Quat{{1.0f, 1.0f, 1.0f}, 1.0f});
    REQUIRE(q1.V.x == 0.5f);
    REQUIRE(q1.V.y == 0.5f);
    REQUIRE(q1.V.z == 0.5f);
    REQUIRE(q1.W == 0.5f);

    const auto q2 = xe::normalize(xe::Quat{{0.0f, 4.0f, 0.0f}, 0.0f});
    REQUIRE(q2.V.x == 0.0f);
    REQUIRE(q2.V.y == 1.0f);
    REQUIRE(q2.V.z == 0.0f);
    REQUIRE(q2.W == 0.0f);

    const auto q3 = xe::normalize(xe::Quat{{0.0f, -1.0f, 0.0f}, 0.0f});
    REQUIRE(q3.V.x == 0.0f);
    REQUIRE(q3.V.y == -1.0f);
    REQUIRE(q3.V.z == 0.0f);
    REQUIRE(q3.W == 0.0f);
}

TEST_CASE("Quaternion operator+ should add each component together", "[math][quaternion]") {
    const auto q1 = xe::Quat(1.0f, 0.0f, 1.0f, 1.0f);
    const auto q2 = xe::Quat(0.0f, 1.0f, 0.0f, 1.0f);

    REQUIRE(q1 + q1 == xe::Quat(2.0f, 0.0f, 2.0f, 2.0f));
    REQUIRE(q1 + q2 == xe::Quat(1.0f, 1.0f, 1.0f, 2.0f));
}

TEST_CASE("Quaternion operator- should subtract each component together", "[math][quaternion]") {
    const auto q1 = xe::Quat(1.0f, 0.0f, 1.0f, 1.0f);
    const auto q2 = xe::Quat(0.0f, 1.0f, 0.0f, 1.0f);

    REQUIRE(q1 - q1 == xe::Quat(0.0f, 0.0f, 0.0f, 0.0f));
    REQUIRE(q1 - q2 == xe::Quat(1.0f, -1.0f, 1.0f, 0.0f));
}

TEST_CASE("Quaternion unary operator- should invert each component", "[math][quaternion]") {
    const auto q1 = xe::Quat(1.0f, 0.0f, 1.0f, 1.0f);
    const auto q2 = xe::Quat(0.0f, 1.0f, 0.0f, 1.0f);

    REQUIRE(-q1 == xe::Quat(-1.0f, -0.0f, -1.0f, -1.0f));
    REQUIRE(-q2 == xe::Quat(0.0f, -1.0f, 0.0f, -1.0f));
}

TEST_CASE("Quaternion unary operator+ should modify no components", "[math][quaternion]") {
    const auto q1 = xe::Quat(1.0f, 0.0f, 1.0f, 1.0f);
    const auto q2 = xe::Quat(0.0f, 1.0f, 0.0f, 1.0f);

    REQUIRE(+q1 == xe::Quat(1.0f, 0.0f, 1.0f, 1.0f));
    REQUIRE(+q2 == xe::Quat(0.0f, 1.0f, 0.0f, 1.0f));
}

TEST_CASE("Quaternion operator* by scalar should multiply each component", "[math][quaternion]") {
    const auto q1 = xe::Quat(1.0f, 0.0f, 1.0f, 1.0f);
    const auto q2 = xe::Quat(0.0f, 1.0f, 0.0f, 1.0f);

    REQUIRE(q1 * 1.0f == xe::Quat(1.0f, 0.0f, 1.0f, 1.0f));
    REQUIRE(q2 * 0.0f == xe::Quat(0.0f, 0.0f, 0.0f, 0.0f));
}

TEST_CASE("Quaternion scalar operator* by quaternion should multiply each component", "[math][quaternion]") {
    const auto q1 = xe::Quat(1.0f, 0.0f, 1.0f, 1.0f);
    const auto q2 = xe::Quat(0.0f, 1.0f, 0.0f, 1.0f);

    REQUIRE(1.0f * q1 == xe::Quat(1.0f, 0.0f, 1.0f, 1.0f));
    REQUIRE(0.0f * q2 == xe::Quat(0.0f, 0.0f, 0.0f, 0.0f));
}

TEST_CASE("Quaternion operator/ by scalar should divide each component", "[math][quaternion]") {
    const auto q1 = xe::Quat(1.0f, 0.0f, 1.0f, 1.0f);
    const auto q2 = xe::Quat(0.0f, 1.0f, 0.0f, 1.0f);

    REQUIRE(q1 / 1.0f == xe::Quat(1.0f, 0.0f, 1.0f, 1.0f));
    REQUIRE(q2 / 0.5f == xe::Quat(0.0f, 2.0f, 0.0f, 2.0f));
}

TEST_CASE("Quaternion operator* should combine two quaternions", "[math][quaternion]") {
    const auto q1 = xe::Quat(1.0f, 0.0f, 0.0f, 1.0f);
    const auto q2 = xe::Quat(0.0f, 1.0f, 0.0f, 1.0f);

    const auto r1 = q1 * q2;
    REQUIRE(r1.V.x == Catch::Approx(1.0f));
    REQUIRE(r1.V.y == Catch::Approx(1.0f));
    REQUIRE(r1.V.z == Catch::Approx(1.0f));
    REQUIRE(r1.W == Catch::Approx(1.0f));

    const auto r2 = q2 * q1;
    REQUIRE(r2.V.x == Catch::Approx(1.0f));
    REQUIRE(r2.V.y == Catch::Approx(1.0f));
    REQUIRE(r2.V.z == Catch::Approx(-1.0f));
    REQUIRE(r2.W == Catch::Approx(1.0f));
}

TEST_CASE("Quaternion multiplication by identity quaternion should not have any effect", "[math][quaternion]") {
    const auto q1 = xe::Quat(1.0f, 0.0f, 1.0f, 1.0f);
    const auto q2 = xe::Quat(0.0f, 1.0f, 0.0f, 1.0f);
    const auto qi = xe::quatId<float>();

    REQUIRE(q1 * qi == q1);
    REQUIRE(q2 * qi == q2);
}

TEST_CASE("Quaternion multiplication by zero quaternion should collapse any quaternion to zero", "[math][quaternion]") {
    const auto q1 = xe::Quat(1.0f, 0.0f, 1.0f, 1.0f);
    const auto q2 = xe::Quat(0.0f, 1.0f, 0.0f, 1.0f);
    const auto qz = xe::quatZero<float>();

    REQUIRE(q1 * qz == qz);
    REQUIRE(q2 * qz == qz);
}

TEST_CASE("Quaternion division by identity quaternion should not affect any quaternion", "[math][quaternion]") {
    const auto q1 = xe::Quat(1.0f, 0.0f, 1.0f, 1.0f);
    const auto q2 = xe::Quat(0.0f, 1.0f, 0.0f, 1.0f);
    const auto qi = xe::quatId<float>();

    REQUIRE(q1 / qi == q1);
    REQUIRE(q2 / qi == q2);
}

TEST_CASE("Quaternion division by zero should collapse any quaternion to NaN values", "[math][quaternion]") {
    const auto q1 = xe::Quat(1.0f, 0.0f, 1.0f, 1.0f);
    const auto q2 = xe::Quat(0.0f, 1.0f, 0.0f, 1.0f);
    const auto qz = xe::quatZero<float>();

    const auto r1 = q1 / qz;
    const auto r2 = q2 / qz;

    REQUIRE(std::isnan(r1.V.x));
    REQUIRE(std::isnan(r1.V.y));
    REQUIRE(std::isnan(r1.V.z));
    REQUIRE(std::isnan(r1.W));

    REQUIRE(std::isnan(r2.V.x));
    REQUIRE(std::isnan(r2.V.y));
    REQUIRE(std::isnan(r2.V.z));
    REQUIRE(std::isnan(r2.W));
}

TEST_CASE("Quaternion operator+= should add each component together", "[math][quaternion]") {
    xe::Quat q1{{1.0f, 0.0f, 1.0f}, 1.0f};
    q1 += q1;

    REQUIRE(q1.V.x == Catch::Approx(2.0f));
    REQUIRE(q1.V.y == Catch::Approx(0.0f));
    REQUIRE(q1.V.z == Catch::Approx(2.0f));
    REQUIRE(q1.W == Catch::Approx(2.0f));
}

TEST_CASE("Quaternion operator-= should subtract each component together", "[math][quaternion]") {
    xe::Quat q1{{1.0f, 0.0f, 1.0f}, 1.0f};
    q1 -= xe::Quat(0.0f, 1.0f, 0.0f, 1.0f);

    REQUIRE(q1.V.x == Catch::Approx(1.0f));
    REQUIRE(q1.V.y == Catch::Approx(-1.0f));
    REQUIRE(q1.V.z == Catch::Approx(1.0f));
    REQUIRE(q1.W == Catch::Approx(0.0f));
}

TEST_CASE("Quaternion operator*= by scalar should multiply each component", "[math][quaternion]") {
    xe::Quat q1{{1.0f, 0.0f, 1.0f}, 1.0f};

    q1 *= 3.0f;

    REQUIRE(q1.V.x == Catch::Approx(3.0f));
    REQUIRE(q1.V.y == Catch::Approx(0.0f));
    REQUIRE(q1.V.z == Catch::Approx(3.0f));
    REQUIRE(q1.W == Catch::Approx(3.0f));
}

TEST_CASE("Quaternion operator/= by scalar should divide each component", "[math][quaternion]") {
    xe::Quat q1{{1.0f, 0.0f, 1.0f}, 1.0f};

    q1 /= 1.0f;

    REQUIRE(q1.V.x == Catch::Approx(1.0f));
    REQUIRE(q1.V.y == Catch::Approx(0.0f));
    REQUIRE(q1.V.z == Catch::Approx(1.0f));
    REQUIRE(q1.W == Catch::Approx(1.0f));
}

TEST_CASE("Quaternion norm2 static function should compute squared magnitude", "[math][quaternion]") {
    const auto q = xe::Quat{3.0f, 4.0f, 5.0f, 0.0f};

    REQUIRE(xe::norm2(q) == Catch::Approx(50.0f));
}

TEST_CASE("Quaternion norm static function should compute magnitude", "[math][quaternion]") {
    const auto q = xe::Quat{3.0f, 4.0f, 5.0f, 0.0f};

    REQUIRE(xe::norm(q) == Catch::Approx(7.071067812f));
}

TEST_CASE("Quaternion conjugate static function should return negated vector part", "[math][quaternion]") {
    const auto q = xe::Quat{3.0f, 4.0f, 5.0f, 1.0f};
    const auto conjugate = xe::conjugate(q);

    REQUIRE(conjugate.V.x == Catch::Approx(-3.0f));
    REQUIRE(conjugate.V.y == Catch::Approx(-4.0f));
    REQUIRE(conjugate.V.z == Catch::Approx(-5.0f));
    REQUIRE(conjugate.W == Catch::Approx(1.0f));
}

TEST_CASE("Quaternion normalize static function should return unit length quaternion", "[math][quaternion]") {
    const auto q = xe::Quat{3.0f, 4.0f, 5.0f, 0.0f};
    const auto normalized = xe::normalize(q);

    REQUIRE(xe::norm(normalized) == Catch::Approx(1.0f));
}

TEST_CASE("Quaternion inverse static function should compute a normalized conjugated quaternion", "[math][quaternion]") {
    const auto q1 = xe::Quat{{3.0f, 4.0f, 5.0f}, 0.0f};
    const auto inverse = xe::inverse(q1);

    REQUIRE(inverse.V.x == Catch::Approx(-3.0f / 50.0f));
    REQUIRE(inverse.V.y == Catch::Approx(-4.0f / 50.0f));
    REQUIRE(inverse.V.z == Catch::Approx(-5.0f / 50.0f));
    REQUIRE(inverse.W == Catch::Approx(0.0f));
}

TEST_CASE("Quaternion transform should rotate point vector", "[math][quaternion]") {
    const auto rotation = xe::quatRotationRH({0.0f, 1.0f, 0.0f}, xe::pi<float> * 0.5f);
    const auto point = xe::Vector3{1.0f, 0.0f, 0.0f};
    const auto result = xe::transform(rotation, point);
    const auto e = xe::defaultEpsilon<float>;

    REQUIRE(result.x == Catch::Approx(0.0f).margin(e));
    REQUIRE(result.y == Catch::Approx(0.0f).margin(e));
    REQUIRE(result.z == Catch::Approx(-1.0f).margin(e));
}
