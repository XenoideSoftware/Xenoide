
#ifndef __XE_GRAPHICS_UNIFORM_HPP__
#define __XE_GRAPHICS_UNIFORM_HPP__

#include <string>
#include <vector>
#include <xe/DataType.h>

namespace xe {
    /**
     * @brief Scalar and vector types usable in a shader uniform, backed by TypeEncoding.
     *
     * Each member's value is a valid TypeEncoding constant, so TypeEncoding utilities
     * (getTypeKind, getTypeCols, getTotalSizeInBytes) can be applied via
     * static_cast<TypeEncoding>(v) without a conversion switch.
     */
    enum class UniformVectorType : TypeEncoding {
        Float1 = Scalar_Float32,
        Float2 = Vec2_Float32,
        Float3 = Vec3_Float32,
        Float4 = Vec4_Float32,
        Int1 = Scalar_Int32,
        Int2 = Vec2_Int32,
        Int3 = Vec3_Int32,
        Int4 = Vec4_Int32,
        UInt1 = Scalar_UInt32,
        UInt2 = Vec2_UInt32,
        UInt3 = Vec3_UInt32,
        UInt4 = Vec4_UInt32,
    };

    /**
     * @brief Float32 matrix shapes usable in a shader uniform, backed by TypeEncoding.
     *
     * Only float32 precision is represented here. Each value encodes rows, columns,
     * and Float32 element type, so getTypeCols / getTypeRows work directly.
     */
    enum class UniformMatrixShape : TypeEncoding {
        R2C2 = makeMatrixType(TypeSize::Byte4, TypeKind::Float, 2, 2),
        R2C3 = makeMatrixType(TypeSize::Byte4, TypeKind::Float, 2, 3),
        R2C4 = makeMatrixType(TypeSize::Byte4, TypeKind::Float, 2, 4),
        R3C2 = makeMatrixType(TypeSize::Byte4, TypeKind::Float, 3, 2),
        R3C3 = Mat3_Float32,
        R3C4 = makeMatrixType(TypeSize::Byte4, TypeKind::Float, 3, 4),
        R4C2 = makeMatrixType(TypeSize::Byte4, TypeKind::Float, 4, 2),
        R4C3 = makeMatrixType(TypeSize::Byte4, TypeKind::Float, 4, 3),
        R4C4 = Mat4_Float32,
    };

    /**
     * @brief A scalar/vector variable visible to all the stages in a ShaderProgram.
     */
    struct Uniform {
        std::string name;

        //! Encodes element type (Float/Int/UInt) and vector width (1-4).
        UniformVectorType type = UniformVectorType::Float1;

        int count = 1;
    };

    /**
     * @brief A Float32 matrix variable visible to all the stages in a ShaderProgram.
     */
    struct UniformMatrix {
        std::string name;

        //! Encodes rows, columns, and Float32 element type.
        UniformMatrixShape type = UniformMatrixShape::R4C4;

        int count = 1;
    };
} // namespace xe

#endif
