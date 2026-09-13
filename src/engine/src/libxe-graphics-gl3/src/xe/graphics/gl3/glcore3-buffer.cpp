
#include "glcore3-context.h"

namespace xe {

    gl::BufferTarget toBufferTargetGL(BufferType type) {
        switch (type) {
        case BufferType::Vertex:
            return gl::BufferTarget::eArrayBuffer;

        case BufferType::Index:
            return gl::BufferTarget::eElementArrayBuffer;

        case BufferType::Uniform:
            return gl::BufferTarget::eUniformBuffer;

        default:
            assert(false && "toTextureTargetGL: Invalid BufferType");
            return gl::BufferTarget::eArrayBuffer;
        }
    }

    gl::BufferUsage toBufferUsageGL(const BufferUsage usage) {
        switch (usage) {
        case BufferUsage::StreamDraw:
            return gl::BufferUsage::eStreamDraw;
        case BufferUsage::StreamRead:
            return gl::BufferUsage::eStreamRead;
        case BufferUsage::StreamCopy:
            return gl::BufferUsage::eStreamCopy;
        case BufferUsage::StaticDraw:
            return gl::BufferUsage::eStaticDraw;
        case BufferUsage::StaticRead:
            return gl::BufferUsage::eStaticRead;
        case BufferUsage::StaticCopy:
            return gl::BufferUsage::eStaticCopy;
        case BufferUsage::DynamicDraw:
            return gl::BufferUsage::eDynamicDraw;
        case BufferUsage::DynamicRead:
            return gl::BufferUsage::eDynamicRead;
        case BufferUsage::DynamicCopy:
            return gl::BufferUsage::eDynamicCopy;
        }

        assert(false && "toBufferUsageGL: Invalid BufferUsage");
        return gl::BufferUsage::eStreamDraw;
    }

    tl::expected<BufferHandle, BackendError> createBufferGL(RenderDeviceBackendContext *ctx, const BufferDescriptor &desc) {
        auto &buffers = glctx(ctx)->buffers;

        if (!poolHasCapacity(buffers)) {
            return makeBackendError(BackendErrorCode::HandlePoolExhausted, "buffer pool exhausted (16-bit index field)");
        }

        gl::BufferTarget const target = toBufferTargetGL(desc.type);
        gl::BufferUsage const usage = toBufferUsageGL(desc.usage);

        auto buffer = glaze::makeUnique<gl::BufferId>();
        gl::bindBuffer(target, buffer);
        gl::bufferData(target, desc.size, desc.data, usage);
        gl::bindBuffer(target, {});

        auto const [index, gen] = acquireSlot(buffers, BufferGL{std::move(buffer)});
        return BufferHandle::make(gen, index, desc.type);
    }

    void destroyBufferGL(RenderDeviceBackendContext *ctx, BufferHandle handle) {
        auto &buffers = glctx(ctx)->buffers;
        uint32_t const index = handle.index();
        assert(index < buffers.size() && "destroyBufferGL: handle index out of range");
        auto &slot = buffers[index];
        assert(slot.obj && "destroyBufferGL: slot already free (double destroy)");
        assert(slot.gen == handle.gen() && "destroyBufferGL: stale handle (generation mismatch)");

        slot.obj.reset();
    }

    void readBufferGL(RenderDeviceBackendContext *ctx, BufferHandle handle, const BufferReadDescriptor &desc) {
        assert(desc.data != nullptr && "BufferReadDescriptor: data must not be null");
        assert(desc.size > 0 && "BufferReadDescriptor: size must be greater than zero");

        auto &buffers = glctx(ctx)->buffers;
        uint32_t const index = handle.index();
        assert(index < buffers.size() && "readBufferGL: handle index out of range");
        auto &slot = buffers[index];
        assert(slot.obj && "readBufferGL: use of freed handle");
        assert(slot.gen == handle.gen() && "readBufferGL: stale handle (generation mismatch)");

        gl::BufferTarget const target = toBufferTargetGL(handle.subType());

        gl::bindBuffer(target, slot.obj->buffer);
        gl::getBufferSubData(target, static_cast<GLintptr>(desc.offset), static_cast<GLsizeiptr>(desc.size), desc.data);
        gl::bindBuffer(target, {});
    }

} // namespace xe
