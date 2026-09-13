
#include "GltfDataLoader.h"

#include <bpstd/span.hpp>

#include <fmt/format.h>

#include "xe/Logger.h"

GltfTextureLoader::GltfTextureLoader(const xe::gl::RendererGL *renderer, ImageLoader *imageLoader) : renderer(renderer), imageLoader(imageLoader) {
}

inline std::optional<GLenum> mapBppToFormat(const int bpp) {
    switch (bpp) {
    case 24:
        return GL_BGR;
    case 32:
        return GL_BGRA;
    default:
        return {};
    }
}

inline std::optional<GLenum> mapBppToInternalFormat(const int bpp) {
    switch (bpp) {
    case 24:
        return GL_RGB;
    case 32:
        return GL_RGBA;
    default:
        return {};
    }
}

xe::gl::Texture GltfTextureLoader::createTexture(const cgltf_texture_view &textureView) const {
    if (!textureView.texture) {
        xe::logWarning(fmt::format("Texture does not contain data"));
        return {};
    }

    xe::logInfo(fmt::format("Loading texture"));

    const auto mimeType = textureView.texture->image->mime_type;
    if (!mimeType) {
        xe::logWarning(fmt::format("Texture MIME type is null"));
        return {};
    }

    const auto imageFormat = parseImageFormat(mimeType);
    if (!imageFormat.has_value()) {
        xe::logWarning(fmt::format("Failed to parse Texture image format from MIME type {}", to_string(mimeType).value_or("<noMimeType>")));
        return {};
    }

    const auto buffer = textureView.texture->image->buffer_view->buffer->data;
    const auto offset = textureView.texture->image->buffer_view->offset;
    const auto size = textureView.texture->image->buffer_view->size;

    auto image = imageLoader->loadImage(addPointerOffset(buffer, offset), size, imageFormat.value());
    auto imageData = image->getData();

    const std::optional<GLenum> internalFormat = mapBppToInternalFormat(imageData.bpp);
    const std::optional<GLenum> format = mapBppToFormat(imageData.bpp);

    if (!internalFormat.has_value() || !format.has_value()) {
        XE_LOG_ERROR("TextureRepository::createTexture: Unsupported image bpp {}. Defaulting to GL_RGB\n", imageData.bpp);
        return {};
    }

    auto clientImage = xe::gl::ClientTextureImage2D{{imageData.width, imageData.height}, *format, GL_UNSIGNED_BYTE, imageData.pixels};

    xe::gl::CreateTextureOptions options;
    options.flags = xe::gl::GenerateMipMaps;

    std::vector<xe::gl::TextureParameter> parameters;
    if (auto *sampler = textureView.texture->sampler; sampler) {
        parameters.push_back({GL_TEXTURE_MAG_FILTER, sampler->mag_filter});
        parameters.push_back({GL_TEXTURE_MIN_FILTER, sampler->min_filter});
        parameters.push_back({GL_TEXTURE_WRAP_S, sampler->wrap_s});
        parameters.push_back({GL_TEXTURE_WRAP_T, sampler->wrap_t});
        options.parameters = bpstd::span<const xe::gl::TextureParameter>(parameters.data(), parameters.size());
    }

    return renderer->createTexture(GL_TEXTURE_2D, *internalFormat, clientImage, options);
}

cgltf_data *GltfDataParser::parse(const std::string &filePath) const {
    cgltf_data *data = nullptr;
    const auto filePathCstr = filePath.c_str();

    if (auto result = cgltf_parse_file(&options, filePathCstr, &data); result != cgltf_result_success) {
        std::cerr << "CGLTF: Couldn't load file '" << filePath << "'. Error code: " << to_string(result);
        return {};
    }

    if (auto result = cgltf_load_buffers(&options, data, filePathCstr); result != cgltf_result_success) {
        std::cerr << "CGLTF: error while loading buffers '" << filePath << "'. Error code: " << to_string(result);
        return {};
    }

    return data;
}

GltfDataLoader::GltfDataLoader(cgltf_data *data, xe::gl::RendererGL *renderer, GltfTextureLoader *textureLoader, xe::gl::Program program, const GltfAttributeMap &attributeMap)
    : data(data),
      renderer(renderer),
      textureLoader(textureLoader),
      program(program),
      attributeMap(attributeMap) {
}

std::vector<GltfMesh> GltfDataLoader::loadAllMeshes() {
    std::vector<GltfMesh> meshes;
    meshes.reserve(data->meshes_count);

    for (cgltf_size i = 0; i < data->meshes_count; i++) {
        const auto mesh = createMesh(data->meshes + i);

        if (mesh.primitives.empty()) {
            std::cerr << "Could not create mesh" << '\n';
            return {};
        }

        meshes.push_back(mesh);
    }

    return meshes;
}

void process_animation(cgltf_animation *animation) {
    std::cout << "Animation name: " << evaluate_name(animation->name) << '\n';
    std::cout << "Animation samplers count: " << animation->samplers_count << '\n';
    std::cout << "Animation channels count: " << animation->channels_count << '\n';
    std::cout << "Animation extensions count: " << animation->extensions_count << '\n';

    for (cgltf_size i = 0; i < animation->samplers_count; i++) {
        const auto sampler = animation->samplers + i;
        std::cout << "Animation Sampler Intepolation Type " << sampler->interpolation << '\n';
    }
    std::cout << '\n';

    for (cgltf_size i = 0; i < animation->channels_count; i++) {
        const auto channel = animation->channels + i;
        std::cout << "Animation Channel Target Path " << channel->target_path << '\n';
    }
    std::cout << '\n';
}

void GltfDataLoader::loadAllAnimations() {
    std::cout << "Found " << data->animations_count << " animations" << '\n';
    for (cgltf_size i = 0; i < data->animations_count; i++) {
        process_animation(data->animations + i);
        std::cout << '\n';
    }
    std::cout << '\n';
}

GltfMeshPrimitive GltfDataLoader::createMeshPrimitive(const cgltf_primitive &primitive) {
    const GLenum primitiveType = mapToPrimitive(primitive.type);
    const xe::gl::Buffer vertexBuffer = createVertexBuffer(primitive);

    std::optional<GltfIndexData> indexData;

    if (primitive.indices) {
        const GLenum indexType = mapToGLDataType(primitive.indices->component_type).value();
        const xe::gl::Buffer indexBuffer = createIndexBuffer(*primitive.indices);
        indexData = GltfIndexData{indexBuffer, indexType};
    }

    // FIXME: Assuming that all of the attributes are referencing the same count of vertices
    const auto count = static_cast<GLsizei>(primitive.indices ? primitive.indices->count : primitive.attributes[0].data->count);
    const auto vao = createVertexArray(primitive, vertexBuffer, indexData ? indexData->buffer : xe::gl::Buffer());

    if (!vao.id) {
        std::cerr << "Could not create vertex array." << '\n';
        return {};
    }

    GltfMeshPrimitive meshPrimitive;
    meshPrimitive.primitive = primitiveType;
    meshPrimitive.vertexBuffer = vertexBuffer;
    meshPrimitive.vao = vao;
    meshPrimitive.count = count;

    if (primitive.material) {
        meshPrimitive.material.texture = textureLoader->createTexture(primitive.material->pbr_metallic_roughness.base_color_texture);
    }

    meshPrimitive.indexData = indexData;

    return meshPrimitive;
}

// hola laurita
// eres mi hermosa capi <3
GltfMesh GltfDataLoader::createMesh(const cgltf_mesh *mesh) {
    GltfMesh result;

    result.name = sanitizeString(mesh->name);

    const bpstd::span<cgltf_primitive> meshPrimitives = {mesh->primitives, mesh->primitives_count};
    for (const cgltf_primitive &primitive : meshPrimitives) {
        const auto meshPrimitive = createMeshPrimitive(primitive);

        if (!meshPrimitive.vao.id) {
            std::cerr << "Could not create mesh primitive array." << '\n';
            return {};
        }

        result.primitives.push_back(meshPrimitive);
    }

    return result;
}

xe::gl::Buffer GltfDataLoader::createIndexBuffer(const cgltf_accessor &accessor) {
    const auto indicesOffset = accessor.buffer_view->offset;
    const auto indicesSize = accessor.buffer_view->size;
    const auto indexPtr = addPointerOffset(accessor.buffer_view->buffer->data, indicesOffset);

    return renderer->createBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, {indexPtr, indicesSize});
}

xe::gl::Buffer GltfDataLoader::createVertexBuffer(const cgltf_primitive &primitive) {
    // FIXME: Assuming that the attributes are stored in different regions of the same buffer

    const auto lastAttribIndex = primitive.attributes_count - 1;
    const auto vertexOffset = primitive.attributes[0].data->buffer_view->offset;
    const auto vertexSize = primitive.attributes[lastAttribIndex].data->buffer_view->offset;
    const auto vertexBufferView = primitive.attributes[0].data->buffer_view;

    const auto vertexPtr = addPointerOffset(vertexBufferView->buffer->data, vertexOffset);

    return renderer->createBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, {vertexPtr, vertexSize});
}

GLint GltfDataLoader::computeAttributeLocation(const std::string &gltfAttributeName) {
    const auto it = attributeMap.find(gltfAttributeName);
    if (it == attributeMap.end()) {
        std::cout << "Ignoring unused gltf attribute '" << gltfAttributeName << '\n';
        return -1;
    }

    const ShaderAttrib &shaderAttrib = it->second;
    const auto location = program.getAttribLocation(shaderAttrib.name.c_str());

    if (shaderAttrib.required && location == -1) {
        const auto msg = "Shader Attribute '" + shaderAttrib.name + "' does not exists";
        std::cerr << msg << '\n';
        throw std::runtime_error(msg);
    }

    return location;
}

xe::gl::VertexArray GltfDataLoader::createVertexArray(const cgltf_primitive &primitive, xe::gl::Buffer vertexBuffer, xe::gl::Buffer indexBuffer) {
    assert(vertexBuffer.id);

    std::vector<xe::gl::Attribute> attributesGL;

    const bpstd::span<cgltf_attribute> attributes = {primitive.attributes, primitive.attributes_count};
    for (const cgltf_attribute &attribute : attributes) {
        const auto &accessor = *attribute.data;
        const auto &bufferView = *accessor.buffer_view;

        auto dataTypeGL = mapToAttributeDataType(accessor.component_type);
        if (!dataTypeGL) {
            xe::logError(
                fmt::format("Could not map attribute {} with accessor component type {}", to_string(accessor.name).value_or("<noname>"), to_string(accessor.component_type))
            );
            return {};
        }

        auto attribDimGL = mapToAttribDim(accessor.type);
        if (!attribDimGL) {
            xe::logError(fmt::format("Could not map attribute {} with accessor type {}", to_string(accessor.name).value_or("<noname>"), to_string(accessor.type)));
            return {};
        }

        const auto location = computeAttributeLocation(attribute.name);

        if (location != -1) {
            xe::gl::Attribute attributeGL;
            attributeGL.index = location;
            attributeGL.offset = static_cast<GLuint>(bufferView.offset);
            attributeGL.type = dataTypeGL.value();
            attributeGL.stride = static_cast<GLsizei>(bufferView.stride);
            attributeGL.normalized = accessor.normalized ? GL_TRUE : GL_FALSE;
            attributeGL.buffer = vertexBuffer;
            attributeGL.size = attribDimGL.value();
            attributesGL.push_back(attributeGL);
        }
    }

    return renderer->createVertexArray(bpstd::span<const xe::gl::Attribute>(attributesGL.data(), attributesGL.size()), indexBuffer);
}
