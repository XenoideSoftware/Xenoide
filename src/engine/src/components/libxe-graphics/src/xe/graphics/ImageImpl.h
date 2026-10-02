
#ifndef __XE_GRAPHICS_IMAGEIMPL_HPP__
#define __XE_GRAPHICS_IMAGEIMPL_HPP__

#include "Image.h"

namespace xe {
    /**
     * @brief Image implementation class.
     */
    class ImageImpl : public Image {
    public:
        ImageImpl();

        virtual ~ImageImpl();

        virtual const void *getPointer() const override;

        virtual PixelFormat getFormat() const override;

        virtual Vector2i getSize() const override;

        void allocate(const PixelFormat newFormat, const Vector2i &newSize);

        void free();

    private:
        void *pixels = nullptr;
        PixelFormat format = PixelFormat::Unknown;
        Vector2i size = {0, 0};
    };
} // namespace xe

#endif
