
#ifndef __XE_IO_FILESTREAM_HPP__
#define __XE_IO_FILESTREAM_HPP__

#include "Stream.h"
#include <cstdio>
#include <string>

namespace xe {
    /**
     * @brief Streaming for binary data
     */
    class FileStream : public Stream {
    public:
        FileStream(const std::string &fileName, const StreamFlags flags);

        virtual ~FileStream();

        virtual int read(void *buffer, const int size, const int count) override;

        virtual int write(const void *buffer, const int size, const int count) override;

        virtual bool seek(const int offset, const StreamOffset position) override;

        virtual int tell() const override;

        virtual StreamFlags getFlags() const override;

    private:
        std::string m_fileName;
        StreamFlags m_flags;

        FILE *m_fileHandle = nullptr;
    };
} // namespace xe

#endif
