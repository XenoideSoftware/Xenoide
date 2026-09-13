
#pragma once

#include <cstdint>

#include <tl/expected.hpp>

#include <xe/graphics/types.h>

namespace xe {
    /**
     * @brief render device dispatch table
     * Allows to abstract away the underlying graphics API without resorting to polymorphism.
     * All handle-typed fields use the phantom-typed aliases from xe::types, so cross-resource
     * mix-ups (e.g. destroying a TextureHandle via destroyBuffer) are compile-time errors.
     * Factory entry points return tl::expected<..., BackendError> so that failure modes are
     * explicit at the call site and the backend layer can remain exception-free.
     */
    struct RenderDeviceBackendVTable {
        tl::expected<RenderDeviceBackendContext *, BackendError> (*createContext)() = nullptr;
        void (*destroyContext)(RenderDeviceBackendContext *) = nullptr;

        tl::expected<BufferHandle, BackendError> (*createBuffer)(RenderDeviceBackendContext *, const BufferDescriptor &) = nullptr;
        void (*destroyBuffer)(RenderDeviceBackendContext *, BufferHandle) = nullptr;
        void (*readBuffer)(RenderDeviceBackendContext *, BufferHandle, const BufferReadDescriptor &) = nullptr;

        tl::expected<ShaderHandle, BackendError> (*createShaderProgram)(RenderDeviceBackendContext *, const ShaderProgramDescriptor &) = nullptr;
        void (*destroyShaderProgram)(RenderDeviceBackendContext *, ShaderHandle) = nullptr;

        tl::expected<TextureHandle, BackendError> (*createTexture)(RenderDeviceBackendContext *, const TextureDescriptor &) = nullptr;
        void (*destroyTexture)(RenderDeviceBackendContext *, TextureHandle) = nullptr;
        void (*updateTexture)(RenderDeviceBackendContext *, TextureHandle, const TextureUpdateDescriptor &) = nullptr;
        void (*readTexture)(RenderDeviceBackendContext *, TextureHandle, const TextureReadDescriptor &) = nullptr;

        tl::expected<VertexLayoutHandle, BackendError> (*createVertexLayout)(RenderDeviceBackendContext *, const VertexLayoutDescriptor &) = nullptr;
        void (*destroyVertexLayout)(RenderDeviceBackendContext *, VertexLayoutHandle) = nullptr;

        tl::expected<GeometryHandle, BackendError> (*createGeometry)(RenderDeviceBackendContext *, const GeometryDescriptor &) = nullptr;
        void (*destroyGeometry)(RenderDeviceBackendContext *, GeometryHandle) = nullptr;

        tl::expected<PipelineHandle, BackendError> (*createPipeline)(RenderDeviceBackendContext *, const PipelineDescriptor &) = nullptr;
        void (*destroyPipeline)(RenderDeviceBackendContext *, PipelineHandle) = nullptr;

        void (*submitCommand)(RenderDeviceBackendContext *ctx, const CommandBuffer::Command *command, size_t count) = nullptr;

        // TODO: These should be part of the command buffer. Left here for testing purposes
        tl::expected<UniformLocation, BackendError> (*resolveUniformLocation)(RenderDeviceBackendContext *, ShaderHandle, const char *name) = nullptr;
        void (*applyUniforms)(
            RenderDeviceBackendContext *, ShaderHandle, const UniformValueSubmission *values, size_t valueCount, const UniformMatrixSubmission *matrices, size_t matrixCount
        ) = nullptr;
        void (*bindUniformBuffer)(RenderDeviceBackendContext *, uint32_t bindingPoint, BufferHandle, size_t offset, size_t size) = nullptr;

        // TODO: Review if these are necessary or not
        void (*beginFrame)(RenderDeviceBackendContext *) = nullptr;
        void (*endFrame)(RenderDeviceBackendContext *) = nullptr;
        void (*present)(RenderDeviceBackendContext *) = nullptr;
    };
} // namespace xe
