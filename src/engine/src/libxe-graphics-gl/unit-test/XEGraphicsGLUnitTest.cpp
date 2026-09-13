
#include <catch2/catch_test_macros.hpp>

#include <xe/DataType.h>
#include <xe/graphics/BufferDescriptor.h>
#include <xe/graphics/Material.h>
#include <xe/graphics/PixelFormat.h>
#include <xe/graphics/Subset.h>
#include <xe/graphics/Uniform.h>
#include <xe/graphics/RenderBackend.h>
/*
#include <xe/graphics/gl3/glcore3-api.h>

TEST_CASE("BufferType::Uniform is a distinct enumerant usable alongside Vertex and Index") {
    xe::BufferType const vertex = xe::BufferType::Vertex;
    xe::BufferType const index = xe::BufferType::Index;
    xe::BufferType const uniform = xe::BufferType::Uniform;

    REQUIRE(vertex != uniform);
    REQUIRE(index != uniform);
    REQUIRE(static_cast<int>(uniform) != static_cast<int>(vertex));
}

TEST_CASE("UniformLocation default-constructs to the invalid sentinel") {
    xe::UniformLocation loc;

    REQUIRE_FALSE(loc.isValid());
    REQUIRE(loc.raw == -1);
    REQUIRE(loc.programKey == 0u);
}

TEST_CASE("UniformLocation with a non-negative raw value reports isValid()") {
    xe::UniformLocation loc;
    loc.raw = 0;
    loc.programKey = 0xDEADBEEFu;

    REQUIRE(loc.isValid());
    REQUIRE(loc.programKey == 0xDEADBEEFu);
}

TEST_CASE("UniformValueSubmission defaults to a safe, empty shape") {
    xe::UniformValueSubmission sub;

    REQUIRE_FALSE(sub.location.isValid());
    REQUIRE(sub.type == xe::UniformVectorType::Float1);
    REQUIRE(xe::getTypeKind(static_cast<xe::TypeEncoding>(sub.type)) == xe::TypeKind::Float);
    REQUIRE(xe::getTypeCols(static_cast<xe::TypeEncoding>(sub.type)) == 1u);
    REQUIRE(sub.count == 1u);
    REQUIRE(sub.data == nullptr);
}

TEST_CASE("UniformMatrixSubmission defaults to a 4x4 float matrix with transpose off") {
    xe::UniformMatrixSubmission sub;

    REQUIRE_FALSE(sub.location.isValid());
    REQUIRE(sub.shape == xe::UniformMatrixShape::R4C4);
    REQUIRE(xe::getTypeCols(static_cast<xe::TypeEncoding>(sub.shape)) == 4u);
    REQUIRE(xe::getTypeRows(static_cast<xe::TypeEncoding>(sub.shape)) == 4u);
    REQUIRE(sub.count == 1u);
    REQUIRE_FALSE(sub.transpose);
    REQUIRE(sub.data == nullptr);
}

TEST_CASE("PipelineDescriptor carries an (initially empty) uniform block list") {
    xe::PipelineDescriptor desc;

    REQUIRE(desc.uniformBlocks.empty());

    desc.uniformBlocks.push_back({"CameraBlock", 0u});
    desc.uniformBlocks.push_back({"MaterialBlock", 1u});

    REQUIRE(desc.uniformBlocks.size() == 2u);
    REQUIRE(desc.uniformBlocks[0].blockName == "CameraBlock");
    REQUIRE(desc.uniformBlocks[1].bindingPoint == 1u);
}

TEST_CASE("RenderDeviceBackendVTable default-constructs every slot to nullptr, including the uniform slots") {
    xe::RenderDeviceBackendVTable vt;

    REQUIRE(vt.createBuffer == nullptr);
    REQUIRE(vt.createShaderProgram == nullptr);
    REQUIRE(vt.createPipeline == nullptr);
    REQUIRE(vt.destroyPipeline == nullptr);
    REQUIRE(vt.resolveUniformLocation == nullptr);
    REQUIRE(vt.applyUniforms == nullptr);
    REQUIRE(vt.bindUniformBuffer == nullptr);
}

TEST_CASE("initializeBackendTableGL installs every slot, uniform path included") {
    xe::RenderDeviceBackendVTable vt;
    xe::initializeBackendTableGL(&vt);

    REQUIRE(vt.createBuffer != nullptr);
    REQUIRE(vt.destroyBuffer != nullptr);
    REQUIRE(vt.createShaderProgram != nullptr);
    REQUIRE(vt.createTexture != nullptr);
    REQUIRE(vt.createVertexLayout != nullptr);
    REQUIRE(vt.createGeometry != nullptr);
    REQUIRE(vt.createPipeline != nullptr);
    REQUIRE(vt.destroyPipeline != nullptr);
    REQUIRE(vt.resolveUniformLocation != nullptr);
    REQUIRE(vt.applyUniforms != nullptr);
    REQUIRE(vt.bindUniformBuffer != nullptr);
}
*/
