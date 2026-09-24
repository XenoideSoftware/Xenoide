
#pragma once

#include <glaze/gl.hpp>
#include <glaze/raii.hpp>

#include <xe/graphics/types.h>
#include <xe/graphics/RenderBackend.h>
#include "glcore3-common.h"

namespace xe {
    struct TextureGL {
        glaze::Unique<gl::Texture> texture;
    };

    tl::expected<TextureHandle, BackendError> createTextureGL(RenderDeviceBackendContext *ctx, const TextureDescriptor &desc);
    void destroyTextureGL(RenderDeviceBackendContext *ctx, TextureHandle handle);
    void updateTextureGL(RenderDeviceBackendContext *ctx, TextureHandle handle, const TextureUpdateDescriptor &desc);
    void readTextureGL(RenderDeviceBackendContext *ctx, TextureHandle handle, const TextureReadDescriptor &desc);
} // namespace xe
