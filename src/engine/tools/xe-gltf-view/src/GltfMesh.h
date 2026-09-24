

#pragma once

#include "xe/graphics/gl/RendererGL.h"
#include <glaze/gl.h>
#include <vector>

struct GltfMaterial {
    xe::gl::Texture texture;
    std::vector<xe::gl::TextureParameter> parameters;
};

struct GltfIndexData {
    xe::gl::Buffer buffer;
    GLenum type = GL_UNSIGNED_INT;
};

// Contains OpenGL objects with data loaded from a gltf mesh
struct GltfMeshPrimitive {
    // TODO: this should be a reference / pointer.
    GltfMaterial material;

    xe::gl::VertexArray vao;
    xe::gl::Buffer vertexBuffer;
    GLenum primitive = GL_NONE;
    GLsizei count = 0;
    std::optional<GltfIndexData> indexData;
};

struct GltfMesh {
    std::string name;
    std::vector<GltfMeshPrimitive> primitives;
};
