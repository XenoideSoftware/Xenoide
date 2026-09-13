
#ifndef __XE_IO_MEMORYSTREAM_HPP__
#define __XE_IO_MEMORYSTREAM_HPP__

#include <cstddef>
#include <cstdint>

#include "Stream.h"

namespace xe {
    class MemoryStream : public Stream {
    public:
        MemoryStream(std::uint8_t *data, const std::size_t size);

        virtual ~MemoryStream();

        virtual int read(void *buffer, const int size, const int count) override;

        virtual int write(const void *buffer, const int size, const int count) override;

        virtual bool seek(const int offset, const StreamOffset position) override;

        virtual int tell() const override;

        virtual StreamFlags getFlags() const override;

    private:
        std::uint8_t *m_data = nullptr;
        const std::size_t m_size;

        int m_offset = 0;
    };
} // namespace xe

#endif
