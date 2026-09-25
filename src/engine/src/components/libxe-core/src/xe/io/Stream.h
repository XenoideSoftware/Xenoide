
#ifndef __XE_IO_STREAM_HPP__
#define __XE_IO_STREAM_HPP__

namespace xe {
    enum class StreamOffset { Set, Current, End };

    enum class StreamFlags { Readable = 1, Writtable = 2, Both = Readable | Writtable };

    /**
     * @brief Streaming for binary data
     */
    class Stream {
    public:
        virtual ~Stream();

        virtual int read(void *buffer, const int size, const int count) = 0;

        virtual int write(const void *buffer, const int size, const int count) = 0;

        virtual bool seek(const int offset, const StreamOffset position) = 0;

        virtual int tell() const = 0;

        virtual StreamFlags getFlags() const = 0;
    };
} // namespace xe

#endif
