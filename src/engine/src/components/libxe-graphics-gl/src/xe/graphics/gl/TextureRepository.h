
#pragma once

#include <glaze/gl.h>
#include <map>
#include <string>

struct ImageData;

class Renderer;
class ImageLoader;
class TextureRepository {
public:
    explicit TextureRepository(ImageLoader &loader);

    GLuint getOrCreate(const std::string &filePath, Renderer &renderer);

    GLuint createTexture(Renderer &renderer, const std::string &imageTypeStr, const size_t size, const void *imageData) const;

private:
    GLuint createTexture(const char *theFileName, Renderer &renderer);

    GLuint createTexture(Renderer &renderer, const ImageData &image) const;

private:
    ImageLoader &loader;
    std::map<std::string, GLuint> cachedTextureMap;
};
