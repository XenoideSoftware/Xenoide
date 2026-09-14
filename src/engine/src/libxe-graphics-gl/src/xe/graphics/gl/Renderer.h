
#pragma once

#if 0

#include "TextureRepository.h"

#include <algorithm>
#include <fstream>
#include <glaze/gl.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <optional>
#include <string>
#include <vector>

struct Camera {
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 lookAt = {0.0f, 0.0f, -1.0f};
    glm::vec3 up = {0.0f, 1.0f, 0.0f};

    float fov = 60.0f;
    float znear = 0.1f;
    float zfar = 100.0f;
    float aspect = 4.0f / 3.0f;
};

struct Transformation {
    glm::vec3 scaling = glm::vec3(1.0f);
    glm::vec3 translation = glm::vec3(0.0f);

    glm::mat4 computeMatrix() const {
        auto model = glm::identity<glm::mat4>();

        model = glm::scale(model, scaling);
        model = glm::translate(model, translation);

        return model;
    }
};

struct MaterialChannel {
    glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
    GLuint textureMap = 0;
};

struct Material {
    MaterialChannel ambient;
    MaterialChannel diffuse;
    MaterialChannel specular;
    MaterialChannel emissive;
};

struct Light {
    glm::vec3 direction = glm::normalize(glm::vec3{0.5f, 1.0f, 0.25f});
    glm::vec4 ambient = {0.6f, 0.6f, 0.6f, 1.0f};
    glm::vec4 diffuse = {0.8f, 0.8f, 0.8f, 0.8f};
};

struct Lighting {
    glm::vec4 globalAmbient = {0.0f, 0.0f, 0.0f, 1.0f};
    std::vector<Light> lights;
    bool enabled = true;
};

struct MaterialImportedProperties {
    std::optional<glm::vec4> ambient;
    std::optional<glm::vec4> diffuse;
    std::optional<glm::vec4> specular;
    std::optional<glm::vec4> emissive;
    std::optional<glm::vec4> transparent;
    std::optional<glm::vec4> reflective;
};

struct ShaderLocationMap {
    GLint coord = -1;
    GLint normal = -1;
    GLint texCoord = -1;

    GLint uModel = -1;
    GLint uView = -1;
    GLint uProj = -1;

    GLint uMaterialDiffuseSamplerEnable = -1;
    GLint uMaterialDiffuseSampler = -1;
    GLint uMaterialAmbient = -1;
    GLint uMaterialDiffuse = -1;
    GLint uMaterialSpecular = -1;

    GLint uLightAmbient = -1;
    GLint uLightDirection = -1;
    GLint uLightDiffuse = -1;
};

struct Mesh {
    GLuint vao = 0;
    GLenum primitiveType = GL_TRIANGLES;
    bool indexed = false;
    size_t count = 0;
    GLenum indexDataType = GL_UNSIGNED_INT;

    std::optional<size_t> material;

    Mesh() {
    }

    bool empty() const {
        return vao == 0;
    }
};

struct MeshAttribute {
    void *data = nullptr;
    size_t size = 0;

    bool empty() const {
        return data == nullptr || size == 0;
    }
};

struct MeshData {
    MeshAttribute vertexCoord;
    MeshAttribute vertexNormal;
    MeshAttribute vertexTexCoord;
    MeshAttribute indices;

    size_t vertexCount = 0;
    size_t indexCount = 0;
    size_t materialIndex = 0;
};

class Platform;
class Renderer {
public:
    explicit Renderer(Platform &platform);

    bool initialize();

    virtual ~Renderer();

    GLuint createShader(const std::string &source, const GLenum type);

    GLuint createShaderProgram(const std::vector<GLuint> &shaders);

    GLuint createBuffer(const GLenum target, const GLsizeiptr size, const void *data, GLenum usage);

    GLuint createBuffer(const GLenum target, const MeshAttribute &attrib, GLenum usage) {
        return createBuffer(target, attrib.size, attrib.data, usage);
    }

    template <class ArrayLike> GLuint createBuffer(const GLenum target, const ArrayLike &values, GLenum usage) {
        using T = typename ArrayLike::value_type;

        return createBuffer(target, sizeof(T) * values.size(), values.data(), usage);
    }

    ShaderLocationMap createShaderLocationMap(const GLuint program);

    void beginRenderFrame();

    void endRenderFrame();

    Mesh createMeshVAO(const ShaderLocationMap &location, const MeshData &meshData);

    GLuint createTexture(GLenum internalFormat, const unsigned width, const unsigned height, const GLenum format, const GLenum type, const void *data);

    GLuint createTexture(
        GLenum internalFormat, const unsigned width, const unsigned height, const GLenum format, const GLenum type, const void *data, const GLuint wrapS, const GLuint wrapT
    );

    void renderCamera(const ShaderLocationMap &location, const Camera &camera);

    void renderLighting(const GLuint programId, const Lighting &lighting);

    void renderMaterial(const GLuint programId, const Material &material);

    void renderMaterialChannel(const GLuint programId, const std::string &uniformPrefix, const GLint textureUnit, const MaterialChannel &channel);

    void renderModelTransform(const ShaderLocationMap &location, const float *transform);

    void renderMesh(const Mesh &mesh);

    void renderMeshes(const Mesh *meshes, const size_t count);

    void renderTranformation(const ShaderLocationMap &location, const Transformation &transformation);

public:
    Platform &platform;
    GLuint program = 0;
};

#endif
