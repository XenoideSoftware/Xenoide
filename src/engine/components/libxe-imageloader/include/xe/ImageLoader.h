
#pragma once

#include <string>

#include "Image.h"

#include <memory>
#include <optional>

enum class ImageFormat { Jpeg, Png };

inline std::optional<ImageFormat> parseImageFormat(std::string_view str) {
    if (str == "image/jpeg") {
        return ImageFormat::Jpeg;
    }

    if (str == "image/png") {
        return ImageFormat::Png;
    }

    return std::nullopt;
}

class ImageLoader {
public:
    virtual ~ImageLoader() = default;

    virtual std::unique_ptr<Image> loadImage(const std::string &file) const = 0;

    virtual std::unique_ptr<Image> loadImage(const void *data, const size_t size, const std::string &compressionFormat) const = 0;

    virtual std::unique_ptr<Image> loadImage(const void *data, const size_t size, const ImageFormat &format) const = 0;
};

std::unique_ptr<ImageLoader> createImageLoader();
