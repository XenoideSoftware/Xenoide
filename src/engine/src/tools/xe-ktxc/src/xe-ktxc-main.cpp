
#include <cassert>
#include <filesystem>
#include <map>
#include <bpstd/span.hpp>
#include <string>

#include <IL/il.h>
#include <IL/ilu.h>
#include <cxxopts.hpp>
#include <iostream>
#include <ktx.h>
#include <vulkan/vulkan.h>

#include "xe/Logger.h"

using ImageSpan = bpstd::span<uint8_t>;
using FileSpan = bpstd::span<uint8_t>;

struct ImageDesc {
    int width = 0;
    int height = 0;
    int bitsPerChannel = 0;
    int channels = 0;
    int bitsPerPixel = 0;
    ILenum dataType = 0;
    ILenum format = 0;
    ImageSpan data;
};

inline std::string to_string(ILenum t) {
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
};

ILenum makeImageType(const std::string &ext) {
    if (const auto it = typeILMap.find(ext); it != typeILMap.end()) {
        return it->second;
    }

    return IL_TYPE_UNKNOWN;
}

static void logDevILErrors(const char *ctx) {
    for (ILenum err = ilGetError(); err != IL_NO_ERROR; err = ilGetError()) {
        XE_LOG_WARNING("{}: DevIL error 0x{:X}\n", ctx, int(err));
    }
}

ImageDesc describeCurrentImage() {
    /*
    if (!ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE)) {
        logDevILErrors("ilConvertImage");
        return {};
    }
    */

    ImageDesc image;
    image.width = ilGetInteger(IL_IMAGE_WIDTH);
    image.height = ilGetInteger(IL_IMAGE_HEIGHT);
    image.channels = ilGetInteger(IL_IMAGE_CHANNELS);
    image.bitsPerPixel = ilGetInteger(IL_IMAGE_BITS_PER_PIXEL);
    image.dataType = ilGetInteger(IL_IMAGE_TYPE);
    image.format = ilGetInteger(IL_IMAGE_FORMAT);
    image.data = ImageSpan{ilGetData(), static_cast<size_t>(ilGetInteger(IL_IMAGE_SIZE_OF_DATA))};

    return image;
}

ILuint createImage(const FileSpan &fileSpan, ILenum imageType) {
    const auto data = fileSpan.data();
    const auto size = fileSpan.size();

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

    return id;
}

ILuint createImage(const std::string &file) {
    XE_LOG_INFO("Loading image: {}\n", file);

    std::filesystem::path path{file};
    if (!std::filesystem::exists(path)) {
        XE_LOG_ERROR("Bitmap file {} doesn't exist.\n", file);
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

    return id;
}

const std::map<std::pair<ILuint, ILuint>, ktx_uint32_t> vkFormatMap = {
    {{IL_LUMINANCE, IL_UNSIGNED_BYTE}, VK_FORMAT_R8_UNORM},
    {{IL_LUMINANCE_ALPHA, IL_UNSIGNED_BYTE}, VK_FORMAT_R8G8_UNORM},
    {{IL_RGB, IL_UNSIGNED_BYTE}, VK_FORMAT_R8G8B8_UNORM},
    {{IL_RGBA, IL_UNSIGNED_BYTE}, VK_FORMAT_R8G8B8A8_UNORM},
};

ktx_uint32_t computeVkFormat(const ImageDesc &imageDesc) {
    const auto key = std::make_pair(imageDesc.format, imageDesc.dataType);

    if (const auto it = vkFormatMap.find(key); it != vkFormatMap.end()) {
        return it->second;
    }

    return VK_FORMAT_UNDEFINED;
}

void writeTextureKTX2(const std::string &fileName, const ImageDesc &image) {
    XE_LOG_INFO("Creating KTX2 texture\n");
    const uint32_t height = static_cast<uint32_t>(image.width);
    const uint32_t width = static_cast<uint32_t>(image.height);
    const uint32_t depth = 1;

    // assert(depth >= 1 && depth <= 4 && "Expects an image with 1 to 4 color channels");
    assert(depth == 1 && "Only gray-scale images are supported for KTX export for now");

    // Prepare KTX texture info
    ktxTextureCreateInfo createInfo{};
    createInfo.baseWidth = width;
    createInfo.baseHeight = height;
    createInfo.baseDepth = depth;
    createInfo.numDimensions = 2;
    createInfo.numLevels = 1;
    createInfo.numLayers = 1;
    createInfo.numFaces = 1;
    createInfo.isArray = KTX_FALSE;
    createInfo.generateMipmaps = KTX_FALSE;
    createInfo.vkFormat = computeVkFormat(image);

    if (createInfo.vkFormat == VK_FORMAT_UNDEFINED) {
        throw std::runtime_error("Could not determine texture format from image description");
    }

    ktxTexture2 *texture = nullptr;
    KTX_error_code result = ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);
    if (result != KTX_SUCCESS) {
        throw std::runtime_error("Failed to create KTX texture");
    }

    result = ktxTexture_SetImageFromMemory(ktxTexture(texture), 0, 0, 0, image.data.data(), image.data.size());

    if (result != KTX_SUCCESS) {
        ktxTexture_Destroy(ktxTexture(texture));
        throw std::runtime_error("Failed to upload image to KTX texture");
    }

    const bool compress = true;

    if (compress) {
        XE_LOG_INFO("Compressing KTX2 texture\n");
        // Basis compression parameters
        ktxBasisParams params{};

        // ETC1S (smaller) or UASTC (higher quality) can be chosen
        params.structSize = sizeof(ktxBasisParams);
        params.uastc = KTX_FALSE; // set to KTX_TRUE for UASTC instead of ETC1S
        params.verbose = KTX_FALSE;

        result = ktxTexture2_CompressBasisEx(texture, &params);
        if (result != KTX_SUCCESS) {
            ktxTexture_Destroy(ktxTexture(texture));
            throw std::runtime_error("Failed to compress KTX texture with BasisU");
        }
    }

    XE_LOG_INFO("Writing KTX2 texture to file: {}\n", fileName);
    ktxTexture_WriteToNamedFile(ktxTexture(texture), fileName.c_str());
}

std::optional<ILenum> paletteTypeToFormat(ILenum paletteType) {
    switch (paletteType) {
    case IL_PAL_RGB24:
    case IL_PAL_RGB32:
    case IL_PAL_BGR24:
    case IL_PAL_BGR32:
        return IL_RGB;

    case IL_PAL_RGBA32:
    case IL_PAL_BGRA32:
        return IL_RGBA;

    default:
        return {};
    }
}

enum class KtxcOutputFormat { KTX, KTX2 };

struct KtxcOptions {
    std::filesystem::path inputImageFilePath = "";
    KtxcOutputFormat outputFormat = KtxcOutputFormat::KTX2;
};

void compileImage(const KtxcOptions &options) {
    ilInit();
    iluInit();

    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

    const std::filesystem::path inputFilePath = options.inputImageFilePath;

    const ILuint imageId = createImage(inputFilePath.string());
    ilBindImage(imageId);

    // convert images with palettes to appropiate RGB/A formats,
    if (const ILenum format = ilGetInteger(IL_IMAGE_FORMAT); format == IL_COLOUR_INDEX) {
        const ILenum paletteType = ilGetInteger(IL_PALETTE_TYPE);
        const std::optional<ILenum> destFormatOpt = paletteTypeToFormat(paletteType);

        if (!destFormatOpt.has_value()) {
            throw std::runtime_error("Could not determine output format from image description");
        }

        if (!ilConvertImage(destFormatOpt.value(), IL_UNSIGNED_BYTE)) {
            logDevILErrors("ilConvertImage");
            throw std::runtime_error("Could not convert paletted image");
        }
    }

    const ImageDesc image = describeCurrentImage();

    switch (options.outputFormat) {
    case KtxcOutputFormat::KTX:
        // writeTextureKTX(outputFilePath, image);
        // break;
        throw std::runtime_error("Only KTX2 file format is supported for now");

    case KtxcOutputFormat::KTX2: {
        const std::filesystem::path outputFilePath = inputFilePath.parent_path() / (inputFilePath.stem().string() + ".ktx2");
        writeTextureKTX2(outputFilePath.string(), image);
        break;
    }
    }

    ilShutDown();
}

std::optional<KtxcOptions> parseCommandLine(const int argc, char *argv[]) {
    cxxopts::Options options("xe-ktxc", "KTX texture compiler");

    options.add_options()("h,help", "Print usage")("i,input-file", "Input image file", cxxopts::value<std::string>())
        // ("v,verbose", "Enable verbose output", cxxopts::value<bool>()->default_value("false"))
        ("f,output-format", "Output format", cxxopts::value<std::string>()->default_value("ktx2"));

    const auto parseResult = options.parse(argc, argv);

    if (parseResult.count("help")) {
        std::cout << options.help() << '\n';
        return {};
    }

    KtxcOptions result;

    if (parseResult.count("input-file")) {
        result.inputImageFilePath = parseResult["input-file"].as<std::string>();
    } else {
        throw std::runtime_error("No input image file specified");
    }

    if (parseResult.count("output-format")) {
        const std::string outputFormat = parseResult["output-format"].as<std::string>();
        if (outputFormat == "ktx2") {
            result.outputFormat = KtxcOutputFormat::KTX2;
        } else if (outputFormat == "ktx") {
            result.outputFormat = KtxcOutputFormat::KTX;
        } else {
            throw std::runtime_error("Unknown output-format specified");
        }
    } else {
        throw std::runtime_error("No output-format specified.");
    }

    return result;
}

int main(int argc, char *argv[]) {
    try {
        if (std::optional<KtxcOptions> options = parseCommandLine(argc, argv); options) {
            compileImage(options.value());
        }

        return EXIT_SUCCESS;
    } catch (const std::exception &e) {
        XE_LOG_ERROR("Error while compiling image\n");
        XE_LOG_ERROR("{}\n", e.what());
        return EXIT_FAILURE;
    }
}
