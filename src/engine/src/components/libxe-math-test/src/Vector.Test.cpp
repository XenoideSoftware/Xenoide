
#include <catch2/catch_all.hpp>

#include "xe/math/Vector.h"
#include <sstream>

TEST_CASE("Vector default constructor should initialize to zeroes", "[math][vector]") {
    const xe::Vector3 v;

    REQUIRE(v.x == 0.0f);
    REQUIRE(v.y == 0.0f);
    REQUIRE(v.z == 0.0f);
}

TEST_CASE("Vector constructor should setup components correctly", "[math][vector]") {
    const xe::Vector3 v = {1.0f, 2.0f, 3.0f};

    REQUIRE(v.x == 1.0f);
    REQUIRE(v.y == 2.0f);
    REQUIRE(v.z == 3.0f);

    REQUIRE(v.values[0] == 1.0f);
    REQUIRE(v.values[1] == 2.0f);
    REQUIRE(v.values[2] == 3.0f);

    REQUIRE(v[0] == 1.0f);
    REQUIRE(v[1] == 2.0f);
    REQUIRE(v[2] == 3.0f);
}

TEST_CASE("Vector constructor from pointer should setup components correctly", "[math][vector]") {
    const float data[] = {1.0f, 2.0f, 3.0f};
    const xe::Vector3 v{data};

    REQUIRE(v.x == 1.0f);
    REQUIRE(v.y == 2.0f);
    REQUIRE(v.z == 3.0f);
}

TEST_CASE("Vector data method should return an address to the first component", "[math][vector]") {
    xe::Vector3 varv;
    REQUIRE(varv.data() != nullptr);
    REQUIRE(varv.data() == &varv.values[0]);
}

TEST_CASE("Vector const data method should return an address to the first component", "[math][vector]") {
    const xe::Vector3 constv;
    REQUIRE(constv.data() != nullptr);
    REQUIRE(constv.data() == &constv.values[0]);
}

TEST_CASE("Vector cast should convert underlying type", "[math][vector]") {
    const xe::Vector3 vf{1.0f, 2.0f, 3.0f};
    const auto vd = xe::cast<double>(vf);

    REQUIRE(vd.x == 1.0);
    REQUIRE(vd.y == 2.0);
    REQUIRE(vd.z == 3.0);
}

TEST_CASE("Vector comparison operators should check components for equality and inequality", "[math][vector]") {
    const xe::Vector3 v1 = {1.0f, 2.0f, 3.0f};
    const xe::Vector3 v2 = {1.0f, 2.0f, 3.0f};
    const xe::Vector3 v3 = {-1.0f, -2.0f, -3.0f};

    REQUIRE(v1 == v2);
    REQUIRE(v1 != v3);
    REQUIRE(v2 != v3);

    REQUIRE(v2 == v1);
    REQUIRE(v3 != v1);
    REQUIRE(v3 != v2);
}

TEST_CASE("Vector operator+ should add per component", "[math][vector]") {
    const xe::Vector3 v1{1.0f, -2.0f, 3.0f};
    const xe::Vector3 v2{-2.0f, -1.0f, -2.0f};

    REQUIRE((v2 + v1) == xe::Vector3(-1.0f, -3.0f, 1.0f));
    REQUIRE((v1 + v2) == xe::Vector3(-1.0f, -3.0f, 1.0f));
    REQUIRE(v1 == +v1);
    REQUIRE(v2 == +v2);
    REQUIRE(((xe::Vector3(0.0f) += v1) == v1));
    REQUIRE(((xe::Vector3(0.0f) += v2) == v2));
}

TEST_CASE("Vector operator- should subtract component-wise", "[math][vector]") {
    const xe::Vector3 v1{1.0f, -2.0f, 3.0f};
    const xe::Vector3 v2{-2.0f, -1.0f, -2.0f};

    REQUIRE((v2 - v1) == xe::Vector3(-3.0f, 1.0f, -5.0f));
    REQUIRE((v1 - v2) == xe::Vector3(3.0f, -1.0f, 5.0f));
    REQUIRE((v1 - v1) == xe::Vector3(0.0f));
    REQUIRE((v2 - v2) == xe::Vector3(0.0f));

    REQUIRE(xe::Vector3(-1.0f, 2.0f, -3.0f) == -v1);
    REQUIRE(xe::Vector3(2.0f, 1.0f, 2.0f) == -v2);

    REQUIRE(((xe::Vector3(0.0f) -= v1) == -v1));
    REQUIRE(((xe::Vector3(0.0f) -= v2) == -v2));
}

TEST_CASE("Vector operator* should multiply component-wise", "[math][vector]") {
    const xe::Vector3 v1 = {1.0f, -2.0f, 3.0f};
    const xe::Vector3 v2 = {-2.0f, -1.0f, -2.0f};

    REQUIRE((v1 * v2) == xe::Vector3(-2.0f, 2.0f, -6.0f));
    REQUIRE((v2 * v1) == xe::Vector3(-2.0f, 2.0f, -6.0f));

    REQUIRE((v1 * 1.0f) == v1);
    REQUIRE((v1 * -1.0f) == -v1);
    REQUIRE((v1 * 2.0f) == xe::Vector3(2.0f, -4.0f, 6.0f));
    REQUIRE((v2 * 1.0f) == v2);
    REQUIRE((v2 * -1.0f) == -v2);
    REQUIRE((v2 * 2.0f) == xe::Vector3(-4.0f, -2.0f, -4.0f));

    REQUIRE((1.0f * v2) == v2);
    REQUIRE((-1.0f * v2) == -v2);
    REQUIRE((1.0f * v1) == v1);
    REQUIRE((-1.0f * v1) == -v1);

    REQUIRE((1.4f * v1) == (v1 * 1.4f));
    REQUIRE((1.4f * v2) == (v2 * 1.4f));

    REQUIRE(((xe::Vector3(1.0f) *= v1) == v1));
    REQUIRE(((xe::Vector3(1.0f) *= v2) == v2));
}

TEST_CASE("Vector operator*= by scalar should multiply component-wise", "[math][vector]") {
    xe::Vector3 v1 = {1.0f, -2.0f, 3.0f};

    v1 *= -1.0f;

    REQUIRE(v1.x == -1.0f);
    REQUIRE(v1.y == 2.0f);
    REQUIRE(v1.z == -3.0f);
}

TEST_CASE("Vector operator/= by scalar should multiply component-wise", "[math][vector]") {
    xe::Vector3 v1 = {2.0f, -2.0f, 4.0f};

    v1 /= 2.0f;

    REQUIRE(v1.x == 1.0f);
    REQUIRE(v1.y == -1.0f);
    REQUIRE(v1.z == 2.0f);
}

TEST_CASE("Vector operator/ should divide component-wise", "[math][vector]") {
    const xe::Vector3 v1{2.0f, 8.0f, 32.0f};
    const xe::Vector3 v2{1.0f, 2.0f, 4.0f};

    REQUIRE((v1 / v2) == xe::Vector3(2.0f, 4.0f, 8.0f));
    REQUIRE((v1 / 2.0f) == xe::Vector3(1.0f, 4.0f, 16.0f));
    REQUIRE((v2 / 2.0f) == xe::Vector3(0.5f, 1.0f, 2.0f));

    REQUIRE(((xe::Vector3(v1) /= v1) == xe::Vector3(1.0f, 1.0f, 1.0f)));
    REQUIRE(((xe::Vector3(v2) /= v2) == xe::Vector3(1.0f, 1.0f, 1.0f)));
}

TEST_CASE("Vector dot product should compute a sum of products of each component", "[math][vector]") {
    const xe::Vector3 v1 = {2.0f, 8.0f, 32.0f};
    const xe::Vector3 v2 = {1.0f, 2.0f, 4.0f};

    REQUIRE(dot(xe::Vector3(1.0f, 1.0f, 1.0f), v1) == Catch::Approx(42.0f));
    REQUIRE(dot(xe::Vector3(1.0f, 1.0f, 1.0f), v2) == Catch::Approx(7.0f));
    REQUIRE(dot(xe::Vector3(0.0f), v1) == Catch::Approx(0.0f));
    REQUIRE(dot(xe::Vector3(0.0f), v2) == Catch::Approx(0.0f));

    REQUIRE(dot(v1, xe::Vector3(1.0f, 1.0f, 1.0f)) == Catch::Approx(42.0f));
    REQUIRE(dot(v2, xe::Vector3(1.0f, 1.0f, 1.0f)) == Catch::Approx(7.0f));
    REQUIRE(dot(v1, xe::Vector3(0.0f)) == Catch::Approx(0.0f));
    REQUIRE(dot(v2, xe::Vector3(0.0f)) == Catch::Approx(0.0f));

    REQUIRE(dot(v1, v2) == Catch::Approx(146.0f));
    REQUIRE(dot(v2, v1) == Catch::Approx(146.0f));
}

TEST_CASE("Vector triple dot product from unit axis computes one", "[math][vector]") {
    const xe::Vector3 v1 = {1.0f, 0.0f, 0.0f};
    const xe::Vector3 v2 = {0.0f, 1.0f, 0.0f};
    const xe::Vector3 v3 = {0.0f, 0.0f, 1.0f};

    const float result = xe::dot(v1, v2, v3);

    REQUIRE(result == Catch::Approx(1.0f));
}

TEST_CASE("Vector triple dot product from coplanar vector computes zero", "[math][vector]") {
    const xe::Vector3 v1 = {1.0f, 0.0f, 0.0f};
    const xe::Vector3 v2 = {0.0f, 1.0f, 0.0f};
    const xe::Vector3 v3 = {-1.0f, -1.0f, 0.0f};

    REQUIRE(xe::dot(v1, v2, v3) == Catch::Approx(0.0f));
}

TEST_CASE("Vector triple dot product from arbitrary axes computes seven", "[math][vector]") {
    const xe::Vector3 v1 = {1.0f, -1.0f, 1.0f};
    const xe::Vector3 v2 = {2.0f, 1.0f, 1.0f};
    const xe::Vector3 v3 = {1.0f, 1.0f, -2.0f};

    const float result = xe::dot(v1, v2, v3);

    REQUIRE(result == Catch::Approx(-7.0f));
}

TEST_CASE("Vector triple dot product does not change from circular rotation of parameters", "[math][vector]") {
    const xe::Vector3 v1 = {1.0f, -1.0f, 1.0f};
    const xe::Vector3 v2 = {2.0f, 1.0f, 1.0f};
    const xe::Vector3 v3 = {1.0f, 1.0f, -2.0f};

    REQUIRE(xe::dot(v1, v2, v3) == Catch::Approx(xe::dot(v3, v1, v2)));
    REQUIRE(xe::dot(v1, v2, v3) == Catch::Approx(xe::dot(v2, v3, v1)));
}

TEST_CASE("Vector triple dot product changes sign from parameter swapping", "[math][vector]") {
    const xe::Vector3 v1 = {1.0f, -1.0f, 1.0f};
    const xe::Vector3 v2 = {2.0f, 1.0f, 1.0f};
    const xe::Vector3 v3 = {1.0f, 1.0f, -2.0f};

    REQUIRE(xe::dot(v1, v2, v3) == Catch::Approx(-xe::dot(v2, v1, v3)));
    REQUIRE(xe::dot(v1, v2, v3) == Catch::Approx(-xe::dot(v1, v3, v2)));
}

TEST_CASE("Vector two-dimensional cross product should return cross vector length", "[math][vector]") {
    const xe::Vector2 v1 = {2.0f, 0.0f};
    const xe::Vector2 v2 = {0.0f, 2.0f};
    const xe::Vector2 v3 = {0.0f, -1.0f};

    REQUIRE(cross(v1, v2) == 4.0f);
    REQUIRE(cross(v1, v3) == -2.0f);
}

TEST_CASE("Vector three-dimensional cross product should perform correctly for unit vectors", "[math][vector]") {
    const xe::Vector3 v1 = {1.0f, 0.0f, 0.0f};
    const xe::Vector3 v2 = {0.0f, 1.0f, 0.0f};
    const xe::Vector3 v3 = {0.0f, 0.0f, 1.0f};

    REQUIRE(cross(v1, v2) == v3);
    REQUIRE(cross(v2, v1) == -v3);

    REQUIRE(cross(v2, v3) == v1);
    REQUIRE(cross(v3, v2) == -v1);

    REQUIRE(cross(v3, v1) == v2);
    REQUIRE(cross(v1, v3) == -v2);

    REQUIRE(cross(v1, v1) == xe::Vector3(0.0f));
    REQUIRE(cross(v2, v2) == xe::Vector3(0.0f));
    REQUIRE(cross(v3, v3) == xe::Vector3(0.0f));
}

TEST_CASE("Vector triple cross product computes ACB minus ABC product", "[math][vector]") {
    const xe::Vector3 v1 = {1.0f, 0.0f, 0.0f};
    const xe::Vector3 v2 = {0.0f, 1.0f, 0.0f};
    const xe::Vector3 v3 = {0.0f, 0.0f, 1.0f};

    REQUIRE(cross(v1, v2, v3) == dot(v1, v3) * v2 - dot(v1, v2) * v3);
}

TEST_CASE("Vector minimize should return minimum values between two vectors", "[math][vector]") {
    const xe::Vector3 v1 = {1.0f, -2.0f, -1.3f};
    const xe::Vector3 v2 = {1.3f, -1.6f, 0.0f};
    const xe::Vector3 v3 = {-0.3f, 2.0f, 2.0f};

    REQUIRE(minimize(v1, v1) == v1);
    REQUIRE(minimize(v2, v2) == v2);
    REQUIRE(minimize(v3, v3) == v3);

    REQUIRE(minimize(v1, v2) == xe::Vector3(1.0f, -2.0f, -1.3f));
    REQUIRE(minimize(v1, v3) == xe::Vector3(-0.3f, -2.0f, -1.3f));
    REQUIRE(minimize(v2, v3) == xe::Vector3(-0.3f, -1.6f, 0.0f));

    REQUIRE(minimize(v2, v1) == xe::Vector3(1.0f, -2.0f, -1.3f));
    REQUIRE(minimize(v3, v1) == xe::Vector3(-0.3f, -2.0f, -1.3f));
    REQUIRE(minimize(v3, v2) == xe::Vector3(-0.3f, -1.6f, 0.0f));

    REQUIRE(minimize(v1, minimize(v2, v3)) == xe::Vector3(-0.3f, -2.0f, -1.3f));
    REQUIRE(minimize(v1, minimize(v3, v2)) == xe::Vector3(-0.3f, -2.0f, -1.3f));
    REQUIRE(minimize(v2, minimize(v1, v3)) == xe::Vector3(-0.3f, -2.0f, -1.3f));
    REQUIRE(minimize(v2, minimize(v3, v1)) == xe::Vector3(-0.3f, -2.0f, -1.3f));
    REQUIRE(minimize(v3, minimize(v2, v1)) == xe::Vector3(-0.3f, -2.0f, -1.3f));
    REQUIRE(minimize(v3, minimize(v1, v2)) == xe::Vector3(-0.3f, -2.0f, -1.3f));
}

TEST_CASE("Vector maximize function should return maximum values between two vectors", "[math][vector]") {
    const xe::Vector3 v1 = {1.0f, -2.0f, -1.3f};
    const xe::Vector3 v2 = {1.3f, -1.6f, 0.0f};
    const xe::Vector3 v3 = {-0.3f, 2.0f, 2.0f};

    REQUIRE(maximize(v1, v1) == v1);
    REQUIRE(maximize(v2, v2) == v2);
    REQUIRE(maximize(v3, v3) == v3);

    REQUIRE(maximize(v1, v2) == xe::Vector3(1.3f, -1.6f, 0.0f));
    REQUIRE(maximize(v1, v3) == xe::Vector3(1.0f, 2.0f, 2.0f));
    REQUIRE(maximize(v2, v3) == xe::Vector3(1.3f, 2.0f, 2.0f));

    REQUIRE(maximize(v2, v1) == xe::Vector3(1.3f, -1.6f, 0.0f));
    REQUIRE(maximize(v3, v1) == xe::Vector3(1.0f, 2.0f, 2.0f));
    REQUIRE(maximize(v3, v2) == xe::Vector3(1.3f, 2.0f, 2.0f));

    REQUIRE(maximize(v1, maximize(v2, v3)) == xe::Vector3(1.3f, 2.0f, 2.0f));
    REQUIRE(maximize(v1, maximize(v3, v2)) == xe::Vector3(1.3f, 2.0f, 2.0f));
    REQUIRE(maximize(v2, maximize(v1, v3)) == xe::Vector3(1.3f, 2.0f, 2.0f));
    REQUIRE(maximize(v2, maximize(v3, v1)) == xe::Vector3(1.3f, 2.0f, 2.0f));
    REQUIRE(maximize(v3, maximize(v2, v1)) == xe::Vector3(1.3f, 2.0f, 2.0f));
    REQUIRE(maximize(v3, maximize(v1, v2)) == xe::Vector3(1.3f, 2.0f, 2.0f));
}

TEST_CASE("Vector norm2 function should return a squared vector length", "[math][vector]") {
    const xe::Vector3 v1 = {1.0f, -2.0f, -1.6f};
    const xe::Vector3 v2 = {1.3f, -1.6f, 0.0f};
    const xe::Vector3 v3 = {-0.3f, 2.0f, 2.0f};

    REQUIRE(norm2(v1) == Catch::Approx(1.0f + 4.0f + 1.6f * 1.6f));
    REQUIRE(norm2(v2) == Catch::Approx(1.3f * 1.3f + 1.6f * 1.6f));
    REQUIRE(norm2(v3) == Catch::Approx(0.3f * 0.3f + 4.0f + 4.0f));
}

TEST_CASE("Vector norm function should return the vector length", "[math][vector]") {
    const xe::Vector3 v1 = {0.0f, 0.0f, 0.0f};
    const xe::Vector3 v2 = {4.0f, 2.0f, 4.0f};
    const xe::Vector3 v3 = {1.0f, 1.0f, 1.0f};

    REQUIRE(norm(v1) == Catch::Approx(0.0f));
    REQUIRE(norm(v2) == Catch::Approx(6.0f));
    REQUIRE(norm(v3) == Catch::Approx(std::sqrt(3.0f)));
}

TEST_CASE("Vector xe::vec factory functions should construct a Vector correctly", "[math][vector]") {
    const float x = 1.0f;
    const float y = 2.0f;
    const float z = 3.0f;
    const float w = 4.0f;

    REQUIRE(xe::Vector2(x, y) == xe::vec(x, y));

    REQUIRE(xe::Vector3(x, y, z) == xe::vec(x, y, z));
    REQUIRE(xe::Vector3(x, y, z) == xe::vec(x, xe::vec(y, z)));
    REQUIRE(xe::Vector3(x, y, z) == xe::vec(xe::vec(x, y), z));

    REQUIRE(xe::Vector4(x, y, z, w) == xe::vec(x, y, z, w));
    REQUIRE(xe::Vector4(x, y, z, w) == xe::vec(xe::vec(x, y), z, w));
    REQUIRE(xe::Vector4(x, y, z, w) == xe::vec(x, xe::vec(y, z), w));
    REQUIRE(xe::Vector4(x, y, z, w) == xe::vec(x, y, xe::vec(z, w)));
    REQUIRE(xe::Vector4(x, y, z, w) == xe::vec(xe::vec(x, y), xe::vec(z, w)));
    REQUIRE(xe::Vector4(x, y, z, w) == xe::vec(xe::vec(x, y, z), w));
    REQUIRE(xe::Vector4(x, y, z, w) == xe::vec(x, xe::vec(y, z, w)));
}
