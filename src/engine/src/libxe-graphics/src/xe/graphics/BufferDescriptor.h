
#ifndef __XE_GRAPHICS_BUFFERDESCRIPTOR_HPP__
#define __XE_GRAPHICS_BUFFERDESCRIPTOR_HPP__

#include <cstddef>
#include <xe/Predef.h>

namespace xe {
    enum class BufferUsage { StreamDraw, StreamRead, StreamCopy, StaticDraw, StaticRead, StaticCopy, DynamicDraw, DynamicRead, DynamicCopy };

    enum class BufferType { Vertex, Index, Uniform };

    struct BufferDescriptor {
        BufferType type;
        BufferUsage usage;
        std::size_t size;
        const void *data;
    };
} // namespace xe

#endif
