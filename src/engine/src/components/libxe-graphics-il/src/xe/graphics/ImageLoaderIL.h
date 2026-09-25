
#pragma once

#include "xe/ImageLoader.h"

class ImageLoaderIL : public ImageLoader {
public:
    ImageLoaderIL();

    ~ImageLoaderIL();

    std::unique_ptr<Image> loadImage(const std::string &file) const override;

    std::unique_ptr<Image> loadImage(const void *data, const size_t size, const std::string &compressionFormat) const override;

    std::unique_ptr<Image> loadImage(const void *data, const size_t size, const ImageFormat &format) const override;
};
