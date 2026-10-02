#include "glaze/codegen/NameTransform.h"

#include <catch2/catch_test_macros.hpp>

#include <set>
#include <string>

using glaze::codegen::DefaultNameTransform;
using glaze::codegen::NameTransform;

TEST_CASE("DefaultNameTransform::transformCommandName strips gl prefix",
          "[glaze][codegen][name]") {
    DefaultNameTransform nt;
    REQUIRE(nt.transformCommandName("glClear") == "clear");
    REQUIRE(nt.transformCommandName("glBindBuffer") == "bindBuffer");
    REQUIRE(nt.transformCommandName("glDrawElementsInstanced") == "drawElementsInstanced");
}

TEST_CASE("DefaultNameTransform::transformCommandName passes through short non-gl names",
          "[glaze][codegen][name]") {
    DefaultNameTransform nt;
    REQUIRE(nt.transformCommandName("foo") == "foo");
    REQUIRE(nt.transformCommandName("") == "");
}

TEST_CASE("DefaultNameTransform::transformHandleTypeName camel-cases space-delimited classes",
          "[glaze][codegen][name]") {
    DefaultNameTransform nt;
    REQUIRE(nt.transformHandleTypeName("buffer") == "Buffer");
    REQUIRE(nt.transformHandleTypeName("program pipeline") == "ProgramPipeline");
    REQUIRE(nt.transformHandleTypeName("vertex array") == "VertexArray");
}

TEST_CASE("DefaultNameTransform::transformEnumEntry strips GL_ and camel-cases with e prefix",
          "[glaze][codegen][name]") {
    DefaultNameTransform nt;
    REQUIRE(nt.transformEnumEntry("ClearBufferMask", "GL_COLOR_BUFFER_BIT") == "eColorBufferBit");
    REQUIRE(nt.transformEnumEntry("PrimitiveType", "GL_TRIANGLES") == "eTriangles");
}

TEST_CASE("DefaultNameTransform::transformEnumGroupName strips vendor suffixes when safe",
          "[glaze][codegen][name]") {
    DefaultNameTransform nt;
    const std::set<std::string> groups{"BufferTargetARB", "FramebufferTarget"};
    REQUIRE(nt.transformEnumGroupName("BufferTargetARB", groups) == "BufferTarget");
    // No collision with any base name — FramebufferTarget carries no suffix.
    REQUIRE(nt.transformEnumGroupName("FramebufferTarget", groups) == "FramebufferTarget");
}

TEST_CASE("DefaultNameTransform::transformEnumGroupName keeps suffix on collision",
          "[glaze][codegen][name]") {
    DefaultNameTransform nt;
    const std::set<std::string> groups{"BufferTarget", "BufferTargetARB"};
    REQUIRE(nt.transformEnumGroupName("BufferTargetARB", groups) == "BufferTargetARB");
}

TEST_CASE("NameTransform base class forwards to default impl when not overridden",
          "[glaze][codegen][name]") {
    struct Custom : public NameTransform {
        std::string transformCommandName(std::string_view glName) const override {
            return "custom_" + std::string{glName};
        }
    };

    Custom c;
    REQUIRE(c.transformCommandName("glClear") == "custom_glClear");
    // Untouched methods still run the default behavior.
    REQUIRE(c.transformHandleTypeName("buffer") == "Buffer");
}
