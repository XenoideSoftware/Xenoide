
#pragma once

#include "GL.h"

#include <cassert>

namespace xe::gl {
    /**
     * Defines a strongly-typed alias for a basic type.
     * This ensures that common errors, like binding a Texture where a Buffer was expected, are caught at build time.
     */
    template <typename Tag, typename T> struct TaggedType {
        typedef T Type;
        typedef Tag TagType;

        T id = {};
    };

    struct CheckedEnum {
        GLenum value = {};
        CheckedEnum() = default;

        CheckedEnum(GLenum value) {
            this->value = value;
        }

        operator GLenum() const {
            assert(value);
            return value;
        }
    };

    struct buffer_tag {};
    struct texture_tag {};
    struct shader_tag {};
    struct program_tag {};
    struct vertex_array_tag {};

    struct Buffer : TaggedType<buffer_tag, GLuint> {
        CheckedEnum target = {};

        Buffer() = default;

        Buffer(GLuint id, GLenum target) {
            this->id = id;
            this->target = target;
        }
    };

    struct Texture : TaggedType<texture_tag, GLuint> {
        CheckedEnum target = {};

        Texture() = default;

        Texture(GLuint id, GLenum target) {
            this->id = id;
            this->target = target;
        }

        operator bool() const {
            return id != 0 && target;
        }
    };

    using Shader = TaggedType<shader_tag, GLuint>;
    using VertexArray = TaggedType<vertex_array_tag, GLuint>;

    struct Program : TaggedType<program_tag, GLuint> {
        Program() = default;

        explicit Program(GLuint id) {
            this->id = id;
        }

        GLint getAttribLocation(const GLchar *name) const {
            assert(name);

            const auto loc = glGetAttribLocation(id, name);
            assert(loc >= 0);

            return loc;
        }

        GLint getUniformLocation(const GLchar *name) const {
            assert(name);

            const auto loc = glGetUniformLocation(id, name);
            assert(loc >= 0);

            return loc;
        }
    };

    /**
     * Like std::span, but it should be used in all places where a typeless memory region along with a size in bytes is expected.
     *
     * Utility methods will be added on as-needed basis
     */
    class MemoryRegion {
    public:
        MemoryRegion() = default;

        MemoryRegion(const void *data, size_t size) : m_data(data), m_size(size) {
        }

        [[nodiscard]]
        size_t size() const {
            return m_size;
        }

        [[nodiscard]]
        const void *data() const {
            return m_data;
        }

    private:
        const void *m_data = nullptr;
        size_t m_size = 0;
    };
} // namespace xe::gl
