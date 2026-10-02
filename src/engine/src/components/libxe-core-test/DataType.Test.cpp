#include <catch2/catch_test_macros.hpp>

#include <xe/DataType.h>

using namespace xe;

TEST_CASE("getElementSize returns correct TypeSize for 1-byte scalar types", "[datatype][elementsize]") {
    REQUIRE(getElementSize(Scalar_UInt8) == TypeSize::Byte1);
    REQUIRE(getElementSize(Scalar_Int8) == TypeSize::Byte1);
}

TEST_CASE("getElementSize returns correct TypeSize for 2-byte scalar types", "[datatype][elementsize]") {
    REQUIRE(getElementSize(Scalar_UInt16) == TypeSize::Byte2);
    REQUIRE(getElementSize(Scalar_Int16) == TypeSize::Byte2);
    REQUIRE(getElementSize(Scalar_Float16) == TypeSize::Byte2);
}

TEST_CASE("getElementSize returns correct TypeSize for 4-byte scalar types", "[datatype][elementsize]") {
    REQUIRE(getElementSize(Scalar_UInt32) == TypeSize::Byte4);
    REQUIRE(getElementSize(Scalar_Int32) == TypeSize::Byte4);
    REQUIRE(getElementSize(Scalar_Float32) == TypeSize::Byte4);
}

TEST_CASE("getElementSize returns correct TypeSize for 8-byte scalar types", "[datatype][elementsize]") {
    REQUIRE(getElementSize(Scalar_UInt64) == TypeSize::Byte8);
    REQUIRE(getElementSize(Scalar_Int64) == TypeSize::Byte8);
    REQUIRE(getElementSize(Scalar_Float64) == TypeSize::Byte8);
}

TEST_CASE("getElementSize is consistent across scalars vectors and matrices of same base type", "[datatype][elementsize]") {
    REQUIRE(getElementSize(Vec2_Float32) == getElementSize(Scalar_Float32));
    REQUIRE(getElementSize(Vec3_Float32) == getElementSize(Scalar_Float32));
    REQUIRE(getElementSize(Vec4_Float32) == getElementSize(Scalar_Float32));
    REQUIRE(getElementSize(Mat4_Float32) == getElementSize(Scalar_Float32));

    REQUIRE(getElementSize(Vec4_UInt8) == getElementSize(Scalar_UInt8));
    REQUIRE(getElementSize(Vec4_Int16) == getElementSize(Scalar_Int16));
    REQUIRE(getElementSize(Mat4_Float64) == getElementSize(Scalar_Float64));
}

TEST_CASE("toBytes converts TypeSize to correct byte count", "[datatype][tobytes]") {
    REQUIRE(toBytes(TypeSize::Byte1) == 1);
    REQUIRE(toBytes(TypeSize::Byte2) == 2);
    REQUIRE(toBytes(TypeSize::Byte4) == 4);
    REQUIRE(toBytes(TypeSize::Byte8) == 8);
}

TEST_CASE("toBytes round-trips through getElementSize", "[datatype][tobytes]") {
    REQUIRE(toBytes(getElementSize(Scalar_UInt8)) == 1);
    REQUIRE(toBytes(getElementSize(Scalar_Float16)) == 2);
    REQUIRE(toBytes(getElementSize(Scalar_Float32)) == 4);
    REQUIRE(toBytes(getElementSize(Scalar_Float64)) == 8);
}

TEST_CASE("getTotalSizeInBytes returns element size for scalar types", "[datatype][totalsize]") {
    REQUIRE(getTotalSizeInBytes(Scalar_UInt8) == 1);
    REQUIRE(getTotalSizeInBytes(Scalar_Float16) == 2);
    REQUIRE(getTotalSizeInBytes(Scalar_Float32) == 4);
    REQUIRE(getTotalSizeInBytes(Scalar_Float64) == 8);
}

TEST_CASE("getTotalSizeInBytes returns element_size times cols for vector types", "[datatype][totalsize]") {
    REQUIRE(getTotalSizeInBytes(Vec2_Float32) == 8);
    REQUIRE(getTotalSizeInBytes(Vec3_Float32) == 12);
    REQUIRE(getTotalSizeInBytes(Vec4_Float32) == 16);
    REQUIRE(getTotalSizeInBytes(Vec4_UInt8) == 4);
    REQUIRE(getTotalSizeInBytes(Vec4_Float64) == 32);
}

TEST_CASE("getTotalSizeInBytes returns element_size times cols times rows for matrix types", "[datatype][totalsize]") {
    REQUIRE(getTotalSizeInBytes(Mat2_Float32) == 16);
    REQUIRE(getTotalSizeInBytes(Mat3_Float32) == 36);
    REQUIRE(getTotalSizeInBytes(Mat4_Float32) == 64);
    REQUIRE(getTotalSizeInBytes(Mat4_Float64) == 128);
    REQUIRE(getTotalSizeInBytes(Mat3x4_Float32) == 48);
    REQUIRE(getTotalSizeInBytes(Mat4x3_Float32) == 48);
}

TEST_CASE("kind returns UInt for unsigned integer types", "[datatype][kind]") {
    REQUIRE(getTypeKind(Scalar_UInt8) == TypeKind::UInt);
    REQUIRE(getTypeKind(Scalar_UInt16) == TypeKind::UInt);
    REQUIRE(getTypeKind(Scalar_UInt32) == TypeKind::UInt);
    REQUIRE(getTypeKind(Scalar_UInt64) == TypeKind::UInt);
}

TEST_CASE("kind returns Int for signed integer types", "[datatype][kind]") {
    REQUIRE(getTypeKind(Scalar_Int8) == TypeKind::Int);
    REQUIRE(getTypeKind(Scalar_Int16) == TypeKind::Int);
    REQUIRE(getTypeKind(Scalar_Int32) == TypeKind::Int);
    REQUIRE(getTypeKind(Scalar_Int64) == TypeKind::Int);
}

TEST_CASE("kind returns Float for floating-point types", "[datatype][kind]") {
    REQUIRE(getTypeKind(Scalar_Float16) == TypeKind::Float);
    REQUIRE(getTypeKind(Scalar_Float32) == TypeKind::Float);
    REQUIRE(getTypeKind(Scalar_Float64) == TypeKind::Float);
}

TEST_CASE("kind is preserved in vector and matrix variants", "[datatype][kind]") {
    REQUIRE(getTypeKind(Vec4_Float32) == TypeKind::Float);
    REQUIRE(getTypeKind(Vec4_Int32) == TypeKind::Int);
    REQUIRE(getTypeKind(Vec4_UInt8) == TypeKind::UInt);
    REQUIRE(getTypeKind(Mat4_Float32) == TypeKind::Float);
    REQUIRE(getTypeKind(Mat4_Float64) == TypeKind::Float);
    REQUIRE(getTypeKind(Mat3x4_Float32) == TypeKind::Float);
}

TEST_CASE("getTypeCols returns 1 for all scalar types", "[datatype][cols]") {
    REQUIRE(getTypeCols(Scalar_UInt8) == 1);
    REQUIRE(getTypeCols(Scalar_Int32) == 1);
    REQUIRE(getTypeCols(Scalar_Float32) == 1);
    REQUIRE(getTypeCols(Scalar_Float64) == 1);
}

TEST_CASE("getTypeCols returns correct dimension for vector types", "[datatype][cols]") {
    REQUIRE(getTypeCols(Vec2_Float32) == 2);
    REQUIRE(getTypeCols(Vec3_Float32) == 3);
    REQUIRE(getTypeCols(Vec4_Float32) == 4);
    REQUIRE(getTypeCols(Vec2_UInt8) == 2);
    REQUIRE(getTypeCols(Vec3_Int16) == 3);
    REQUIRE(getTypeCols(Vec4_UInt8) == 4);
}

TEST_CASE("getTypeCols returns correct column count for matrix types", "[datatype][cols]") {
    REQUIRE(getTypeCols(Mat2_Float32) == 2);
    REQUIRE(getTypeCols(Mat3_Float32) == 3);
    REQUIRE(getTypeCols(Mat4_Float32) == 4);
    REQUIRE(getTypeCols(Mat2_Float64) == 2);
    REQUIRE(getTypeCols(Mat4_Float64) == 4);
    REQUIRE(getTypeCols(Mat3x4_Float32) == 3);
    REQUIRE(getTypeCols(Mat4x3_Float32) == 4);
}

TEST_CASE("getTypeRows returns 0 for all scalar types", "[datatype][rows]") {
    REQUIRE(getTypeRows(Scalar_UInt8) == 0);
    REQUIRE(getTypeRows(Scalar_Int32) == 0);
    REQUIRE(getTypeRows(Scalar_Float32) == 0);
    REQUIRE(getTypeRows(Scalar_Float64) == 0);
}

TEST_CASE("getTypeRows returns 0 for all vector types", "[datatype][rows]") {
    REQUIRE(getTypeRows(Vec2_Float32) == 0);
    REQUIRE(getTypeRows(Vec3_Float32) == 0);
    REQUIRE(getTypeRows(Vec4_Float32) == 0);
    REQUIRE(getTypeRows(Vec4_UInt8) == 0);
    REQUIRE(getTypeRows(Vec4_Int32) == 0);
    REQUIRE(getTypeRows(Vec4_Float64) == 0);
}

TEST_CASE("getTypeRows returns correct row count for matrix types", "[datatype][rows]") {
    REQUIRE(getTypeRows(Mat2_Float32) == 2);
    REQUIRE(getTypeRows(Mat3_Float32) == 3);
    REQUIRE(getTypeRows(Mat4_Float32) == 4);
    REQUIRE(getTypeRows(Mat2_Float64) == 2);
    REQUIRE(getTypeRows(Mat4_Float64) == 4);
    REQUIRE(getTypeRows(Mat3x4_Float32) == 4);
    REQUIRE(getTypeRows(Mat4x3_Float32) == 3);
}

TEST_CASE("all TypeEncoding functions are usable in constexpr context", "[datatype][constexpr]") {
    static_assert(getElementSize(Scalar_Float32) == TypeSize::Byte4);
    static_assert(getElementSize(Scalar_UInt8) == TypeSize::Byte1);

    static_assert(toBytes(TypeSize::Byte1) == 1);
    static_assert(toBytes(TypeSize::Byte4) == 4);
    static_assert(toBytes(getElementSize(Scalar_Float32)) == 4);

    static_assert(getTotalSizeInBytes(Scalar_Float32) == 4);
    static_assert(getTotalSizeInBytes(Vec4_Float32) == 16);
    static_assert(getTotalSizeInBytes(Mat4_Float32) == 64);
    static_assert(getTotalSizeInBytes(Mat3x4_Float32) == 48);

    static_assert(getTypeKind(Scalar_Float32) == TypeKind::Float);
    static_assert(getTypeKind(Scalar_Int32) == TypeKind::Int);
    static_assert(getTypeKind(Scalar_UInt8) == TypeKind::UInt);

    static_assert(getTypeCols(Scalar_Float32) == 1);
    static_assert(getTypeCols(Vec4_Float32) == 4);
    static_assert(getTypeCols(Mat4_Float32) == 4);
    static_assert(getTypeCols(Mat3x4_Float32) == 3);

    static_assert(getTypeRows(Scalar_Float32) == 0);
    static_assert(getTypeRows(Vec4_Float32) == 0);
    static_assert(getTypeRows(Mat4_Float32) == 4);
    static_assert(getTypeRows(Mat3x4_Float32) == 4);
    static_assert(getTypeRows(Mat4x3_Float32) == 3);

    static_assert(getTypeShape(Scalar_Float32) == TypeShape::Scalar);
    static_assert(getTypeShape(Vec4_Float32) == TypeShape::Vector);
    static_assert(getTypeShape(Mat4_Float32) == TypeShape::Matrix);

    REQUIRE(true); // suppress empty-test-case warning
}

TEST_CASE("getTypeShape returns Scalar for all scalar types", "[datatype][shape]") {
    REQUIRE(getTypeShape(Scalar_UInt8) == TypeShape::Scalar);
    REQUIRE(getTypeShape(Scalar_Int32) == TypeShape::Scalar);
    REQUIRE(getTypeShape(Scalar_Float32) == TypeShape::Scalar);
    REQUIRE(getTypeShape(Scalar_Float64) == TypeShape::Scalar);
}

TEST_CASE("getTypeShape returns Vector for all vector types", "[datatype][shape]") {
    REQUIRE(getTypeShape(Vec2_Float32) == TypeShape::Vector);
    REQUIRE(getTypeShape(Vec3_Float32) == TypeShape::Vector);
    REQUIRE(getTypeShape(Vec4_Float32) == TypeShape::Vector);
    REQUIRE(getTypeShape(Vec4_UInt8) == TypeShape::Vector);
    REQUIRE(getTypeShape(Vec4_Int32) == TypeShape::Vector);
    REQUIRE(getTypeShape(Vec2_Float64) == TypeShape::Vector);
}

TEST_CASE("getTypeShape returns Matrix for all matrix types", "[datatype][shape]") {
    REQUIRE(getTypeShape(Mat2_Float32) == TypeShape::Matrix);
    REQUIRE(getTypeShape(Mat3_Float32) == TypeShape::Matrix);
    REQUIRE(getTypeShape(Mat4_Float32) == TypeShape::Matrix);
    REQUIRE(getTypeShape(Mat4_Float64) == TypeShape::Matrix);
    REQUIRE(getTypeShape(Mat3x4_Float32) == TypeShape::Matrix);
    REQUIRE(getTypeShape(Mat4x3_Float32) == TypeShape::Matrix);
}
