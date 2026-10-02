
#pragma once

#include <glaze/gl.hpp>
#include <glaze/raii.hpp>

#include <xe/graphics/types.h>
#include <xe/graphics/RenderBackend.h>

#include "glcore3-common.h"

#include "glcore3-buffer.h"
#include "glcore3-geometry.h"
#include "glcore3-pipeline.h"
#include "glcore3-geometry.h"
#include "glcore3-program.h"
#include "glcore3-sampler.h"
#include "glcore3-texture.h"
#include "glcore3-vertexlayout.h"

namespace xe {
    /**
     * @brief GL-backed implementation of RenderDeviceBackendContext.
     * Holds per-resource pools addressed by Handle::index(). Each pool element is a Slot
     * pairing the RAII-managed GL object with its generation counter; a slot is free when
     * its Slot::obj is empty, and reused in-place by acquireSlot rather than by parallel
     * free lists.
     */
    struct RenderDeviceBackendContextGL : RenderDeviceBackendContext {
        //! Pool of buffer objects. A slot's obj is empty after destroyBufferGL, awaiting reuse.
        std::vector<OptSlot<BufferGL>> buffers;

        //! Pool of linked shader programs. A slot's obj is empty after destroyShaderProgramGL.
        std::vector<OptSlot<ProgramGL>> shaderPrograms;

        //! Pool of texture objects. A slot's obj is empty after destroyTextureGL.
        std::vector<OptSlot<TextureGL>> textures;

        //! Pool of vertex layout descriptors. A slot's obj is empty after destroyVertexLayoutGL, awaiting reuse.
        std::vector<OptSlot<VertexLayoutGL>> layouts;

        //! Pool of geometries (VAO + index metadata). A slot's obj is empty after destroyGeometryGL, awaiting reuse.
        std::vector<OptSlot<GeometryGL>> geometries;

        //! Pool of pipeline objects (baked clear state + shader program reference + uniform block bindings).
        std::vector<OptSlot<PipelineGL>> pipelines;
    };

    inline RenderDeviceBackendContextGL *glctx(RenderDeviceBackendContext *ctx) {
        assert(ctx);
        return static_cast<RenderDeviceBackendContextGL *>(ctx);
    }

    tl::expected<RenderDeviceBackendContext *, BackendError> createContextGL();
    void destroyContextGL(RenderDeviceBackendContext *ctx);
} // namespace xe
