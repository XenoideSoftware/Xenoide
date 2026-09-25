
#ifndef __XE_GRAPHICS_IMAGELOADER_HPP__
#define __XE_GRAPHICS_IMAGELOADER_HPP__

#include <memory>

namespace xe {
    class Stream;
}

namespace xe {
    class Image;

    /**
     * @brief ImageLoader loader for loading images from a Stream
     */
    class ImageLoader {
    public:
        virtual ~ImageLoader();

        /**
         * @brief Constructs a Image object from an input stream.
         */
        virtual std::unique_ptr<Image> load(Stream *inputStream) = 0;
    };
} // namespace xe

#endif
