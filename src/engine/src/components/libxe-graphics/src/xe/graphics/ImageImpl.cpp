
#include <xe/graphics/ImageImpl.h>

#include <cassert>
#include <cstdlib>

namespace xe {
    ImageImpl::ImageImpl() {
    }

    ImageImpl::~ImageImpl() {
        this->free();
    }

    const void *ImageImpl::getPointer() const {
        return pixels;
    }

    PixelFormat ImageImpl::getFormat() const {
        return format;
    }

    Vector2i ImageImpl::getSize() const {
        return size;
    }

    void ImageImpl::allocate(const PixelFormat newFormat, const Vector2i &newSize) {
        this->free();
        format = newFormat;
        size = newSize;

        const int pixelByteSize = ComputeStorage(format);
        const int byteSize = size.x * size.y * pixelByteSize;

        assert(byteSize);

        pixels = static_cast<void *>(std::malloc(byteSize));
    }

    void ImageImpl::free() {
        if (pixels) {
            std::free(pixels);
            pixels = nullptr;

            format = PixelFormat::Unknown;
            size = {0, 0};
        }
    }
} // namespace xe
