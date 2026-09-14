
// TODO: Refactor the handle assert validation into generic functions that works across different Handle types

#include <glaze/gl.hpp>
#include <glaze/raii.hpp>
#include <xe/math/Vector.h>
#include <xe/graphics/RenderBackend.h>
#include <cassert>
#include <new>
#include <string>
#include <utility>
#include <vector>
#include <array>

#include "glcore3-api.h"
#include "glcore3-context.h"


//! Suppresses unused warnings
//! TODO: Move to a common header
#define XE_UNUSED(var) ((void)(var))


namespace xe {
    tl::expected<UniformLocation, BackendError> resolveUniformLocationGL(RenderDeviceBackendContext *ctx, ShaderHandle handle, const char *name) {
        assert(name != nullptr && "resolveUniformLocationGL: name must not be null");

        auto &shaderPrograms = glctx(ctx)->shaderPrograms;
        const ProgramGL *programPtr = tryProgramExtract(shaderPrograms, handle);
        if (programPtr == nullptr) {
            return makeBackendError(BackendErrorCode::InvalidDescriptor, "resolveUniformLocationGL: shader handle is invalid or references a freed program");
        }

        gl::UniformLocation const loc = gl::getUniformLocation(programPtr->program.get(), name);
        if (!loc.valid()) {
            return makeBackendError(BackendErrorCode::InvalidDescriptor, std::string{"resolveUniformLocationGL: uniform '"} + name + "' is not active in the shader program");
        }

        UniformLocation out;
        out.raw = loc.loc;
        out.programKey = handle.raw;
        return out;
    }

    // -- Fallback stubs --
    static void uniformFvFallback(GLint, GLsizei, const GLfloat *) {
        // TODO: replace assert with a proper logging call once the logging subsystem is in place.
        assert(false && "uniformFvFallback: glUniform*fv function pointer not loaded");
    }
    static void uniformIvFallback(GLint, GLsizei, const GLint *) {
        // TODO: replace assert with a proper logging call once the logging subsystem is in place.
        assert(false && "uniformIvFallback: glUniform*iv function pointer not loaded");
    }
    static void uniformUivFallback(GLint, GLsizei, const GLuint *) {
        // TODO: replace assert with a proper logging call once the logging subsystem is in place.
        assert(false && "uniformUivFallback: glUniform*uiv function pointer not loaded");
    }
    static void uniformMatFvFallback(GLint, GLsizei, GLboolean, const GLfloat *) {
        // TODO: replace assert with a proper logging call once the logging subsystem is in place.
        assert(false && "uniformMatFvFallback: glUniformMatrix*fv function pointer not loaded");
    }

    using UniformFvFn = void (*)(GLint, GLsizei, const GLfloat *);
    using UniformIvFn = void (*)(GLint, GLsizei, const GLint *);
    using UniformUivFn = void (*)(GLint, GLsizei, const GLuint *);
    using UniformMatFvFn = void (*)(GLint, GLsizei, GLboolean, const GLfloat *);

    static UniformFvFn s_uniformFv[4] = {uniformFvFallback, uniformFvFallback, uniformFvFallback, uniformFvFallback};
    static UniformIvFn s_uniformIv[4] = {uniformIvFallback, uniformIvFallback, uniformIvFallback, uniformIvFallback};
    static UniformUivFn s_uniformUiv[4] = {uniformUivFallback, uniformUivFallback, uniformUivFallback, uniformUivFallback};

    static UniformMatFvFn s_uniformMatFv[3][3] = {
        {uniformMatFvFallback, uniformMatFvFallback, uniformMatFvFallback},
        {uniformMatFvFallback, uniformMatFvFallback, uniformMatFvFallback},
        {uniformMatFvFallback, uniformMatFvFallback, uniformMatFvFallback},
    };

    /**
     * @brief Dispatch a scalar/vector uniform upload to the correct glUniform{1,2,3,4}{f,i,ui}v call.
     *
     * The backend assumes the payload in `sub.data` is already in the memory layout the matching
     * glUniform*v entrypoint expects: count * getTypeCols(te) tightly-packed elements of
     * the C type implied by getTypeKind(te) (GLfloat / GLint / GLuint).
     */
    static void applyUniformGL(const UniformValueSubmission &sub) {
        GLint const loc = sub.location.raw;
        GLsizei const count = static_cast<GLsizei>(sub.count);
        const TypeEncoding te = static_cast<TypeEncoding>(sub.type);
        const uint8_t cols = getTypeCols(te);
        assert(cols >= 1 && cols <= 4 && "applyUniformGL: unsupported vector width");

        switch (getTypeKind(te)) {
        case TypeKind::Float:
            s_uniformFv[cols - 1](loc, count, static_cast<const GLfloat *>(sub.data));
            return;
        case TypeKind::Int:
            s_uniformIv[cols - 1](loc, count, static_cast<const GLint *>(sub.data));
            return;
        case TypeKind::UInt:
            s_uniformUiv[cols - 1](loc, count, static_cast<const GLuint *>(sub.data));
            return;
        }
        assert(false && "applyUniformGL: unhandled TypeKind");
    }

    /**
     * @brief Dispatch a matrix uniform upload to the correct glUniformMatrix*fv call.
     *
     * Data is always GLfloat; double-precision matrix uniforms are not part of this API surface.
     */
    static void applyUniformMatrixGL(const UniformMatrixSubmission &sub) {
        GLint const loc = sub.location.raw;
        GLsizei const count = static_cast<GLsizei>(sub.count);
        GLboolean const transpose = sub.transpose ? GL_TRUE : GL_FALSE;
        auto const *data = static_cast<const GLfloat *>(sub.data);
        const TypeEncoding te = static_cast<TypeEncoding>(sub.shape);
        const uint8_t cols = getTypeCols(te);
        const uint8_t rows = getTypeRows(te);
        assert(cols >= 2 && cols <= 4 && rows >= 2 && rows <= 4 && "applyUniformMatrixGL: unsupported matrix shape");
        s_uniformMatFv[cols - 2][rows - 2](loc, count, transpose, data);
    }

    void applyUniformsGL(
        RenderDeviceBackendContext *ctx, ShaderHandle handle, const UniformValueSubmission *values, size_t valueCount, const UniformMatrixSubmission *matrices, size_t matrixCount
    ) {
        auto &shaderPrograms = glctx(ctx)->shaderPrograms;
        const ProgramGL *programPtr = tryProgramExtract(shaderPrograms, handle);
        assert(programPtr != nullptr && "applyUniformsGL: shader handle is invalid or references a freed program");

        gl::useProgram(programPtr->program.get());

        for (size_t i = 0; i < valueCount; ++i) {
            assert(values[i].location.isValid() && "applyUniformsGL: UniformValueSubmission carries an invalid location");
            assert(values[i].location.programKey == handle.raw && "applyUniformsGL: UniformValueSubmission location was resolved against a different program");
            assert(values[i].data != nullptr && "applyUniformsGL: UniformValueSubmission::data must not be null");
            applyUniformGL(values[i]);
        }

        for (size_t i = 0; i < matrixCount; ++i) {
            assert(matrices[i].location.isValid() && "applyUniformsGL: UniformMatrixSubmission carries an invalid location");
            assert(matrices[i].location.programKey == handle.raw && "applyUniformsGL: UniformMatrixSubmission location was resolved against a different program");
            assert(matrices[i].data != nullptr && "applyUniformsGL: UniformMatrixSubmission::data must not be null");
            applyUniformMatrixGL(matrices[i]);
        }
    }

    void bindUniformBufferGL(RenderDeviceBackendContext *ctx, uint32_t bindingPoint, BufferHandle handle, size_t offset, size_t size) {
        assert(handle.subType() == BufferType::Uniform && "bindUniformBufferGL: handle must reference a BufferType::Uniform buffer");

        auto &buffers = glctx(ctx)->buffers;
        uint32_t const index = handle.index();
        assert(index < buffers.size() && "bindUniformBufferGL: handle index out of range");
        auto &slot = buffers[index];
        assert(slot.obj && "bindUniformBufferGL: use of freed handle");
        assert(slot.gen == handle.gen() && "bindUniformBufferGL: stale handle (generation mismatch)");

        if (size == 0) {
            gl::bindBufferBase(gl::BufferTarget::eUniformBuffer, bindingPoint, slot.obj->buffer);
        } else {
            gl::bindBufferRange(gl::BufferTarget::eUniformBuffer, bindingPoint, slot.obj->buffer, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size));
        }
    }

    void clear(xe::Vector4 color, float depth, uint8_t stencil, gl::Flags<gl::ClearBufferMask> flags) {
        gl::clearColor(color.x, color.y, color.z, color.w);
        gl::clearDepth(depth);
        gl::clearStencil(stencil);

        gl::clear(flags);
    }

    void bindPipeline(RenderDeviceBackendContextGL */*glctx*/, const PipelineGL &pipeline) {
        // TODO: Apply list of enable / render state, cull mode, etc
        gl::useProgram(pipeline.shaderProgram);
    }

    gl::TextureUnit getTextureUnit(const uint32_t unit) {
        assert(unit < 32);
        return static_cast<gl::TextureUnit>(static_cast<int>(gl::TextureUnit::eTexture0) + unit);
    }

    void bindTexture(RenderDeviceBackendContextGL */*glctx*/, const uint32_t unit, gl::TextureTarget target, const gl::Texture &texture) {
        gl::activeTexture(getTextureUnit(unit));
        gl::bindTexture(target, texture);
    }

    void setUniform(const UniformValueSubmission *uniform, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            applyUniformGL(uniform[i]);
        }
    }

    void setUniformMatrix(const UniformMatrixSubmission *uniformMatrix, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            applyUniformMatrixGL(uniformMatrix[i]);
        }
    }

    void dispatch(RenderDeviceBackendContextGL *glctx, const CommandClear &c) {
        XE_UNUSED(glctx);
        XE_UNUSED(c);

        // TODO: Add flags mapping to GL
        // clear(c.color, c.depth, c.depth, c.flags);
    }

    void dispatch(RenderDeviceBackendContextGL *glctx, const CommandBindPipeline &c) {
        XE_UNUSED(glctx);
        XE_UNUSED(c);
    }

    void dispatch(RenderDeviceBackendContextGL *glctx, const CommandBindTexture &c) {
        XE_UNUSED(glctx);
        XE_UNUSED(c);
    }

    void dispatch(RenderDeviceBackendContextGL *glctx, const CommandSetUniform &c) {
        XE_UNUSED(glctx);
        XE_UNUSED(c);
    }

    void dispatch(RenderDeviceBackendContextGL *glctx, const CommandSetUniformMatrix &c) {
        XE_UNUSED(glctx);
        XE_UNUSED(c);
    }

    void dispatch(RenderDeviceBackendContextGL *glctx, const CommandDraw &c) {
        XE_UNUSED(glctx);
        XE_UNUSED(c);
    }
    
    void submitCommandGL(RenderDeviceBackendContext *ctx, const CommandBuffer::Command *command, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            switch (command[i].opcode) {
            case CommandOp::Clear:
                dispatch(glctx(ctx), command[i].cmd.clear);
                break;
            case CommandOp::BindPipeline:
                dispatch(glctx(ctx), command[i].cmd.bindPipeline);
                break;
            case CommandOp::BindTexture:
                dispatch(glctx(ctx), command[i].cmd.bindTexture);
                break;
            case CommandOp::SetUniform:
                dispatch(glctx(ctx), command[i].cmd.setUniform);
                break;
            case CommandOp::SetUniformMatrix:
                dispatch(glctx(ctx), command[i].cmd.setUniformMatrix);
                break;
            case CommandOp::Draw:
                dispatch(glctx(ctx), command[i].cmd.draw);
                break;
            }
        }
    }

    void initializeBackendTableGL(RenderDeviceBackendVTable *vtable) {
        vtable->createContext = &createContextGL;
        vtable->destroyContext = &destroyContextGL;
        vtable->createBuffer = &createBufferGL;
        vtable->destroyBuffer = &destroyBufferGL;
        vtable->readBuffer = &readBufferGL;
        vtable->createShaderProgram = &createShaderProgramGL;
        vtable->destroyShaderProgram = &destroyShaderProgramGL;
        vtable->createTexture = &createTextureGL;
        vtable->destroyTexture = &destroyTextureGL;
        vtable->updateTexture = &updateTextureGL;
        vtable->readTexture = &readTextureGL;
        vtable->createVertexLayout = &createVertexLayoutGL;
        vtable->destroyVertexLayout = &destroyVertexLayoutGL;
        vtable->createGeometry = &createGeometryGL;
        vtable->destroyGeometry = &destroyGeometryGL;
        vtable->createPipeline = &createPipelineGL;
        vtable->destroyPipeline = &destroyPipelineGL;
        vtable->resolveUniformLocation = &resolveUniformLocationGL;
        vtable->applyUniforms = &applyUniformsGL;
        vtable->bindUniformBuffer = &bindUniformBufferGL;
        vtable->submitCommand = &submitCommandGL;

        s_uniformFv[0] = glUniform1fv;
        s_uniformFv[1] = glUniform2fv;
        s_uniformFv[2] = glUniform3fv;
        s_uniformFv[3] = glUniform4fv;

        s_uniformIv[0] = glUniform1iv;
        s_uniformIv[1] = glUniform2iv;
        s_uniformIv[2] = glUniform3iv;
        s_uniformIv[3] = glUniform4iv;

        s_uniformUiv[0] = glUniform1uiv;
        s_uniformUiv[1] = glUniform2uiv;
        s_uniformUiv[2] = glUniform3uiv;
        s_uniformUiv[3] = glUniform4uiv;

        s_uniformMatFv[0][0] = glUniformMatrix2fv;   // 2c 2r
        s_uniformMatFv[0][1] = glUniformMatrix2x3fv; // 2c 3r
        s_uniformMatFv[0][2] = glUniformMatrix2x4fv; // 2c 4r
        s_uniformMatFv[1][0] = glUniformMatrix3x2fv; // 3c 2r
        s_uniformMatFv[1][1] = glUniformMatrix3fv;   // 3c 3r
        s_uniformMatFv[1][2] = glUniformMatrix3x4fv; // 3c 4r
        s_uniformMatFv[2][0] = glUniformMatrix4x2fv; // 4c 2r
        s_uniformMatFv[2][1] = glUniformMatrix4x3fv; // 4c 3r
        s_uniformMatFv[2][2] = glUniformMatrix4fv;   // 4c 4r
    }
} // namespace xe
