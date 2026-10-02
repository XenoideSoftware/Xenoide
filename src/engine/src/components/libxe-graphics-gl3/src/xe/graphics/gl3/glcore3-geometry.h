
#pragma once

#include <glaze/gl.hpp>
#include <glaze/raii.hpp>

#include <xe/graphics/types.h>
#include <xe/graphics/RenderBackend.h>
#include "glcore3-common.h"

namespace xe {
    /**
     * @brief A single geometry: the VAO wiring together buffer bindings, plus the draw-time index
     * data type captured at creation so glDrawElements calls don't need to re-query the layout.
     */
    struct GeometryGL {
        //! RAII-owned VAO that encodes the attribute bindings and the element array buffer binding.
        glaze::Unique<gl::VertexArray> vao;

        //! Index component type (uint16 or uint32), propagated from the source VertexLayoutGL.
        gl::DrawElementsType indexDataType = gl::DrawElementsType::eUnsignedShort;
    };

    tl::expected<GeometryHandle, BackendError> createGeometryGL(RenderDeviceBackendContext *ctx, const GeometryDescriptor &desc);

    void destroyGeometryGL(RenderDeviceBackendContext *ctx, GeometryHandle handle);
} // namespace xe
