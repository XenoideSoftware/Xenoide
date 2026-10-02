
#ifndef __XE_GRAPHICS_PNG_IMAGELOADERPNG_HPP__
#define __XE_GRAPHICS_PNG_IMAGELOADERPNG_HPP__

#include <xe/graphics/ImageLoader.h>

namespace xe {
    class ImageLoaderPNG : public ImageLoader {
    public:
        virtual ~ImageLoaderPNG();

        virtual std::unique_ptr<Image> load(Stream *inputStream) override;
    };
} // namespace xe

#endif
