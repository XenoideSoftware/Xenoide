
#pragma once

#include <vector>
#include <glaze/gl.hpp>
#include <glaze/raii.hpp>

namespace xe {
    struct VertexAttribGL {
        gl::AttribLocation loc;
        GLboolean normalized = GL_FALSE;
        gl::VertexAttribPointerType dataType;
        int dim = 3;

        VertexAttribGL(gl::AttribLocation loc, int dim, gl::VertexAttribPointerType dataType, GLboolean normalized = GL_FALSE) {
            this->loc = loc;
            this->dim = dim;
            this->dataType = dataType;
            this->normalized = normalized;
        }
    };

    struct VertexLayoutGL {
        std::vector<VertexAttribGL> attributes;
        gl::DrawElementsType indexDataType = gl::DrawElementsType::eUnsignedShort;
    };

    tl::expected<VertexLayoutHandle, BackendError> createVertexLayoutGL(RenderDeviceBackendContext *ctx, const VertexLayoutDescriptor &desc);
    void destroyVertexLayoutGL(RenderDeviceBackendContext *ctx, VertexLayoutHandle handle);
} // namespace xe
