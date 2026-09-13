
#pragma once

#include "GltfMesh.h"
#include "xe/gl/RendererGL.h"

#include <cgltf.h>
#include <memory>
#include <map>

namespace xe::gltf_view {
    struct ShaderProgramUniformData {
        xe::Matrix4 projViewModel = xe::mat4Identity();

        xe::Matrix4 model = xe::mat4Identity();

        // The Texture Unit which contains the diffuse texture
        GLint diffuseTexture = 0;

        float seconds = 0.0f;

        [[nodiscard]]
        std::vector<xe::gl::UniformMatrix> mapMatrixUniforms(xe::gl::Program shaderProgram) const;

        [[nodiscard]]
        std::vector<xe::gl::Uniform> mapUniforms(xe::gl::Program shaderProgram) const;
    };

    class GltfRenderer {
    public:
        explicit GltfRenderer(const std::string &filePath);

        ~GltfRenderer();

        void render();

        void beginFrame();

        void endFrame();

        [[nodiscard]]
        cgltf_data *getData() const {
            return data;
        }

    private:
        void renderScene(const cgltf_scene &scene);

        void renderNode(const xe::Matrix4 &parentTransformation, const cgltf_node &node);

        void renderCamera(const cgltf_camera &camera);

        void renderMesh(const cgltf_mesh &mesh);

        std::map<const cgltf_mesh *, GltfMesh> meshMap;

        std::unique_ptr<xe::gl::RendererGL> renderer;
        ShaderProgramUniformData uniformData;
        float startSeconds = 0.0f;
        float angle = 0.0f;
        xe::gl::Program program;
        std::vector<GltfMesh> meshes;
        cgltf_data *data = nullptr;

        xe::Matrix4 projection = xe::mat4Identity();
    };
} // namespace xe::gltf_view
