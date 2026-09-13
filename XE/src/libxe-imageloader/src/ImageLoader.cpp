
#include "xe/ImageLoader.h"
#include "ImageLoaderIL.h"

std::unique_ptr<ImageLoader> createImageLoader() {
    return {};
    // return std::make_unique<ImageLoaderIL>();
}
