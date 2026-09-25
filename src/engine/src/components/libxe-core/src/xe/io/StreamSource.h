
#ifndef __XE_IO_STREAMSOURCE_HPP__
#define __XE_IO_STREAMSOURCE_HPP__

#include <memory>
#include <string>

namespace xe {
    class Stream;
    class StreamSource {
    public:
        virtual ~StreamSource();

        virtual std::unique_ptr<Stream> open(const std::string &path) = 0;

        virtual bool exists(const std::string &path) const = 0;
    };
} // namespace xe

#endif
