
#pragma once

#include <glaze/gl.h>
#include <string>
#include <vector>

#include "Types.h"
#include "xe/math/Matrix.h"
#include "xe/math/Vector.h"

#include <optional>

#include <bpstd/span.hpp>

struct RendererInfo {
    std::string vendor;
    std::string renderer;
    std::string version;
    std::string shadingLanguageVersion;
};

namespace xe::gl {
    enum class Primitive {
        Points,
        Lines,
        LineStrip,
        Triangles,
        TriangleStrip,
        TriangleFan,
    };

    struct CapabilityStatus {
        GLenum capability = {};
        GLboolean enabled = GL_FALSE;
    };

    enum class UniformType { Float, Int, UnsignedInt };

    template <typename BasicType> struct MetaUniformTypeMapper {};

    template <> struct MetaUniformTypeMapper<float> {
        static UniformType map() {
            return UniformType::Float;
        }
    };

    template <> struct MetaUniformTypeMapper<int> {
        static UniformType map() {
            return UniformType::Int;
        }
    };

    template <> struct MetaUniformTypeMapper<unsigned int> {
        static UniformType map() {
            return UniformType::UnsignedInt;
        }
    };

    enum class UniformDim { _1, _2, _3, _4 };

    struct Uniform {
        GLint location = 0;
        UniformType type = UniformType::Float;
        UniformDim dim = UniformDim::_1;
        GLsizei count = 0;

        const void *data = nullptr;
    };

    enum class UniformMatrixDim {
        _2x2,
        _2x3,
        _2x4,
        _3x2,
        _3x3,
        _3x4,
        _4x2,
        _4x3,
        _4x4,
    };

    template <int rows, int cols> constexpr UniformMatrixDim mapUniformMatrixDim() {
        static_assert(rows >= 2 && rows <= 4);
        static_assert(cols >= 2 && cols <= 4);

        if constexpr (rows == 2) {
            if constexpr (cols == 2) {
                return UniformMatrixDim::_2x2;
            }
            if constexpr (cols == 3) {
                return UniformMatrixDim::_2x3;
            }
            if constexpr (cols == 4) {
                return UniformMatrixDim::_2x4;
            }
        }

        if constexpr (rows == 3) {
            if constexpr (cols == 2) {
                return UniformMatrixDim::_3x2;
            }
            if constexpr (cols == 3) {
                return UniformMatrixDim::_3x3;
            }
            if constexpr (cols == 4) {
                return UniformMatrixDim::_3x4;
            }
        }

        if constexpr (rows == 4) {
            if constexpr (cols == 2) {
                return UniformMatrixDim::_4x2;
            }
            if constexpr (cols == 3) {
                return UniformMatrixDim::_4x3;
            }
            if constexpr (cols == 4) {
                return UniformMatrixDim::_4x4;
            }
        }
    }

    enum class UniformMatrixType { Float, Double };

    template <typename BasicType> struct MetaUniformMatrixTypeMapper {};

    template <> struct MetaUniformMatrixTypeMapper<float> {
        static UniformMatrixType map() {
            return UniformMatrixType::Float;
        }
    };

    template <> struct MetaUniformMatrixTypeMapper<double> {
        static UniformMatrixType map() {
            return UniformMatrixType::Double;
        }
    };

    struct UniformMatrix {
        GLint location = 0;
        UniformMatrixType type = UniformMatrixType::Float;
        UniformMatrixDim dim = UniformMatrixDim::_4x4;
        GLboolean transpose = GL_FALSE;
        GLsizei count = 0;
        const void *data = nullptr;
    };

    template <typename Type, int Rows, int Cols> UniformMatrix makeUniform(GLint location, const xe::TMatrix<Type, Rows, Cols> &matrix, const bool transpose = false) {
        UniformMatrix uniform;

        uniform.location = location;
        uniform.type = MetaUniformMatrixTypeMapper<Type>::map();
        uniform.dim = mapUniformMatrixDim<Rows, Cols>();
        uniform.transpose = transpose == GL_TRUE;
        uniform.count = 1;
        uniform.data = matrix.data();

        return uniform;
    }

    template <typename Type> Uniform makeUniform(GLint location, Type &value) {
        Uniform uniform;

        uniform.location = location;
        uniform.type = MetaUniformTypeMapper<typename std::remove_const<Type>::type>::map();
        uniform.dim = UniformDim::_1;
        uniform.count = 1;
        uniform.data = &value;

        return uniform;
    }

    enum class AttributeDim { _1, _2, _3, _4 };

    enum class AttributeType { Float, Int, UnsignedInt, UnsignedByte, UnsignedShort };

    struct Attribute {
        GLint index = 0;
        AttributeDim size = AttributeDim::_3;
        AttributeType type = AttributeType::Float;
        GLboolean normalized = GL_FALSE;
        GLsizei stride = 0;
        Buffer buffer = {};
        GLuint offset = 0;
        const void *data = nullptr;
    };

    struct VertexArrayPrimitive {
        GLint start = 0;
        GLsizei count = 0;

        //! attributes to set prior making the rendering call
        bpstd::span<const Attribute> attribs;

        VertexArrayPrimitive(GLint start, GLsizei count) {
            this->start = start;
            this->count = count;
        }

        VertexArrayPrimitive(GLint start, GLsizei count, const bpstd::span<const Attribute> &attribs) {
            this->start = start;
            this->count = count;
            this->attribs = attribs;
        }
    };

    struct VertexArrayMultiDraw {
        GLint *start = nullptr;
        GLsizei *count = nullptr;
        GLsizei drawCount = 0;
    };

    struct TextureParameter {
        GLenum param = {};
        GLint value = {};
    };

    struct TextureLayer {
        Texture texture = {};
        bpstd::span<const TextureParameter> parameters;
    };

    enum class ClearFlags { Color = 0x01, Depth = 0x02, Stencil = 0x04 };

    struct ClearParams {
        std::optional<xe::Vector4> color;
        std::optional<float> depth;
        std::optional<int> stencil;
    };

    enum class DataType { Double, Float, Int, UnsignedInt, UnsignedByte, UnsignedShort, Short, Byte };

    enum class PixelFormat {
        Luminance,
        LuminanceAlpha,
        RGB,
        RGBA,
    };

    struct ClientTextureImage1D {
        int size = 0;
        GLenum format = GL_RGBA;
        GLenum type = GL_UNSIGNED_BYTE;
        const void *pixels = nullptr;
    };

    struct ClientTextureImage2D {
        xe::Vector2i size = {0, 0};
        GLenum format = GL_RGBA;
        GLenum type = GL_UNSIGNED_BYTE;
        const void *pixels = nullptr;
    };

    struct ClientTextureImage3D {
        xe::Vector3i size = {0, 0, 0};
        GLenum format = GL_RGBA;
        GLenum type = GL_UNSIGNED_BYTE;
        const void *pixels = nullptr;
    };

    class Context;

    // -- Low Priority --
    // TODO: Integrate the Logging facility (maybe use lib-xecore?)
    // TODO: Define more specifically how to manage the errors
    // TODO: Add 2d texture support
    // TODO: Add cubemap texture support
    // TODO: Define Mixin classes to support both manual and automatic resource management

    enum CreateTextureFlags {
        None = 0x0,
        GenerateMipMaps = 0x1,
    };

    struct CreateTextureOptions {
        CreateTextureFlags flags = None;
        bpstd::span<const TextureParameter> parameters;
    };

    /**
     * @brief Wrapper to OpenGL 3+ APIs
     */
    class RendererGL {
    public:
        using GLproc = void (*)();
        using GetProcAddress = GLproc (*)(const char *);

        static std::unique_ptr<RendererGL> create(GetProcAddress getProcAddress);

        static std::unique_ptr<RendererGL> create();

    private:
        RendererGL(); // caca i pipi

    public:
        [[nodiscard]]
        Shader createShader(GLenum type, const char *source) const;

        [[nodiscard]]
        Program createProgram(const bpstd::span<Shader> &shaders) const;

        [[nodiscard]]
        Buffer createBuffer(GLenum target, GLenum usage, const MemoryRegion &memory) const;

        [[nodiscard]]
        VertexArray createVertexArray(const bpstd::span<const Attribute> &attributes, Buffer elementArrayBuffer) const;

        [[nodiscard]]
        RendererInfo getInfo() const;

        [[nodiscard]]
        Texture createTexture(GLenum target, GLenum internalFormat, const ClientTextureImage1D &image, const CreateTextureOptions &options = {}) const;

        [[nodiscard]]
        Texture createTexture(GLenum target, GLenum internalFormat, const ClientTextureImage2D &image, const CreateTextureOptions &options = {}) const;

        [[nodiscard]]
        Texture createTexture(GLenum target, GLenum internalFormat, const ClientTextureImage3D &image, const CreateTextureOptions &options = {}) const;

        void bindRenderState(const bpstd::span<const CapabilityStatus> &capabilities) const;

        void bindRenderState(const bpstd::span<const TextureLayer> &layers) const;

        void bindRenderState(GLenum textureTarget, const bpstd::span<const TextureParameter> &parameters) const;

        void bindRenderState(const bpstd::span<const Attribute> &attribs) const;

        void bindRenderState(const bpstd::span<const Uniform> &uniforms) const;

        void bindRenderState(const bpstd::span<const UniformMatrix> &uniforms) const;

        void draw(VertexArray vertexArray, GLenum primitiveType, const VertexArrayMultiDraw &multiDraw) const;

        void draw(VertexArray vertexArray, GLenum primitiveType, const bpstd::span<const VertexArrayPrimitive> &primitives) const;

        // Draws an indexed geometry
        void draw(VertexArray vertexArray, GLenum primitiveType, const bpstd::span<const VertexArrayPrimitive> &primitives, GLenum dataType) const;

        void clear(GLenum flags, std::optional<xe::Vector4> color, std::optional<float> depth, std::optional<int> stencil) const;

        void flush() const;

        void viewport(const xe::Vector2i &pos, const xe::Vector2i &size) const;

        void useProgram(const Program &program) const;

    private:
        using PFNGLVERTEXATTRIBMXFVPROC = void (*)(GLuint index, const GLfloat *v);
        using PFNGLVERTEXATTRIBMXIVPROC = void (*)(GLuint index, const GLint *v);

        using PFNGLUNIFORMXFVPROC = void (*)(GLint location, GLsizei count, const GLfloat *value);
        using PFNGLUNIFORMXIVPROC = void (*)(GLint location, GLsizei count, const GLint *value);
        using PFNGLUNIFORMXUIVPROC = void (*)(GLint location, GLsizei count, const GLuint *value);

        using PFNGLUNIFORMMATRIXXFVPROC = void (*)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
        using PFNGLUNIFORMMATRIXXDVPROC = void (*)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);

        using PFNGLXABLEPROC = void (*)(GLenum pname);

        PFNGLVERTEXATTRIBMXFVPROC glVertexAttribXfv[4]{};
        PFNGLVERTEXATTRIBMXIVPROC glVertexAttribXiv[4]{};

        PFNGLUNIFORMXFVPROC glUniformXfv[4]{};
        PFNGLUNIFORMXIVPROC glUniformXiv[4]{};
        PFNGLUNIFORMXUIVPROC glUniformXuiv[4]{};

        PFNGLUNIFORMMATRIXXFVPROC glUniformMatrixXfv[9]{};
        PFNGLUNIFORMMATRIXXDVPROC glUniformMatrixXdv[9]{};

        PFNGLXABLEPROC glXable[2]{};
    };
} // namespace xe::gl
