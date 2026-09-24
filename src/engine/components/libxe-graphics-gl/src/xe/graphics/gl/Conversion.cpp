
#include "Conversion.h"

#include <cassert>
#include <xe/DataType.h>
#include <xe/graphics/types.h>
#include <xe/graphics/BufferDescriptor.h>
#include <xe/graphics/Material.h>
#include <xe/graphics/PixelFormat.h>
#include <xe/graphics/Subset.h>

namespace xe {
    static const std::array<GLenum, 12> dataType{
        static_cast<GLenum>(0),
        GL_UNSIGNED_BYTE,
        GL_UNSIGNED_SHORT,
        GL_UNSIGNED_INT,
        static_cast<GLenum>(0),
        GL_BYTE,
        GL_SHORT,
        GL_INT,
        static_cast<GLenum>(0),
        static_cast<GLenum>(0),
        GL_FLOAT,
        GL_DOUBLE
    };

    GLenum convertToGL(const DataType type) {
        assert(type >= DataType::MetaFirst);
        assert(type < DataType::MetaCount);

        const GLenum result = dataType[static_cast<size_t>(type)];

        assert(result != static_cast<GLenum>(0));

        return result;
    }

    static const std::array<GLenum, 3> pixelFormat{static_cast<GLenum>(0), GL_RGB, GL_RGBA};

    GLenum convertToGL(const PixelFormat format) {
        assert(format >= PixelFormat::MetaFirst);
        assert(format < PixelFormat::MetaCount);

        const GLenum result = pixelFormat[static_cast<size_t>(format)];

        assert(result != static_cast<GLenum>(0));

        return result;
    }

    static const std::array<GLenum, 2> bufferType{GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER};

    GLenum convertToGL(const BufferType type) {
        return bufferType[static_cast<size_t>(type)];
    }

    static const std::array<std::array<GLenum, 3>, 3> bufferUsageAccess{
        {{GL_STATIC_COPY, GL_STATIC_READ, GL_STATIC_DRAW}, {GL_DYNAMIC_COPY, GL_DYNAMIC_READ, GL_DYNAMIC_DRAW}, {GL_STREAM_COPY, GL_STREAM_READ, GL_STREAM_DRAW}}
    };

    GLenum convertToGL(const BufferUsage usage, const BufferAccess access) {
        const auto &bufferUsage = bufferUsageAccess[static_cast<size_t>(access)];

        return bufferUsage[static_cast<size_t>(usage)];
    }

    static const std::array<GLenum, 8> primitiveType{GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_TRIANGLE_STRIP, GL_TRIANGLES, GL_TRIANGLE_FAN};

    GLenum convertToGL(const PrimitiveType type) {
        return primitiveType[static_cast<size_t>(type)];
    }

    static const std::array<GLenum, 8> depthFunc{GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS};

    GLenum convertToGL(const DepthFunc func) {
        return depthFunc[static_cast<size_t>(func)];
    }

    static const std::array<GLenum, 3> polygonMode{GL_POINT, GL_LINE, GL_FILL};

    GLenum convertToGL(const PolygonMode mode) {
        return polygonMode[static_cast<size_t>(mode)];
    }

    static const std::array<GLenum, 2> frontFaceOrder{GL_CW, GL_CCW};

    GLenum convertToGL(const FrontFaceOrder order) {
        return frontFaceOrder[static_cast<size_t>(order)];
    }

    static const std::array<GLenum, 14> blendParams{
        GL_ZERO,
        GL_ONE,
        GL_SRC_COLOR,
        GL_ONE_MINUS_SRC_COLOR,
        GL_DST_COLOR,
        GL_ONE_MINUS_DST_COLOR,
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA,
        GL_DST_ALPHA,
        GL_ONE_MINUS_DST_ALPHA,
        GL_CONSTANT_COLOR,
        GL_ONE_MINUS_CONSTANT_COLOR,
        GL_CONSTANT_ALPHA,
        GL_ONE_MINUS_CONSTANT_ALPHA
    };

    GLenum convertToGL(const BlendParam param) {
        return blendParams[static_cast<size_t>(param)];
    }

    static const std::array<GLenum, 2> filters{GL_NEAREST, GL_LINEAR};

    GLenum convertToGL(const TextureFilter filter) {
        return filters[static_cast<size_t>(filter)];
    }

    static const std::array<GLenum, 2> textureWraps{GL_REPEAT, GL_CLAMP_TO_BORDER};

    GLenum convertToGL(const TextureWrap wrap) {
        return textureWraps[static_cast<size_t>(wrap)];
    }

    GLenum convertToGL(const PixelDataType type) {
        const TypeEncoding te = static_cast<TypeEncoding>(type);
        const TypeKind kind = getTypeKind(te);
        const TypeSize size = getElementSize(te);
        if (kind == TypeKind::Float) {
            return size == TypeSize::Byte2 ? GL_HALF_FLOAT : GL_FLOAT;
        }
        if (kind == TypeKind::UInt) {
            if (size == TypeSize::Byte1)
                return GL_UNSIGNED_BYTE;
            if (size == TypeSize::Byte2)
                return GL_UNSIGNED_SHORT;
            return GL_UNSIGNED_INT;
        }
        if (size == TypeSize::Byte1)
            return GL_BYTE;
        if (size == TypeSize::Byte2)
            return GL_SHORT;
        return GL_INT;
    }

    GLboolean convertToGL(const bool value) {
        return static_cast<GLboolean>(value ? GL_TRUE : GL_FALSE);
    }
} // namespace xe
