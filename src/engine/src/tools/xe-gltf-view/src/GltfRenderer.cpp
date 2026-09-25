
#include "GltfRenderer.h"

#include <array>
#include <filesystem>
#include <stdexcept>
#include <cstddef>
#include <bpstd/span.hpp>

#include "fmt/printf.h"
#include "xe/graphics/gl/RendererGL.h"
#include "xe/Logger.h"
#include "xe/FileUtil.h"

#include "GltfDataLoader.h"
#include "GltfProcessor.h"
#include "xe/ImageLoader.h"
#include "xe/Timer.h"
#include "xe/math/Matrix.h"
#include "xe/math/Quaternion.h"
#include "xe/math/Vector.h"

namespace xe::gltf_view {

    template <typename T> const T SCREEN_WIDTH = T{640};

    template <typename T> const T SCREEN_HEIGHT = T{480};

    std::vector<xe::gl::UniformMatrix> ShaderProgramUniformData::mapMatrixUniforms(xe::gl::Program shaderProgram) const {
        return {
            xe::gl::makeUniform(shaderProgram.getUniformLocation("modelViewProj"), projViewModel),
        };
    }

    [[nodiscard]]
    std::vector<xe::gl::Uniform> ShaderProgramUniformData::mapUniforms(xe::gl::Program shaderProgram) const {
        return {
            xe::gl::makeUniform(shaderProgram.getUniformLocation("diffuseTexture"), diffuseTexture),
            xe::gl::makeUniform(shaderProgram.getUniformLocation("seconds"), seconds),
        };
    }

    GltfRenderer::GltfRenderer(const std::string &filePath) {
        renderer = xe::gl::RendererGL::create();

        const std::filesystem::path internalAssetsPath = XE_GLTF_VIEW_SOURCE_FOLDER;
        const std::string vertexShaderSource = xe::loadTextFile((internalAssetsPath / "shaders/gltf-view.vert").string());
        const std::string fragmentShaderSource = xe::loadTextFile((internalAssetsPath / "shaders/gltf-view.frag").string());

        std::vector<xe::gl::Shader> shaders = {
            renderer->createShader(GL_VERTEX_SHADER, vertexShaderSource.c_str()),
            renderer->createShader(GL_FRAGMENT_SHADER, fragmentShaderSource.c_str())
        };

        program = renderer->createProgram(bpstd::span<xe::gl::Shader>(shaders.data(), shaders.size()));
        if (!program.id) {
            throw std::runtime_error("Failed create program.");
        }

        auto gltfParser = GltfDataParser{};
        auto gltfData = gltfParser.parse(filePath);

        if (!gltfData) {
            throw std::runtime_error("Failed gltfParser.parse().");
        }

        data = gltfData;

        auto imageLoader = createImageLoader();
        auto gltfTextureLoader = GltfTextureLoader{renderer.get(), imageLoader.get()};

        auto gltfLoader = GltfDataLoader{
            gltfData,
            renderer.get(),
            &gltfTextureLoader,
            program,
            {{"POSITION", ShaderAttrib("vertCoord")}, {"NORMAL", ShaderAttrib{"vertNormal"}}, {"TEXCOORD_0", ShaderAttrib{"vertTexCoord"}}}
        };

        gltfLoader.loadAllAnimations();

        meshes = gltfLoader.loadAllMeshes();

        if (meshes.empty()) {
            throw std::runtime_error("Meshes could not be loaded.");
        }

        std::cout << meshes.size() << " meshes were loaded" << '\n';
    }

    GltfRenderer::~GltfRenderer() {
    }

    void GltfRenderer::beginFrame() {
        const auto clearFlags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
        const auto clearColor = xe::Vector4{0.2f, 0.2f, 0.8f, 1.0f};

        renderer->clear(clearFlags, clearColor, {}, {});
    }

    void GltfRenderer::endFrame() {
        renderer->flush();
    }

    void GltfRenderer::render() {
        uniformData.seconds = (static_cast<float>(xe::Timer::getTick()) / 1000.0f) - startSeconds;
        startSeconds = uniformData.seconds;

        const float aspect = SCREEN_HEIGHT<float> / SCREEN_WIDTH<float>;
        const auto proj = xe::mat4Perspective(xe::radians(60.0f), aspect, 0.001f, 1000.0f);

        const xe::Vector3 eye = {0.0f, 0.0f, -25.0f};
        const xe::Vector3 at = {0.0f, 0.0f, 0.0f};
        const xe::Vector3 up = {0.0f, 1.0f, 0.0f};
        const auto view = xe::mat4LookAtRH(eye, at, up);
        const auto model = xe::mat4RotationY(angle += 0.005f);

        uniformData.model = model;
        uniformData.projViewModel = proj * view * model;

        const std::vector<xe::gl::CapabilityStatus> renderState = {{GL_DEPTH_TEST, GL_TRUE}, {GL_CULL_FACE, GL_TRUE}};

        renderer->bindRenderState(bpstd::span<const xe::gl::CapabilityStatus>(renderState.data(), renderState.size()));
        renderer->useProgram(program);

        {
            const auto uniforms = uniformData.mapUniforms(program);
            renderer->bindRenderState(bpstd::span<const xe::gl::Uniform>(uniforms.data(), uniforms.size()));
        }
        {
            const auto matrixUniforms = uniformData.mapMatrixUniforms(program);
            renderer->bindRenderState(bpstd::span<const xe::gl::UniformMatrix>(matrixUniforms.data(), matrixUniforms.size()));
        }

        for (const auto &mesh : meshes) {
            for (const auto &meshSubset : mesh.primitives) {
                const xe::gl::VertexArrayPrimitive prims[] = {{0, meshSubset.count}};

                xe::gl::TextureLayer layer;
                layer.texture = meshSubset.material.texture;
                renderer->bindRenderState(bpstd::span<const xe::gl::TextureLayer>(&layer, static_cast<size_t>(1)));

                if (meshSubset.indexData.has_value()) {
                    renderer->draw(meshSubset.vao, meshSubset.primitive, prims, meshSubset.indexData->type);
                } else {
                    renderer->draw(meshSubset.vao, meshSubset.primitive, prims);
                }
            }
        }
    }

    xe::Matrix4 computeLocalTransformation(const cgltf_node &node) {
        if (node.has_matrix) {
            return xe::Matrix4(node.matrix);
        }

        if (node.has_rotation) {
            XE_LOG_ERROR("Node rotation is not supported. Defaulting to Identity");
            // transform = xe::Quat(node.rotation);
            return xe::mat4Identity();
        }

        if (node.has_scale) {
            return xe::mat4Scaling(xe::vec(xe::Vector3(node.scale), 1.0f));
        }

        if (node.has_translation) {
            return xe::mat4Translation(xe::Vector3(node.translation));
        }

        return xe::mat4Identity();
    }

    void GltfRenderer::renderScene(const cgltf_scene &scene) {
        bpstd::span<cgltf_node *> nodes{scene.nodes, scene.nodes_count};

        for (const cgltf_node *node : nodes) {
            if (node) {
                const xe::Matrix4 transformation = xe::mat4Identity();
                renderNode(transformation, *node);
            }
        }
    }

    void GltfRenderer::renderNode(const xe::Matrix4 &parentTransformation, const cgltf_node &node) {
        const xe::Matrix4 transformation = parentTransformation * computeLocalTransformation(node);

        if (node.camera) {
            renderCamera(*node.camera);
        }

        if (node.mesh) {
            renderMesh(*node.mesh);
        }

        bpstd::span<cgltf_node *> children{node.children, node.children_count};

        for (const cgltf_node *child : children) {
            if (child) {
                renderNode(transformation, *child);
            }
        }
    }

    void GltfRenderer::renderCamera(const cgltf_camera &camera) {
        switch (camera.type) {
        case cgltf_camera_type_perspective: {
            const cgltf_camera_perspective &pers = camera.data.perspective;
            projection = xe::mat4Perspective(pers.yfov, pers.aspect_ratio, pers.znear, pers.zfar);
            break;
        }

        case cgltf_camera_type_orthographic: {
            XE_LOG_ERROR("Orthographic camera is not supported.");
            break;
        }

        case cgltf_camera_type_invalid:
        case cgltf_camera_type_max_enum:
            XE_LOG_ERROR("Invalid camera type.");
            break;
        }
    }

    void GltfRenderer::renderMesh(const cgltf_mesh &cgltfMesh) {
        const auto it = meshMap.find(&cgltfMesh);

        if (it == meshMap.end()) {
            XE_LOG_WARNING("Mesh not found in meshMap.");
            return;
        }

        const GltfMesh &mesh = it->second;

        for (const auto &meshSubset : mesh.primitives) {
            const xe::gl::VertexArrayPrimitive prims[] = {{0, meshSubset.count}};

            xe::gl::TextureLayer layer;
            layer.texture = meshSubset.material.texture;
            renderer->bindRenderState(bpstd::span<const xe::gl::TextureLayer>(&layer, static_cast<size_t>(1)));

            if (meshSubset.indexData.has_value()) {
                renderer->draw(meshSubset.vao, meshSubset.primitive, prims, meshSubset.indexData->type);
            } else {
                renderer->draw(meshSubset.vao, meshSubset.primitive, prims);
            }
        }
    }
} // namespace xe::gltf_view
