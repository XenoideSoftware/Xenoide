
#include <glaze/gl.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <vector>

#include "xe/graphics/RenderBackend.h"
#include "xe/graphics/gl3/glcore3-api.h"

// TODO: The quick initial image generation infra needs to be refactored later
static float checkerboardValue(int x, int y, int tileSize) {
    return (((x / tileSize) + (y / tileSize)) & 1) ? 0.0f : 1.0f;
}

static int channelCountOf(xe::PixelFormat format) {
    switch (format) {
    case xe::PixelFormat::R8G8B8:
        return 3;
    case xe::PixelFormat::R8G8B8A8:
        return 4;
    default:
        return 0;
    }
}

static size_t byteSizeOf(xe::PixelDataType type) {
    return xe::toBytes(xe::getElementSize(static_cast<xe::TypeEncoding>(type)));
}

static void writeNormalizedChannel(void *base, size_t pixelOffset, int channel, float value, xe::PixelDataType type) {
    auto *byteBase = static_cast<uint8_t *>(base) + pixelOffset;
    switch (type) {
    case xe::PixelDataType::UInt8:
        byteBase[channel] = static_cast<uint8_t>(value * 255.0f);
        break;
    case xe::PixelDataType::UInt16:
        reinterpret_cast<uint16_t *>(byteBase)[channel] = static_cast<uint16_t>(value * 65535.0f);
        break;
    case xe::PixelDataType::Float32:
        reinterpret_cast<float *>(byteBase)[channel] = value;
        break;
    default:
        assert(false && "writeNormalizedChannel: unsupported PixelDataType");
    }
}

static void fillCheckerboardImage(void *data, size_t byteSize, int width, int height, xe::PixelFormat format, xe::PixelDataType dataType, int tileSize) {
    int const channels = channelCountOf(format);
    size_t const channelBytes = byteSizeOf(dataType);
    size_t const pixelStride = static_cast<size_t>(channels) * channelBytes;

    assert(data != nullptr);
    assert(channels > 0 && channelBytes > 0);
    assert(byteSize >= static_cast<size_t>(width) * height * pixelStride);
    (void)byteSize;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float const v = checkerboardValue(x, y, tileSize);
            size_t const pixelOffset = (static_cast<size_t>(y) * width + x) * pixelStride;
            for (int c = 0; c < channels; ++c) {
                float const out = (c == 3) ? 1.0f : v;
                writeNormalizedChannel(data, pixelOffset, c, out, dataType);
            }
        }
    }
}

tl::expected<xe::TextureHandle, xe::BackendError> createCheckerBoardTexture(const xe::RenderDeviceBackendVTable &vtable, xe::RenderDeviceBackendContext *ctx, xe::Vector2i size) {
    constexpr int tileSize = 64;
    constexpr xe::PixelFormat format = xe::PixelFormat::R8G8B8A8;
    constexpr xe::PixelDataType dataType = xe::PixelDataType::UInt8;

    size_t const byteSize = static_cast<size_t>(size.x) * static_cast<size_t>(size.y) * 4u;
    std::vector<uint8_t> pixels(byteSize);
    fillCheckerboardImage(pixels.data(), byteSize, size.x, size.y, format, dataType, tileSize);

    xe::MipLevel const mip{pixels.data()};

    xe::TextureDescriptor desc{};
    desc.type = xe::TextureType::Tex2D;
    desc.format = format;
    desc.size = xe::Vector3i(size.x, size.y, 1);
    desc.sourceFormat = format;
    desc.sourceDataType = dataType;
    desc.mipLevels = &mip;
    desc.mipLevelCount = 1;
    desc.generateMipmaps = true;

    return vtable.createTexture(ctx, desc);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "render backend test", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glazeLoadFunctions(glfwGetProcAddress);

    xe::RenderDeviceBackendVTable vtable;
    xe::initializeBackendTableGL(&vtable);

    auto ctxResult = vtable.createContext();
    if (!ctxResult) {
        std::cerr << "createContext failed: " << ctxResult.error().message << std::endl;
        return 1;
    }
    xe::RenderDeviceBackendContext *ctx = *ctxResult;

    // shader initialization
    xe::ShaderProgramDescriptor shaderDesc;
    shaderDesc.glslVertexShader = R"(
#version 330 core
layout(location = 0) in vec3 position; 
layout(location = 1) in vec2 texCoord;

out vec2 fragTexCoord;

void main() {
    gl_Position = vec4(position, 1.0);
    fragTexCoord = texCoord;
}
)";

    shaderDesc.glslFragmentShader = R"(
#version 330 core

in vec2 fragTexCoord;

uniform sampler2D uTexture;

out vec4 fragColor;
void main() {
    vec4 color = texture(uTexture, fragTexCoord);
    fragColor = color;
}
)";

    auto shaderResult = vtable.createShaderProgram(ctx, shaderDesc);
    if (!shaderResult) {
        std::cerr << "createShaderProgram failed: " << shaderResult.error().message << std::endl;
        return 1;
    }
    xe::ShaderHandle shaderHandle = *shaderResult;

    // texture generation
    auto textureResult = createCheckerBoardTexture(vtable, ctx, {512, 512});
    if (!textureResult) {
        std::cerr << "createCheckerBoardTexture failed: " << textureResult.error().message << std::endl;
        return 1;
    }
    xe::TextureHandle textureHandle = *textureResult;

    // vertex buffer initialization
    xe::Vector3 const verts[] = {{-0.5f, 0.5f, 0.0f}, {0.5f, 0.5f, 0.0f}, {-0.5f, -0.5, 0.0f}, {0.5, -0.5, 0.0}};
    xe::Vector2 const texCoords[] = {{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}};

    xe::BufferDescriptor bufferDesc{};
    bufferDesc.type = xe::BufferType::Vertex;
    bufferDesc.usage = xe::BufferUsage::DynamicDraw;
    bufferDesc.data = verts[0].data();
    bufferDesc.size = sizeof(verts);
    auto vertexBufferResult = vtable.createBuffer(ctx, bufferDesc);
    if (!vertexBufferResult) {
        std::cerr << "createBuffer (vertex) failed: " << vertexBufferResult.error().message << std::endl;
        return 1;
    }
    xe::BufferHandle vertexBuffer = *vertexBufferResult;

    bufferDesc.data = texCoords[0].data();
    bufferDesc.size = sizeof(texCoords);
    auto texCoordBufferResult = vtable.createBuffer(ctx, bufferDesc);
    if (!texCoordBufferResult) {
        std::cerr << "createBuffer (texCoord) failed: " << texCoordBufferResult.error().message << std::endl;
        return 1;
    }
    xe::BufferHandle texCoordBuffer = *texCoordBufferResult;

    // Vertex Layout initialization
    xe::VertexLayoutDescriptor layoutDesc;
    layoutDesc.resolveMode = xe::VertexLayoutResolveMode::Explicit;
    layoutDesc.attribs = {{0, xe::VertexAttribFormat::float3, false}, {1, xe::VertexAttribFormat::float2, false}};

    auto layoutResult = vtable.createVertexLayout(ctx, layoutDesc);
    if (!layoutResult) {
        std::cerr << "createGeometry failed: " << layoutResult.error().message << std::endl;
        return 1;
    }

    xe::VertexLayoutHandle layout = *layoutResult;

    // Geometry (enriched VAO wrapper)
    xe::GeometryDescriptor geometryDesc;
    geometryDesc.layoutHandle = layout;
    geometryDesc.bufferAttribs = {
        xe::GeometryBufferAttrib{vertexBuffer, 0},
        xe::GeometryBufferAttrib{texCoordBuffer, 1},
    };

    auto geometryResult = vtable.createGeometry(ctx, geometryDesc);
    if (!geometryResult) {
        std::cerr << "createGeometry failed: " << geometryResult.error().message << std::endl;
        return 1;
    }

    xe::GeometryHandle geometryHandle = *geometryResult;

    auto glctxgl = static_cast<xe::RenderDeviceBackendContextGL *>(ctx);
    gl::Program const programId = glctxgl->shaderPrograms[shaderHandle.index()].obj->program.get();
    gl::VertexArray const vao = glctxgl->geometries[geometryHandle.index()].obj->vao.get();
    gl::Texture const textureId = glctxgl->textures[textureHandle.index()].obj->texture.get();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);

        gl::clear(gl::ClearBufferMask::eColorBufferBit);

        gl::viewport(0, 0, w, h);

        gl::useProgram(programId);

        gl::UniformLocation textureLoc = gl::getUniformLocation(programId, "uTexture");

        gl::activeTexture(gl::TextureUnit::eTexture0);
        gl::bindTexture(gl::TextureTarget::eTexture2d, textureId);
        gl::uniform1i(textureLoc, 0);

        gl::bindVertexArray(vao);
        gl::drawArrays(gl::PrimitiveType::eTriangleStrip, 0, 4);

        gl::flush();

        glfwSwapBuffers(window);
    }

    vtable.destroyContext(ctx);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
