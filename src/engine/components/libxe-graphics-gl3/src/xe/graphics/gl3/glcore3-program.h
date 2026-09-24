
#pragma once

#include <glaze/gl.hpp>
#include <glaze/raii.hpp>

#include <xe/graphics/types.h>
#include <xe/graphics/RenderBackend.h>
#include "glcore3-common.h"

namespace xe {
    struct ProgramGL {
        glaze::Unique<gl::Program> program;
    };

    tl::expected<ShaderHandle, BackendError> createShaderProgramGL(RenderDeviceBackendContext *ctx, const ShaderProgramDescriptor &desc);
    void destroyShaderProgramGL(RenderDeviceBackendContext *ctx, ShaderHandle handle);

} // namespace xe
