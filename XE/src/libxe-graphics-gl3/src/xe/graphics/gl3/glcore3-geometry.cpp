
#include "glcore3-context.h"

namespace xe {
    tl::expected<GeometryHandle, BackendError> createGeometryGL(RenderDeviceBackendContext *ctx, const GeometryDescriptor &desc) {
        auto &geometries = glctx(ctx)->geometries;
        auto &buffers = glctx(ctx)->buffers;
        auto &layouts = glctx(ctx)->layouts;

        if (!poolHasCapacity(geometries)) {
            return makeBackendError(BackendErrorCode::HandlePoolExhausted, "geometry pool exhausted (16-bit index field)");
        }

        const VertexLayoutGL *layout = tryObjectExtract<VertexLayoutGL>(layouts, desc.layoutHandle);
        if (layout == nullptr) {
            return makeBackendError(BackendErrorCode::InvalidDescriptor, "createGeometryGL: layoutHandle is invalid or references a freed layout");
        }

        // check buffer attribs
        for (const GeometryBufferAttrib &bufferAttrib : desc.bufferAttribs) {
            if (bufferAttrib.attribIndex >= layout->attributes.size()) {
                return makeBackendError(BackendErrorCode::InvalidDescriptor, "createGeometryGL: bufferAttrib.attribIndex out of range for the supplied layout");
            }
            BufferHandle const handle = bufferAttrib.bufferHandle;
            uint32_t const index = handle.index();
            if (index >= buffers.size() || !buffers[index].obj || buffers[index].gen != handle.gen()) {
                return makeBackendError(BackendErrorCode::InvalidDescriptor, "createGeometryGL: bufferAttrib.bufferHandle is invalid or references a freed buffer");
            }
        }

        uint32_t const indexBufIdx = desc.indexBufferHandle.index();

        // check index buffer
        if (indexBufIdx >= buffers.size() || !buffers[indexBufIdx].obj || buffers[indexBufIdx].gen != desc.indexBufferHandle.gen()) {
            return makeBackendError(BackendErrorCode::InvalidDescriptor, "createGeometryGL: indexBufferHandle is invalid or references a freed buffer");
        }

        GeometryGL geometry;
        geometry.vao = glaze::makeUnique<gl::VertexArray>();
        geometry.indexDataType = layout->indexDataType;

        gl::bindVertexArray(geometry.vao);

        for (const GeometryBufferAttrib &bufferAttrib : desc.bufferAttribs) {
            VertexAttribGL const &attrib = layout->attributes[bufferAttrib.attribIndex];
            gl::BufferId const bufferId = buffers[bufferAttrib.bufferHandle.index()].obj->buffer.get();

            gl::bindBuffer(gl::BufferTarget::eArrayBuffer, bufferId);
            gl::enableVertexAttribArray(attrib.loc);
            gl::vertexAttribPointer(attrib.loc, attrib.dim, attrib.dataType, attrib.normalized, 0, nullptr);
        }

        // The element array buffer binding is part of VAO state, so this sticks to the geometry.
        gl::bindBuffer(gl::BufferTarget::eElementArrayBuffer, buffers[indexBufIdx].obj->buffer.get());

        gl::bindVertexArray({});

        auto const [index, gen] = acquireSlot(geometries, std::move(geometry));
        return GeometryHandle::make(gen, index);
    }

    void destroyGeometryGL(RenderDeviceBackendContext *ctx, GeometryHandle handle) {
        auto &geometries = glctx(ctx)->geometries;
        uint32_t const index = handle.index();
        assert(index < geometries.size() && "destroyGeometryGL: handle index out of range");
        auto &slot = geometries[index];
        assert(slot.obj && "destroyGeometryGL: slot already free (double destroy)");
        assert(slot.gen == handle.gen() && "destroyGeometryGL: stale handle (generation mismatch)");
        slot.obj.reset();
    }

} // namespace xe
