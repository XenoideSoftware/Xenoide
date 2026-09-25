
#pragma once

#include <glaze/gl.hpp>
#include <glaze/raii.hpp>

#include <xe/graphics/types.h>
#include <xe/graphics/RenderBackend.h>
#include "glcore3-common.h"

namespace xe {
    struct SamplerGL {
        gl::Sampler sampler;
    };
} // namespace xe
