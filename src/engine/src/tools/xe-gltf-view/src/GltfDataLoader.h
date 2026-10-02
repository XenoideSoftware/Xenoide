
#pragma once

#include <iostream>
#include <map>

#include "GltfMesh.h"
#include "GltfUtil.h"
#include "cgltf.h"
#include "xe/ImageLoader.h"
#include "xe/graphics/gl/RendererGL.h"
#include "xe/graphics/ImageLoader.h"

namespace xe::gl {
    class RendererGL;
}

class GltfTextureLoader {
public:
    GltfTextureLoader(const xe::gl::RendererGL *renderer, ImageLoader *imageLoader);

    xe::gl::Texture createTexture(const cgltf_texture_view &texture) const;

private:
    const xe::gl::RendererGL *renderer = nullptr;
    const ImageLoader *imageLoader = nullptr;
};

using GltfAttributeName = std::string;
using ShaderAttributeName = std::string;

struct ShaderAttrib {
    std::string name;
    bool required = false;
    explicit ShaderAttrib(std::string name) : name(std::move(name)) {
    }
    explicit ShaderAttrib(std::string name, bool required) : name(std::move(name)), required(required) {
    }
};

using GltfAttributeMap = std::map<GltfAttributeName, ShaderAttrib>;

class GltfDataParser {
    cgltf_options options = {};

public:
    GltfDataParser() = default;

    explicit GltfDataParser(const cgltf_options &options) : options(options) {
    }

    [[nodiscard]]
    cgltf_data *parse(const std::string &filePath) const;
};

// Loads gltf data in the GPU
class GltfDataLoader {
    cgltf_data *data = nullptr;
    xe::gl::RendererGL *renderer = nullptr;
    GltfTextureLoader *textureLoader = nullptr;
    xe::gl::Program program;
    GltfAttributeMap attributeMap;

public:
    explicit GltfDataLoader(cgltf_data *data, xe::gl::RendererGL *renderer, GltfTextureLoader *textureLoader, xe::gl::Program program, const GltfAttributeMap &attributeMap);

    std::vector<GltfMesh> loadAllMeshes();

    void loadAllAnimations();

private:
    GltfMeshPrimitive createMeshPrimitive(const cgltf_primitive &primitive);

    GltfMesh createMesh(const cgltf_mesh *mesh);

    xe::gl::Buffer createIndexBuffer(const cgltf_accessor &accessor);

    xe::gl::Buffer createVertexBuffer(const cgltf_primitive &primitive);

    GLint computeAttributeLocation(const std::string &gltfAttributeName);

    xe::gl::VertexArray createVertexArray(const cgltf_primitive &primitive, xe::gl::Buffer vertexBuffer, xe::gl::Buffer indexBuffer);
};
