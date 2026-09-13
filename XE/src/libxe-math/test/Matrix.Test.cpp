
#include <math.h>

#include <catch2/catch_all.hpp>

#include "xe/math/Common.h"
#include "xe/math/Matrix.h"
#include "xe/math/Vector.h"

TEST_CASE("Matrix default constructor should initialize to zeroes", "[math][matrix]") {
    xe::Matrix4 mat;

    for (int i = 0; i < 16; i++) {
        REQUIRE(mat.data()[i] == 0.0f);
    }
}

TEST_CASE("Matrix pointer constructor should interpret matrix as row-major", "[math][matrix]") {
    float values[] = {1.0f, 2.0f, 3.0f, 4.0, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};

    xe::Matrix4 mat{values};

    REQUIRE(mat.data()[0] == 1.0f);
    REQUIRE(mat.data()[1] == 2.0f);
    REQUIRE(mat.data()[2] == 3.0f);
    REQUIRE(mat.data()[3] == 4.0f);
    REQUIRE(mat.data()[4] == 5.0f);
    REQUIRE(mat.data()[5] == 6.0f);
    REQUIRE(mat.data()[6] == 7.0f);
    REQUIRE(mat.data()[7] == 8.0f);
    REQUIRE(mat.data()[8] == 9.0f);
    REQUIRE(mat.data()[9] == 10.0f);
    REQUIRE(mat.data()[10] == 11.0f);
    REQUIRE(mat.data()[11] == 12.0f);
    REQUIRE(mat.data()[12] == 13.0f);
    REQUIRE(mat.data()[13] == 14.0f);
    REQUIRE(mat.data()[14] == 15.0f);
    REQUIRE(mat.data()[15] == 16.0f);
}

TEST_CASE("Matrix operator[] returns rows as vector references", "[math][matrix]") {
    float values[] = {1.0f, 2.0f, 3.0f, 4.0, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};

    xe::Matrix4 mat{values};

    REQUIRE(mat[0] == xe::Vector4(1.0f, 2.0f, 3.0f, 4.0));
    REQUIRE(mat[1] == xe::Vector4(5.0f, 6.0f, 7.0f, 8.0f));
    REQUIRE(mat[2] == xe::Vector4(9.0f, 10.0f, 11.0f, 12.0f));
    REQUIRE(mat[3] == xe::Vector4(13.0f, 14.0f, 15.0f, 16.0));
}

TEST_CASE("Matrix initializer list constructor interprets items as matrix rows", "[math][matrix]") {
    xe::Matrix3 mat{1.0f, 2.0f, 3.0f, 4.0, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};

    REQUIRE(mat[0][0] == 1.0f);
    REQUIRE(mat[0][1] == 2.0f);
    REQUIRE(mat[0][2] == 3.0f);
    REQUIRE(mat[1][0] == 4.0f);
    REQUIRE(mat[1][1] == 5.0f);
    REQUIRE(mat[1][2] == 6.0f);
    REQUIRE(mat[2][0] == 7.0f);
    REQUIRE(mat[2][1] == 8.0f);
    REQUIRE(mat[2][2] == 9.0f);
}

TEST_CASE("Matrix vector constructor orders elements in row-major order", "[math][matrix]") {
    xe::Matrix4 mat{xe::TVector<float, 16>{1.0f, 2.0f, 3.0f, 4.0, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f}};

    REQUIRE(mat[0] == xe::Vector4(1.0f, 2.0f, 3.0f, 4.0));
    REQUIRE(mat[1] == xe::Vector4(5.0f, 6.0f, 7.0f, 8.0f));
    REQUIRE(mat[2] == xe::Vector4(9.0f, 10.0f, 11.0f, 12.0f));
    REQUIRE(mat[3] == xe::Vector4(13.0f, 14.0f, 15.0f, 16.0));
}

TEST_CASE("Matrix elements should have row-major order", "[math][matrix]") {
    const xe::Matrix4 mat{};
    REQUIRE(mat.order() == xe::MatrixOrder::RowMajor);
}

TEST_CASE("Matrix constructor should setup components correctly for two dimensions", "[math][matrix]") {
    const auto m = xe::Matrix2::rows({xe::Vector2{1.0f, 2.0f}, xe::Vector2{3.0f, 4.0f}});

    REQUIRE(m(0, 0) == 1.0f);
    REQUIRE(m(0, 1) == 2.0f);
    REQUIRE(m(1, 0) == 3.0f);
    REQUIRE(m(1, 1) == 4.0f);
}

TEST_CASE("Matrix constructor should setup components correctly for three dimensions", "[math][matrix]") {
    const xe::Matrix3 m = xe::Matrix3::rows({xe::Vector3{1.0f, 2.0f, 3.0f}, xe::Vector3{4.0f, 5.0f, 6.0f}, xe::Vector3{7.0f, 8.0f, 9.0f}});

    REQUIRE(m(0, 0) == 1.0f);
    REQUIRE(m(0, 1) == 2.0f);
    REQUIRE(m(0, 2) == 3.0f);
    REQUIRE(m(1, 0) == 4.0f);
    REQUIRE(m(1, 1) == 5.0f);
    REQUIRE(m(1, 2) == 6.0f);
    REQUIRE(m(2, 0) == 7.0f);
    REQUIRE(m(2, 1) == 8.0f);
    REQUIRE(m(2, 2) == 9.0f);
}

TEST_CASE("Matrix constructor should setup components correctly for four dimensions", "[math][matrix]") {
    const auto m = xe::Matrix4::rows(
        {xe::Vector4{1.0f, 2.0f, 3.0f, 4.0f}, xe::Vector4{5.0f, 6.0f, 7.0f, 8.0f}, xe::Vector4{9.0f, 10.0f, 11.0f, 12.0f}, xe::Vector4{13.0f, 14.0f, 15.0f, 16.0f}}
    );

    REQUIRE(m(0, 0) == 1.0f);
    REQUIRE(m(0, 1) == 2.0f);
    REQUIRE(m(0, 2) == 3.0f);
    REQUIRE(m(0, 3) == 4.0f);
    REQUIRE(m(1, 0) == 5.0f);
    REQUIRE(m(1, 1) == 6.0f);
    REQUIRE(m(1, 2) == 7.0f);
    REQUIRE(m(1, 3) == 8.0f);
    REQUIRE(m(2, 0) == 9.0f);
    REQUIRE(m(2, 1) == 10.0f);
    REQUIRE(m(2, 2) == 11.0f);
    REQUIRE(m(2, 3) == 12.0f);
    REQUIRE(m(3, 0) == 13.0f);
    REQUIRE(m(3, 1) == 14.0f);
    REQUIRE(m(3, 2) == 15.0f);
    REQUIRE(m(3, 3) == 16.0f);
}

TEST_CASE("Matrix constructor should setup components correctly for four dimensions row vector", "[math][matrix]") {
    const auto m = xe::Matrix4::rows(
        {xe::Vector4{1.0f, 2.0f, 3.0f, 4.0f}, xe::Vector4{5.0f, 6.0f, 7.0f, 8.0f}, xe::Vector4{9.0f, 10.0f, 11.0f, 12.0f}, xe::Vector4{13.0f, 14.0f, 15.0f, 16.0f}}
    );

    REQUIRE(m(0, 0) == 1.0f);
    REQUIRE(m(0, 1) == 2.0f);
    REQUIRE(m(0, 2) == 3.0f);
    REQUIRE(m(0, 3) == 4.0f);
    REQUIRE(m(1, 0) == 5.0f);
    REQUIRE(m(1, 1) == 6.0f);
    REQUIRE(m(1, 2) == 7.0f);
    REQUIRE(m(1, 3) == 8.0f);
    REQUIRE(m(2, 0) == 9.0f);
    REQUIRE(m(2, 1) == 10.0f);
    REQUIRE(m(2, 2) == 11.0f);
    REQUIRE(m(2, 3) == 12.0f);
    REQUIRE(m(3, 0) == 13.0f);
    REQUIRE(m(3, 1) == 14.0f);
    REQUIRE(m(3, 2) == 15.0f);
    REQUIRE(m(3, 3) == 16.0f);
}

TEST_CASE("Matrix determinant should compute matrix determinant correctly", "[math][matrix]") {
    const auto matA = xe::Matrix4::rows(
        {xe::Vector4{1.0f, 2.0f, 1.0f, 0.0f}, xe::Vector4{2.0f, 1.0f, -3.0f, -1.0f}, xe::Vector4{-3.0f, 2.0f, 1.0f, 0.0f}, xe::Vector4{2.0f, -1.0f, 0.0f, -1.0f}}
    );

    // matrix determinant
    REQUIRE(xe::determinant(xe::Matrix4::zero()) == Catch::Approx(0.0f));
    REQUIRE(xe::determinant(xe::mat4Identity()) == Catch::Approx(1.0f));
    REQUIRE(xe::determinant(matA) == Catch::Approx(-32.0f));
}

TEST_CASE("Matrix comparison operators should check matrix components", "[math][matrix]") {
    const xe::Matrix4 m1 = xe::Matrix4::rows(
        {xe::Vector4{1.0f, 2.0f, 3.0f, 4.0f}, xe::Vector4{5.0f, 6.0f, 7.0f, 8.0f}, xe::Vector4{9.0f, 10.0f, 11.0f, 12.0f}, xe::Vector4{13.0f, 14.0f, 15.0f, 16.0f}}
    );

    const xe::Matrix4 m2 = xe::Matrix4::rows(
        {xe::Vector4{16.0f, 15.0f, 14.0f, 13.0f}, xe::Vector4{12.0f, 11.0f, 10.0f, 9.0f}, xe::Vector4{8.0f, 7.0f, 6.0f, 5.0f}, xe::Vector4{4.0f, 3.0f, 2.0f, 1.0f}}
    );

    REQUIRE(m1 == m1);
    REQUIRE(m2 == m2);
    REQUIRE(m1 != m2);
    REQUIRE(m2 != m1);
}

TEST_CASE("Matrix operator+ adds every field one to one", "[math][matrix]") {
    const auto matA = xe::Matrix4::rows(
        {xe::Vector4{1.0f, 2.0f, 1.0f, 0.0f}, xe::Vector4{2.0f, 1.0f, -3.0f, -1.0f}, xe::Vector4{-3.0f, 2.0f, 1.0f, 0.0f}, xe::Vector4{2.0f, -1.0f, 0.0f, -1.0f}}
    );

    const auto matB = xe::Matrix4::rows(
        {xe::Vector4{-3.0f, 1.0f, 5.0f, 1.0f}, xe::Vector4{1.0f, 2.0f, -1.0f, 1.0f}, xe::Vector4{1.0f, 2.0f, 1.0f, -2.0f}, xe::Vector4{1.0f, -1.0f, -3.0f, -1.0f}}
    );

    const auto matAddResult = xe::Matrix4::rows(
        {xe::Vector4{-2.0f, 3.0f, 6.0f, 1.0f}, xe::Vector4{3.0f, 3.0f, -4.0f, 0.0f}, xe::Vector4{-2.0f, 4.0f, 2.0f, -2.0f}, xe::Vector4{3.0f, -2.0f, -3.0f, -2.0f}}
    );

    // addition
    REQUIRE(matA == +matA);
    REQUIRE(matB == +matB);

    REQUIRE(matA + xe::Matrix4::zero() == matA);
    REQUIRE(matB + xe::Matrix4::zero() == matB);

    REQUIRE(matAddResult == matA + matB);
    REQUIRE(matAddResult == matB + matA);

    REQUIRE(matAddResult == ((+matA) += matB));
    REQUIRE(matAddResult == ((+matB) += matA));
}

TEST_CASE("Matrix operator- subtracts every field one to one", "[math][matrix]") {
    const auto matA = xe::Matrix4::rows(
        {xe::Vector4{1.0f, 2.0f, 1.0f, 0.0f}, xe::Vector4{2.0f, 1.0f, -3.0f, -1.0f}, xe::Vector4{-3.0f, 2.0f, 1.0f, 0.0f}, xe::Vector4{2.0f, -1.0f, 0.0f, -1.0f}}
    );

    const auto matNegA = xe::Matrix4::rows(
        {xe::Vector4{-1.0f, -2.0f, -1.0f, -0.0f}, xe::Vector4{-2.0f, -1.0f, 3.0f, 1.0f}, xe::Vector4{3.0f, -2.0f, -1.0f, -0.0f}, xe::Vector4{-2.0f, 1.0f, -0.0f, 1.0f}}
    );

    const auto matB = xe::Matrix4::rows(
        {xe::Vector4{-3.0f, 1.0f, 5.0f, 1.0f}, xe::Vector4{1.0f, 2.0f, -1.0f, 1.0f}, xe::Vector4{1.0f, 2.0f, 1.0f, -2.0f}, xe::Vector4{1.0f, -1.0f, -3.0f, -1.0f}}
    );

    const auto matSubResult = xe::Matrix4::rows(
        {xe::Vector4{4.0f, 1.0f, -4.0f, -1.0f}, xe::Vector4{1.0f, -1.0f, -2.0f, -2.0f}, xe::Vector4{-4.0f, 0.0f, 0.0f, 2.0f}, xe::Vector4{1.0f, 0.0f, 3.0f, 0.0f}}
    );

    // subtraction
    REQUIRE(matNegA == -matA);
    REQUIRE(matA - xe::Matrix4::zero() == matA);
    REQUIRE(matB - xe::Matrix4::zero() == matB);

    REQUIRE(xe::Matrix4::zero() - matA == -matA);
    REQUIRE(xe::Matrix4::zero() - matB == -matB);

    REQUIRE(matA - matB == matSubResult);
    REQUIRE(matB - matA == -matSubResult);

    REQUIRE(matSubResult == ((+matA) -= matB));
    REQUIRE(-matSubResult == ((+matB) -= matA));
}

TEST_CASE("Matrix scalar multiplication multiplies every field with a scalar", "[math][matrix]") {
    const auto matA = xe::Matrix4::rows(
        {xe::Vector4{1.0f, 2.0f, 1.0f, 0.0f}, xe::Vector4{2.0f, 1.0f, -3.0f, -1.0f}, xe::Vector4{-3.0f, 2.0f, 1.0f, 0.0f}, xe::Vector4{2.0f, -1.0f, 0.0f, -1.0f}}
    );

    // scalar multiplication
    REQUIRE(matA * -1.0f == -matA);
    REQUIRE(matA * -1.0f == -1.0f * matA);
    REQUIRE(matA * 1.0f == matA);
    REQUIRE(matA * 1.0f == 1.0f * matA);
}

TEST_CASE("Matrix multiplication performs a dot product between row and columns for each result field", "[math][matrix]") {
    const auto matA = xe::Matrix4::rows(
        {xe::Vector4{1.0f, 2.0f, 1.0f, 0.0f}, xe::Vector4{2.0f, 1.0f, -3.0f, -1.0f}, xe::Vector4{-3.0f, 2.0f, 1.0f, 0.0f}, xe::Vector4{2.0f, -1.0f, 0.0f, -1.0f}}
    );

    const auto matB = xe::Matrix4::rows(
        {xe::Vector4{-3.0f, 1.0f, 5.0f, 1.0f}, xe::Vector4{1.0f, 2.0f, -1.0f, 1.0f}, xe::Vector4{1.0f, 2.0f, 1.0f, -2.0f}, xe::Vector4{1.0f, -1.0f, -3.0f, -1.0f}}
    );

    const xe::Matrix4 matMulResult = xe::Matrix4::rows(
        {xe::Vector4{0.0f, 7.0f, 4.0f, 1.0f}, xe::Vector4{-9.0f, -1.0f, 9.0f, 10.0f}, xe::Vector4{12.0f, 3.0f, -16.0f, -3.0f}, xe::Vector4{-8.0f, 1.0f, 14.0f, 2.0f}}
    );

    // matrix multiplication
    REQUIRE(xe::Matrix4::zero() == xe::Matrix4::zero() * xe::Matrix4::zero());
    REQUIRE(xe::Matrix4::zero() == xe::mat4Identity() * xe::Matrix4::zero());
    REQUIRE(xe::mat4Identity() == xe::mat4Identity() * xe::mat4Identity());

    REQUIRE(matA == matA * xe::mat4Identity());
    REQUIRE(matA == xe::mat4Identity() * matA);

    REQUIRE(matMulResult == matA * matB);
    REQUIRE(matMulResult == ((+matA) *= matB));
}

TEST_CASE("Matrix transpose should swap rows and columns", "[math][matrix]") {
    const auto mi = xe::mat4Identity();
    const auto m0 = xe::Matrix4::zero();

    REQUIRE(mi == transpose(mi));
    REQUIRE(m0 == transpose(m0));

    const auto m = xe::Matrix4::rows(
        {xe::Vector4{1.0f, 2.0f, 3.0f, 4.0f}, xe::Vector4{5.0f, 6.0f, 7.0f, 8.0f}, xe::Vector4{9.0f, 10.0f, 11.0f, 12.0f}, xe::Vector4{13.0f, 14.0f, 15.0f, 16.0f}}
    );

    REQUIRE(transpose(m) == xe::Matrix4::rows(
            {xe::Vector4{1.0f, 5.0f, 9.0f, 13.0f}, xe::Vector4{2.0f, 6.0f, 10.0f, 14.0f}, xe::Vector4{3.0f, 7.0f, 11.0f, 15.0f}, xe::Vector4{4.0f, 8.0f, 12.0f, 16.0f}}
        ));
}

TEST_CASE("Matrix inverse should compute the multiplicative matrix inverse", "[math][matrix]") {
    const auto invMatA = xe::Matrix4::rows(
        {xe::Vector4{0.25000f, 0.000f, -0.25000f, 0.000},
         xe::Vector4{0.28125f, 0.125f, 0.09375f, -0.125},
         xe::Vector4{0.18750f, -0.250f, 0.06250f, 0.250},
         xe::Vector4{0.21875f, -0.125f, -0.59375f, -0.875}}
    );

    const xe::Matrix4 matA = xe::Matrix4::rows(
        {xe::Vector4{1.0f, 2.0f, 1.0f, 0.0f}, xe::Vector4{2.0f, 1.0f, -3.0f, -1.0f}, xe::Vector4{-3.0f, 2.0f, 1.0f, 0.0f}, xe::Vector4{2.0f, -1.0f, 0.0f, -1.0f}}
    );

    auto mi = xe::mat4Identity();
    auto detMatA = -32.0f;

    REQUIRE(mi == inverse(mi));
    REQUIRE(invMatA == inverse(matA, detMatA));
    REQUIRE(invMatA == inverse(matA));
}

TEST_CASE("Matrix getColumn should extract a certain column from the matrix as vector", "[math][matrix]") {
    const xe::Matrix4 m = xe::Matrix4::rows(
        {xe::Vector4{1.0f, 2.0f, 3.0f, 4.0f}, xe::Vector4{5.0f, 6.0f, 7.0f, 8.0f}, xe::Vector4{9.0f, 10.0f, 11.0f, 12.0f}, xe::Vector4{13.0f, 14.0f, 15.0f, 16.0f}}
    );

    REQUIRE(m.getColumn(0) == xe::Vector4(1.0f, 5.0f, 9.0f, 13.0f));
    REQUIRE(m.getColumn(1) == xe::Vector4(2.0f, 6.0f, 10.0f, 14.0f));
    REQUIRE(m.getColumn(2) == xe::Vector4(3.0f, 7.0f, 11.0f, 15.0f));
    REQUIRE(m.getColumn(3) == xe::Vector4(4.0f, 8.0f, 12.0f, 16.0f));
}

TEST_CASE("Matrix setColumn should change correctly a certain column in the matrix", "[math][matrix]") {
    const auto m = xe::Matrix4::rows(
        {xe::Vector4{1.0f, 2.0f, 3.0f, 4.0f}, xe::Vector4{5.0f, 6.0f, 7.0f, 8.0f}, xe::Vector4{9.0f, 10.0f, 11.0f, 12.0f}, xe::Vector4{13.0f, 14.0f, 15.0f, 16.0f}}
    );

    const auto m1 = xe::Matrix4(m).setColumn(0, xe::Vector4{4.0f, 3.0f, 2.0f, 1.0f});
    const auto m1_result = xe::Matrix4::rows(
        {xe::Vector4{4.0f, 2.0f, 3.0f, 4.0f}, xe::Vector4{3.0f, 6.0f, 7.0f, 8.0f}, xe::Vector4{2.0f, 10.0f, 11.0f, 12.0f}, xe::Vector4{1.0f, 14.0f, 15.0f, 16.0f}}
    );

    REQUIRE(m1 == m1_result);
}

TEST_CASE("Matrix getRow should extract a certain row from the matrix as a vector", "[math][matrix]") {
    const xe::Matrix4 m = xe::Matrix4::rows(
        {xe::Vector4{1.0f, 2.0f, 3.0f, 4.0f}, xe::Vector4{5.0f, 6.0f, 7.0f, 8.0f}, xe::Vector4{9.0f, 10.0f, 11.0f, 12.0f}, xe::Vector4{13.0f, 14.0f, 15.0f, 16.0f}}
    );

    REQUIRE(m.getRow(0) == xe::Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    REQUIRE(m.getRow(1) == xe::Vector4(5.0f, 6.0f, 7.0f, 8.0f));
    REQUIRE(m.getRow(2) == xe::Vector4(9.0f, 10.0f, 11.0f, 12.0f));
    REQUIRE(m.getRow(3) == xe::Vector4(13.0f, 14.0f, 15.0f, 16.0f));
}

TEST_CASE("Matrix getSubMatrix should extract a smaller matrix from another", "[math][matrix]") {
    const xe::Matrix4 m = xe::Matrix4::rows(
        {xe::Vector4{1.0f, 2.0f, 3.0f, 4.0f}, xe::Vector4{5.0f, 6.0f, 7.0f, 8.0f}, xe::Vector4{9.0f, 10.0f, 11.0f, 12.0f}, xe::Vector4{13.0f, 14.0f, 15.0f, 16.0f}}
    );

    REQUIRE(m.getSubMatrix(0, 0) == xe::Matrix3::rows({xe::Vector3(6.0f, 7.0f, 8.0f), xe::Vector3(10.0f, 11.0f, 12.0f), xe::Vector3(14.0f, 15.0f, 16.0f)}));

    REQUIRE(m.getSubMatrix(0, 1) == xe::Matrix3::rows({xe::Vector3(5.0f, 7.0f, 8.0f), xe::Vector3(9.0f, 11.0f, 12.0f), xe::Vector3(13.0f, 15.0f, 16.0f)}));

    REQUIRE(m.getSubMatrix(1, 0) == xe::Matrix3::rows({xe::Vector3(2.0f, 3.0f, 4.0f), xe::Vector3(10.0f, 11.0f, 12.0f), xe::Vector3(14.0f, 15.0f, 16.0f)}));

    REQUIRE(m.getSubMatrix(3, 3) == xe::Matrix3::rows({xe::Vector3(1.0f, 2.0f, 3.0f), xe::Vector3(5.0f, 6.0f, 7.0f), xe::Vector3(9.0f, 10.0f, 11.0f)}));

    REQUIRE(m.getSubMatrix(3, 0) == xe::Matrix3::rows({
            xe::Vector3(2.0f, 3.0f, 4.0f),
            xe::Vector3(6.0f, 7.0f, 8.0f),
            xe::Vector3(10.0f, 11.0f, 12.0f),
        }));
}

TEST_CASE("Matrix vector multiply operation should transform the vector by the right", "[math][matrix]") {
    const xe::Matrix3 m = xe::Matrix3::rows({xe::Vector3{1.0f, -1.0f, 1.0f}, xe::Vector3{-1.0f, 1.0f, -1.0f}, xe::Vector3{1.0f, 0.0f, 1.0f}});

    REQUIRE(m * xe::Vector3(0.0f, 0.0f, 0.0f) == xe::Vector3(0.0f, 0.0f, 0.0f));
    REQUIRE(m * xe::Vector3(1.0f, 1.0f, 1.0f) == xe::Vector3(1.0f, -1.0f, 2.0f));
    REQUIRE(m * xe::Vector3(-1.0f, -1.0f, -1.0f) == xe::Vector3(-1.0f, 1.0f, -2.0f));
}

TEST_CASE("Matrix zero static function should create a valid zero matrix", "[math][matrix]") {
    REQUIRE(xe::Matrix4::zero() == xe::Matrix4::rows({xe::Vector4(0.0f, 0.0f, 0.0f, 0.0f), xe::Vector4(0.0f, 0.0f, 0.0f, 0.0f), xe::Vector4(0.0f, 0.0f, 0.0f, 0.0f), xe::Vector4(0.0f, 0.0f, 0.0f, 0.0f)}));

    REQUIRE(xe::Matrix3::zero() == xe::Matrix3::rows({xe::Vector3(0.0f, 0.0f, 0.0f), xe::Vector3(0.0f, 0.0f, 0.0f), xe::Vector3(0.0f, 0.0f, 0.0f)}));

    REQUIRE(xe::Matrix2::zero() == xe::Matrix2::rows({xe::Vector2(0.0f, 0.0f), xe::Vector2(0.0f, 0.0f)}));

    const auto mzero = xe::Matrix4::zero();

    REQUIRE(mzero * xe::Vector4(0.0f, 0.0f, 0.0f, 0.0f) == xe::Vector4(0.0f, 0.0f, 0.0f, 0.0f));
    REQUIRE(mzero * xe::Vector4(1.0f, 0.0f, 0.0f, 0.0f) == xe::Vector4(0.0f, 0.0f, 0.0f, 0.0f));
    REQUIRE(mzero * xe::Vector4(0.0f, 1.0f, 0.0f, 0.0f) == xe::Vector4(0.0f, 0.0f, 0.0f, 0.0f));
    REQUIRE(mzero * xe::Vector4(0.0f, 0.0f, 1.0f, 0.0f) == xe::Vector4(0.0f, 0.0f, 0.0f, 0.0f));
}

TEST_CASE("Matrix identity static function should create a valid identity matrix", "[math][matrix]") {
    REQUIRE(xe::mat4Identity() == xe::Matrix4::rows({xe::Vector4(1.0f, 0.0f, 0.0f, 0.0f), xe::Vector4(0.0f, 1.0f, 0.0f, 0.0f), xe::Vector4(0.0f, 0.0f, 1.0f, 0.0f), xe::Vector4(0.0f, 0.0f, 0.0f, 1.0f)}));

    REQUIRE(xe::mat3Identity() == xe::Matrix3::rows({xe::Vector3(1.0f, 0.0f, 0.0f), xe::Vector3(0.0f, 1.0f, 0.0f), xe::Vector3(0.0f, 0.0f, 1.0f)}));

    REQUIRE(xe::mat2Identity() == xe::Matrix2::rows({xe::Vector2(1.0f, 0.0f), xe::Vector2(0.0f, 1.0f)}));

    const auto mid = xe::mat4Identity();
    REQUIRE(mid * xe::Vector4(0.0f, 0.0f, 0.0f, 0.0f) == xe::Vector4(0.0f, 0.0f, 0.0f, 0.0f));
    REQUIRE(mid * xe::Vector4(1.0f, 0.0f, 0.0f, 0.0f) == xe::Vector4(1.0f, 0.0f, 0.0f, 0.0f));
    REQUIRE(mid * xe::Vector4(0.0f, 1.0f, 0.0f, 0.0f) == xe::Vector4(0.0f, 1.0f, 0.0f, 0.0f));
    REQUIRE(mid * xe::Vector4(0.0f, 0.0f, 1.0f, 0.0f) == xe::Vector4(0.0f, 0.0f, 1.0f, 0.0f));
    REQUIRE(mid * xe::Vector4(1.0f, 2.0f, -3.0f, 4.0f) == xe::Vector4(1.0f, 2.0f, -3.0f, 4.0f));
}

TEST_CASE("Matrix scale static function should create a valid scaling matrix", "[math][matrix]") {
    REQUIRE(xe::mat4Scaling({1.0f, 2.0f, 3.0f, 4.0f}) == xe::Matrix4::rows({xe::Vector4(1.0f, 0.0f, 0.0f, 0.0f), xe::Vector4(0.0f, 2.0f, 0.0f, 0.0f), xe::Vector4(0.0f, 0.0f, 3.0f, 0.0f), xe::Vector4(0.0f, 0.0f, 0.0f, 4.0f)}));

    REQUIRE(xe::mat3Scaling({1.0f, 2.0f, 3.0f}) == xe::Matrix3::rows({xe::Vector3(1.0f, 0.0f, 0.0f), xe::Vector3(0.0f, 2.0f, 0.0f), xe::Vector3(0.0f, 0.0f, 3.0f)}));

    REQUIRE(xe::mat2Scaling({1.0f, 2.0f}) == xe::Matrix2::rows({xe::Vector2(1.0f, 0.0f), xe::Vector2(0.0f, 2.0f)}));

    const auto m_s1 = xe::mat4Scaling({1.0f, 2.0f, 3.0f, 1.0f});
    REQUIRE(m_s1 * xe::Vector4(0.0f, 0.0f, 0.0f, 0.0f) == xe::Vector4(0.0f, 0.0f, 0.0f, 0.0f));
    REQUIRE(m_s1 * xe::Vector4(1.0f, 0.0f, 0.0f, 0.0f) == xe::Vector4(1.0f, 0.0f, 0.0f, 0.0f));
    REQUIRE(m_s1 * xe::Vector4(0.0f, 1.0f, 0.0f, 0.0f) == xe::Vector4(0.0f, 2.0f, 0.0f, 0.0f));
    REQUIRE(m_s1 * xe::Vector4(0.0f, 0.0f, 1.0f, 0.0f) == xe::Vector4(0.0f, 0.0f, 3.0f, 0.0f));
    REQUIRE(m_s1 * xe::Vector4(1.0f, 2.0f, -3.0f, 4.0f) == xe::Vector4(1.0f, 4.0f, -9.0f, 4.0f));
}

TEST_CASE("Matrix createTranslation static function should create a valid translate matrix", "[math][matrix]") {
    const auto m1 = xe::mat4Translation({2.0f, 3.0f, 4.0f});

    REQUIRE(m1.getRow(0) == xe::Vector4(1.0f, 0.0f, 0.0f, 2.0f));
    REQUIRE(m1.getRow(1) == xe::Vector4(0.0f, 1.0f, 0.0f, 3.0f));
    REQUIRE(m1.getRow(2) == xe::Vector4(0.0f, 0.0f, 1.0f, 4.0f));
    REQUIRE(m1.getRow(3) == xe::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
}

TEST_CASE("Matrix rotateX static function should create an X-axis rotation matrix", "[math][matrix]") {
    xe::Matrix4 m;
    float sin = NAN, cos = NAN;

    m = xe::mat4RotationX(0.0f);
    REQUIRE(m.getRow(0) == xe::Vector4(1.0f, 0.0f, 0.0f, 0.0f));
    REQUIRE(m.getRow(1) == xe::Vector4(0.0f, 1.0f, 0.0f, 0.0f));
    REQUIRE(m.getRow(2) == xe::Vector4(0.0f, 0.0f, 1.0f, 0.0f));
    REQUIRE(m.getRow(3) == xe::Vector4(0.0f, 0.0f, 0.0f, 1.0f));

    m = xe::mat4RotationX(xe::pi<float>);
    sin = std::sin(xe::pi<float>);
    cos = std::cos(xe::pi<float>);
    REQUIRE(m.getRow(0) == xe::Vector4(1.0f, 0.0f, 0.0f, 0.0f));
    REQUIRE(m.getRow(1) == xe::Vector4(0.0f, cos, -sin, 0.0f));
    REQUIRE(m.getRow(2) == xe::Vector4(0.0f, sin, cos, 0.0f));
    REQUIRE(m.getRow(3) == xe::Vector4(0.0f, 0.0f, 0.0f, 1.0f));

    m = xe::mat4RotationX(2.0f * xe::pi<float>);
    sin = std::sin(2.0f * xe::pi<float>);
    cos = std::cos(2.0f * xe::pi<float>);
    REQUIRE(m.getRow(0) == xe::Vector4(1.0f, 0.0f, 0.0f, 0.0f));
    REQUIRE(m.getRow(1) == xe::Vector4(0.0f, cos, -sin, 0.0f));
    REQUIRE(m.getRow(2) == xe::Vector4(0.0f, sin, cos, 0.0f));
    REQUIRE(m.getRow(3) == xe::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
}

TEST_CASE("Matrix rotateY static function should create a well constructed rotation matrix", "[math][matrix]") {
    xe::Matrix4 m;
    float sin = NAN, cos = NAN;

    m = xe::mat4RotationY(0.0f);
    REQUIRE(m.getRow(0) == xe::Vector4(1.0f, 0.0f, 0.0f, 0.0f));
    REQUIRE(m.getRow(1) == xe::Vector4(0.0f, 1.0f, 0.0f, 0.0f));
    REQUIRE(m.getRow(2) == xe::Vector4(0.0f, 0.0f, 1.0f, 0.0f));
    REQUIRE(m.getRow(3) == xe::Vector4(0.0f, 0.0f, 0.0f, 1.0f));

    m = xe::mat4RotationY(xe::pi<float>);
    sin = std::sin(xe::pi<float>);
    cos = std::cos(xe::pi<float>);
    REQUIRE(m.getRow(0) == xe::Vector4(cos, 0.0f, sin, 0.0f));
    REQUIRE(m.getRow(1) == xe::Vector4(0.0f, 1.0f, 0.0f, 0.0f));
    REQUIRE(m.getRow(2) == xe::Vector4(-sin, 0.0f, cos, 0.0f));
    REQUIRE(m.getRow(3) == xe::Vector4(0.0f, 0.0f, 0.0f, 1.0f));

    m = xe::mat4RotationY(2.0f * xe::pi<float>);
    sin = std::sin(2.0f * xe::pi<float>);
    cos = std::cos(2.0f * xe::pi<float>);
    REQUIRE(m.getRow(0) == xe::Vector4(cos, 0.0f, sin, 0.0f));
    REQUIRE(m.getRow(1) == xe::Vector4(0.0f, 1.0f, 0.0f, 0.0f));
    REQUIRE(m.getRow(2) == xe::Vector4(-sin, 0.0f, cos, 0.0f));
    REQUIRE(m.getRow(3) == xe::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
}

TEST_CASE("Matrix rotateZ static function should create a well constructed rotation matrix", "[math][matrix]") {
    xe::Matrix4 m;
    float sin = NAN, cos = NAN;

    m = xe::mat4RotationZ(0.0f);
    REQUIRE(m.getRow(0) == xe::Vector4(1.0f, 0.0f, 0.0f, 0.0f));
    REQUIRE(m.getRow(1) == xe::Vector4(0.0f, 1.0f, 0.0f, 0.0f));
    REQUIRE(m.getRow(2) == xe::Vector4(0.0f, 0.0f, 1.0f, 0.0f));
    REQUIRE(m.getRow(3) == xe::Vector4(0.0f, 0.0f, 0.0f, 1.0f));

    m = xe::mat4RotationZ(xe::pi<float>);
    sin = std::sin(xe::pi<float>);
    cos = std::cos(xe::pi<float>);
    REQUIRE(m.getRow(0) == xe::Vector4(cos, -sin, 0.0f, 0.0f));
    REQUIRE(m.getRow(1) == xe::Vector4(sin, cos, 0.0f, 0.0f));
    REQUIRE(m.getRow(2) == xe::Vector4(0.0f, 0.0f, 1.0f, 0.0f));
    REQUIRE(m.getRow(3) == xe::Vector4(0.0f, 0.0f, 0.0f, 1.0f));

    m = xe::mat4RotationZ(2.0f * xe::pi<float>);
    sin = std::sin(2.0f * xe::pi<float>);
    cos = std::cos(2.0f * xe::pi<float>);
    REQUIRE(m.getRow(0) == xe::Vector4(cos, -sin, 0.0f, 0.0f));
    REQUIRE(m.getRow(1) == xe::Vector4(sin, cos, 0.0f, 0.0f));
    REQUIRE(m.getRow(2) == xe::Vector4(0.0f, 0.0f, 1.0f, 0.0f));
    REQUIRE(m.getRow(3) == xe::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
}

TEST_CASE("Matrix rotate with fixed axis should match corresponding rotate XYZ static methods", "[math][matrix]") {
    REQUIRE(xe::mat4Rotation(0.0f, xe::Vector3(0.0f, 1.0f, 0.0f)) == xe::mat4Identity());
    REQUIRE(xe::mat4Rotation(0.0f, xe::Vector3(0.0f, 0.0f, 1.0f)) == xe::mat4Identity());
    REQUIRE(xe::mat4Rotation(0.0f, xe::Vector3(-1.0f, 0.0f, 0.0f)) == xe::mat4Identity());
    REQUIRE(xe::mat4Rotation(0.0f, xe::Vector3(0.0f, -1.0f, 0.0f)) == xe::mat4Identity());
    REQUIRE(xe::mat4Rotation(0.0f, xe::Vector3(0.0f, 0.0f, -1.0f)) == xe::mat4Identity());

    REQUIRE(xe::mat4Rotation(0.0f * xe::pi<float>, xe::Vector3(1.0f, 0.0f, 0.0f)) == xe::mat4RotationX(0.0f * xe::pi<float>));
    REQUIRE(xe::mat4Rotation(0.5f * xe::pi<float>, xe::Vector3(1.0f, 0.0f, 0.0f)) == xe::mat4RotationX(0.5f * xe::pi<float>));
    REQUIRE(xe::mat4Rotation(1.0f * xe::pi<float>, xe::Vector3(1.0f, 0.0f, 0.0f)) == xe::mat4RotationX(1.0f * xe::pi<float>));
    REQUIRE(xe::mat4Rotation(1.5f * xe::pi<float>, xe::Vector3(1.0f, 0.0f, 0.0f)) == xe::mat4RotationX(1.5f * xe::pi<float>));
    REQUIRE(xe::mat4Rotation(2.0f * xe::pi<float>, xe::Vector3(1.0f, 0.0f, 0.0f)) == xe::mat4RotationX(2.0f * xe::pi<float>));

    REQUIRE(xe::mat4Rotation(0.0f * xe::pi<float>, xe::Vector3(0.0f, 1.0f, 0.0f)) == xe::mat4RotationY(0.0f * xe::pi<float>));
    REQUIRE(xe::mat4Rotation(0.5f * xe::pi<float>, xe::Vector3(0.0f, 1.0f, 0.0f)) == xe::mat4RotationY(0.5f * xe::pi<float>));
    REQUIRE(xe::mat4Rotation(1.0f * xe::pi<float>, xe::Vector3(0.0f, 1.0f, 0.0f)) == xe::mat4RotationY(1.0f * xe::pi<float>));
    REQUIRE(xe::mat4Rotation(1.5f * xe::pi<float>, xe::Vector3(0.0f, 1.0f, 0.0f)) == xe::mat4RotationY(1.5f * xe::pi<float>));
    REQUIRE(xe::mat4Rotation(2.0f * xe::pi<float>, xe::Vector3(0.0f, 1.0f, 0.0f)) == xe::mat4RotationY(2.0f * xe::pi<float>));

    REQUIRE(xe::mat4Rotation(0.0f * xe::pi<float>, xe::Vector3(0.0f, 0.0f, 1.0f)) == xe::mat4RotationZ(0.0f * xe::pi<float>));
    REQUIRE(xe::mat4Rotation(0.5f * xe::pi<float>, xe::Vector3(0.0f, 0.0f, 1.0f)) == xe::mat4RotationZ(0.5f * xe::pi<float>));
    REQUIRE(xe::mat4Rotation(1.0f * xe::pi<float>, xe::Vector3(0.0f, 0.0f, 1.0f)) == xe::mat4RotationZ(1.0f * xe::pi<float>));
    REQUIRE(xe::mat4Rotation(1.5f * xe::pi<float>, xe::Vector3(0.0f, 0.0f, 1.0f)) == xe::mat4RotationZ(1.5f * xe::pi<float>));
    REQUIRE(xe::mat4Rotation(2.0f * xe::pi<float>, xe::Vector3(0.0f, 0.0f, 1.0f)) == xe::mat4RotationZ(2.0f * xe::pi<float>));
}

TEST_CASE("Matrix lookAtRH simulating standard camera with center at origin looking at Z axis and Y orientation should generate identity matrix", "[math][matrix]") {
    const auto lookAt = xe::mat4LookAtRH({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f});
    REQUIRE(lookAt == xe::mat4Identity());
}

TEST_CASE("Matrix lookAtRH simulating standard camera with center at origin looking at Z axis and Y orientation should generate pseudo-identity matrix with negative unit axis", "[math][matrix]") {
    const auto lookAt = xe::mat4LookAtRH({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f});

    REQUIRE(lookAt == xe::Matrix4::rows({xe::Vector4(-1.0f, 0.0f, 0.0f, 0.0f), xe::Vector4(0.0f, 1.0f, 0.0f, 0.0f), xe::Vector4(0.0f, 0.0f, -1.0f, 0.0f), xe::Vector4(0.0f, 0.0f, 0.0f, 1.0f)}));
}

TEST_CASE("Matrix lookAtRH simulating standard camera with center at -10 Z looking at Z axis and Y orientation should generate translation matrix", "[math][matrix]") {
    const auto lookAt1 = xe::mat4LookAtRH({0.0f, 0.0f, 10.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f});
    REQUIRE(lookAt1 == xe::mat4Translation(xe::Vector3(0.0f, 0.0f, -10.0f)));

    const auto lookAt2 = xe::mat4LookAtRH({0.0f, 0.0f, 10.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f});
    REQUIRE(lookAt2 == xe::mat4Translation(xe::Vector3(0.0f, 0.0f, -10.0f)));
}

TEST_CASE("Matrix createPerspective should create a perspective transformation matrix", "[math][matrix]") {
    const auto m1 = xe::mat4Perspective(xe::radians(60.0f), (320.0f / 240.0f), 0.1f, 100.0f);
    REQUIRE(m1 == xe::Matrix4::rows(
            {xe::Vector4(1.299038170f, 0.000000000f, 0.000000000f, 0.000000000f),
             xe::Vector4(0.000000000f, 1.73205090f, 0.000000000f, 0.000000000f),
             xe::Vector4(0.000000000f, 0.000000000f, -1.002002001f, -0.200200200f),
             xe::Vector4(0.000000000f, 0.000000000f, -1.000000000f, 0.000000000f)}
        ));

    const auto m2 = xe::mat4Perspective(xe::radians(120.0f), 1.33333f, 0.1f, 100.0f);
    REQUIRE(m2 == xe::Matrix4::rows(
            {xe::Vector4(0.433013767f, 0.000000000f, 0.000000000f, 0.000000000f),
             xe::Vector4(0.000000000f, 0.577350259f, 0.000000000f, 0.000000000f),
             xe::Vector4(0.000000000f, 0.000000000f, -1.002002001f, -0.200200200f),
             xe::Vector4(0.000000000f, 0.000000000f, -1.000000000f, 0.000000000f)}
        ));
}

TEST_CASE("Matrix createOrthographic should create an orthographic transformation matrix", "[math][matrix]") {
    const auto m1 = xe::mat4Ortho({-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f});
    REQUIRE(m1 == xe::Matrix4::rows({
            xe::Vector4(1.0f, 0.0f, 0.0f, 0.0f),
            xe::Vector4(0.0f, 1.0f, 0.0f, 0.0f),
            xe::Vector4(0.0f, 0.0f, -1.0f, 0.0f),
            xe::Vector4(0.0f, 0.0f, 0.0f, 1.0f),
        }));

    const auto m2 = xe::mat4Ortho({-2.0f, -2.0f, -2.0f}, {2.0f, 2.0f, 2.0f});
    REQUIRE(m2 == xe::Matrix4::rows({
            xe::Vector4(0.5f, 0.0f, 0.0f, 0.0f),
            xe::Vector4(0.0f, 0.5f, 0.0f, 0.0f),
            xe::Vector4(0.0f, 0.0f, -0.5f, 0.0f),
            xe::Vector4(0.0f, 0.0f, 0.0f, 1.0f),
        }));

    const auto m3 = xe::mat4Ortho({-0.5f, -1.5f, 0.0f}, {2.5f, 3.5f, 100.0f});
    REQUIRE(m3 == xe::Matrix4::rows(
            {xe::Vector4(0.666666687f, 0.000000000f, 0.000000000f, -0.666666687f),
             xe::Vector4(0.000000000f, 0.400000006f, 0.000000000f, -0.400000006f),
             xe::Vector4(0.000000000f, 0.000000000f, -0.020000000f, -1.000000000f),
             xe::Vector4(0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f)}
        ));

    const auto m4 = xe::mat4Ortho({-0.5f, -1.5f, 100.0f}, {2.5f, 3.5f, -50.0f});
    REQUIRE(m4 == xe::Matrix4::rows(
            {xe::Vector4(0.666666687f, 0.000000000f, 0.000000000f, -0.666666687f),
             xe::Vector4(0.000000000f, 0.400000006f, 0.000000000f, -0.400000006f),
             xe::Vector4(0.000000000f, 0.000000000f, 0.013333334f, 0.333333343f),
             xe::Vector4(0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f)}
        ));
}
