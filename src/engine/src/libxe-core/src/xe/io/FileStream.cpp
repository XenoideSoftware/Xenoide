
#include <stdexcept>
#include <xe/io/FileStream.h>

namespace xe {
    FileStream::FileStream(const std::string &fileName, const StreamFlags flags) : m_fileName(fileName), m_flags(flags) {

        switch (flags) {
        case StreamFlags::Readable:
            m_fileHandle = std::fopen(fileName.c_str(), "rb");
            break;

        case StreamFlags::Writtable:
            m_fileHandle = std::fopen(fileName.c_str(), "ab");
            break;

        case StreamFlags::Both:
            m_fileHandle = std::fopen(fileName.c_str(), "r+b");
            break;
        }

        if (!m_fileHandle) {
            throw std::runtime_error("Couldn't open file \"" + fileName + "\".");
        }
    }

    FileStream::~FileStream() {
    }

    int FileStream::read(void *buffer, const int size, const int count) {
        return (int)std::fread(buffer, size, count, m_fileHandle);
    }

    int FileStream::write(const void *buffer, const int size, const int count) {
        return (int)std::fwrite(buffer, size, count, m_fileHandle);
    }

    bool FileStream::seek(const int offset, const StreamOffset position) {
        switch (position) {
        case StreamOffset::Current:
            std::fseek(m_fileHandle, offset, SEEK_CUR);
            break;

        case StreamOffset::End:
            std::fseek(m_fileHandle, offset, SEEK_END);
            break;

        case StreamOffset::Set:
            std::fseek(m_fileHandle, offset, SEEK_SET);
            break;
        }

        return true;
    }

    int FileStream::tell() const {
        return (int)std::ftell(m_fileHandle);
    }

    StreamFlags FileStream::getFlags() const {
        return m_flags;
    }
} // namespace xe
