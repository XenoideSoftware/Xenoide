
#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include <glaze/gl.hpp>
#include <glaze/raii.hpp>

#include <xe/graphics/types.h>
#include <xe/graphics/RenderBackend.h>
#include "glcore3-common.h"
#include "glcore3-context.h"

namespace xe {
    tl::expected<UniformLocation, BackendError> resolveUniformLocationGL(RenderDeviceBackendContext *ctx, ShaderHandle handle, const char *name);
    void applyUniformsGL(
        RenderDeviceBackendContext *ctx, ShaderHandle handle, const UniformValueSubmission *values, size_t valueCount, const UniformMatrixSubmission *matrices, size_t matrixCount
    );
    void bindUniformBufferGL(RenderDeviceBackendContext *ctx, uint32_t bindingPoint, BufferHandle handle, size_t offset, size_t size);

    void initializeBackendTableGL(RenderDeviceBackendVTable *vtable);
} // namespace xe
