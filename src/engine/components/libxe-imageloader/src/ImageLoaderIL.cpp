#include "ImageLoaderIL.h"

#include "xe/Logger.h"
#include <cassert>
#include <filesystem>
#include <map>

#include <IL/il.h>
#include <IL/ilu.h>

class ImageIL : public Image {
public:
    explicit ImageIL(ILuint imageId, int width, int height, int bpp) : mImageId(imageId), mWidth(width), mHeight(height), mBpp(bpp) {
    }

    ~ImageIL() override {
        if (mImageId != 0) {
            ilDeleteImages(1, &mImageId);
        }
    }

    ImageData getData() const override {
        ilBindImage(mImageId);
        ImageData data;
        data.width = mWidth;
        data.height = mHeight;
        data.bpp = mBpp;
        data.pixels = ilGetData(); // owned by DevIL image until deletion
        return data;
    }

private:
    ILuint mImageId = 0;
    int mWidth = 0;
    int mHeight = 0;
    int mBpp = 0;
};

inline std::string str(ILenum t) {
    switch (t) {
    case IL_PNG:
        return "IL_PNG";
    case IL_JPG:
        return "IL_JPG";
    default:
        return "ILenum(" + std::to_string(static_cast<int>(t)) + ")";
    }
}

static std::map<std::string, ILenum> typeILMap = {
    {".png", IL_PNG},
    {".jpg", IL_JPG},
    {".jpeg", IL_JPG},
    {"png", IL_PNG},
    {"jpg", IL_JPG},
    {"jpeg", IL_JPG},
};

static ILenum mapType(const std::string &ext) {
    auto it = typeILMap.find(ext);
    if (it == typeILMap.end())
        return 0;
    return it->second;
}

static ILenum mapType(const ImageFormat &fmt) {
    switch (fmt) {
    case ImageFormat::Png:
        return IL_PNG;
    case ImageFormat::Jpeg:
        return IL_JPG;
    default:
        return 0;
    }
}

static void logDevILErrors(const char *ctx) {
    for (ILenum err = ilGetError(); err != IL_NO_ERROR; err = ilGetError()) {
        XE_LOG_WARNING("{}: DevIL error 0x{:X}\n", ctx, int(err));
    }
}

static std::unique_ptr<Image> createImageFromBoundIL(ILuint id) {
    if (!ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE)) {
        logDevILErrors("ilConvertImage");
        ilDeleteImages(1, &id);
        return {};
    }

    const int width = ilGetInteger(IL_IMAGE_WIDTH);
    const int height = ilGetInteger(IL_IMAGE_HEIGHT);
    const int bppChannels = ilGetInteger(IL_IMAGE_CHANNELS);
    int bpp = bppChannels * 8; // bits
    return std::make_unique<ImageIL>(id, width, height, bpp);
}

static std::unique_ptr<Image> createImage(const void *data, size_t size, ILenum imageType) {
    XE_LOG_INFO("Loading {} image from memory buffer {}\n", str(imageType), data);

    ILuint id = 0;
    ilGenImages(1, &id);
    ilBindImage(id);

    bool ok = false;
    if (imageType != 0) {
        ok = (ilLoadL(imageType, (const ILubyte *)data, (ILuint)size) == IL_TRUE);
    } else {
        ILenum detected = ilDetermineTypeL((const ILubyte *)data, (ILuint)size);
        if (detected != IL_TYPE_UNKNOWN)
            ok = (ilLoadL(detected, (const ILubyte *)data, (ILuint)size) == IL_TRUE);
    }

    if (!ok) {
        logDevILErrors("ilLoadL");
        ilDeleteImages(1, &id);
        return {};
    }

    return createImageFromBoundIL(id);
}

ImageLoaderIL::ImageLoaderIL() {
    ilInit();
    iluInit();

    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
}

ImageLoaderIL::~ImageLoaderIL() {
    ilShutDown();
}

std::unique_ptr<Image> ImageLoaderIL::loadImage(const std::string &file) const {
    std::filesystem::path path{file};
    if (!std::filesystem::exists(path)) {
        XE_LOG_WARNING("Bitmap file {} doesn't exist.\n", file);
        return {};
    }

    ILuint id = 0;
    ilGenImages(1, &id);
    ilBindImage(id);

    if (!ilLoadImage(file.c_str())) {
        logDevILErrors("ilLoadImage");
        ilDeleteImages(1, &id);
        return {};
    }

    return createImageFromBoundIL(id);
}

std::unique_ptr<Image> ImageLoaderIL::loadImage(const void *data, size_t size, const std::string &compressionFormat) const {
    ILenum t = mapType(compressionFormat);
    return createImage(data, size, t);
}

std::unique_ptr<Image> ImageLoaderIL::loadImage(const void *data, size_t size, const ImageFormat &format) const {
    ILenum t = mapType(format);
    return createImage(data, size, t);
}
