
#include "TextureRepository.h"

#include "xe/Image.h"
#include "xe/ImageLoader.h"
#include "xe/Logger.h"
#include "xe/graphics/gl/Renderer.h"

#include <cassert>
#include <iostream>

TextureRepository::TextureRepository(ImageLoader &loader) : loader{loader} {
}

GLuint TextureRepository::getOrCreate(const std::string &filePath, Renderer &renderer) {
    if (filePath.empty()) {
        std::cout << "TextureRepository::getOrCreate: " << "Can't load texture with empty filepath" << '\n';
        return 0;
    }

    if (auto it = cachedTextureMap.find(filePath); it != cachedTextureMap.end()) {
        return it->second;
    }

    GLuint texture = createTexture(filePath.c_str(), renderer);
    if (!texture) {
        std::cout << "TextureRepository::getOrCreate: " << "Texture file couldn't be loaded: '" << filePath << "'" << '\n';
        return 0;
    }

    std::cout << "TextureRepository::getOrCreate: Loaded texture '" << filePath << "'" << '\n';

    cachedTextureMap[filePath] = texture;

    return texture;
}

GLuint TextureRepository::createTexture(const char *theFileName, Renderer &renderer) {
    assert(theFileName);
    assert(std::string(theFileName) != "");

    const std::unique_ptr<Image> image = loader.loadImage(theFileName);

    if (!image) {
        return 0;
    }

    const ImageData imageData = image->getData();

    return createTexture(renderer, imageData);
}

GLuint TextureRepository::createTexture(Renderer &renderer, const std::string &imageTypeStr, const size_t size, const void *imageData) const {
    const auto image = loader.loadImage(imageData, size, imageTypeStr);
    return createTexture(renderer, image->getData());
}

GLuint TextureRepository::createTexture(Renderer &/*renderer*/, const ImageData &/*image*/) const {
    
    return {};
}
