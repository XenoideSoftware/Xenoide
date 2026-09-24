#include "glaze/codegen/TypeMapper.h"

#include <catch2/catch_test_macros.hpp>

using glaze::codegen::DefaultCTypeMapper;
using glaze::model::Command;
using glaze::model::CommandParam;
using glaze::model::TypeDecl;

namespace {

CommandParam makeParam(std::string name, std::vector<std::string> parts) {
    CommandParam p;
    p.name = std::move(name);
    p.typeParts = std::move(parts);
    return p;
}

Command makeCommand(std::string name, std::string returnType) {
    Command cmd;
    cmd.name = std::move(name);
    cmd.returnType = TypeDecl{"void", false, false};
    cmd.returnTypeStr = std::move(returnType);
    return cmd;
}

} // namespace

TEST_CASE("DefaultCTypeMapper joins typeParts and appends name",
          "[glaze][codegen][typemapper]") {
    DefaultCTypeMapper mapper;
    const auto cmd = makeCommand("glBindBuffer", "void");

    const auto scalar = makeParam("target", {"GLenum"});
    REQUIRE(mapper.mapParamDecl(scalar, cmd) == "GLenum target");

    const auto constPtr = makeParam("buffers", {"const", "GLuint", "*"});
    REQUIRE(mapper.mapParamDecl(constPtr, cmd) == "const GLuint* buffers");

    const auto ptr = makeParam("out", {"GLfloat", "*"});
    REQUIRE(mapper.mapParamDecl(ptr, cmd) == "GLfloat* out");
}

TEST_CASE("DefaultCTypeMapper::mapReturnType returns the raw returnTypeStr",
          "[glaze][codegen][typemapper]") {
    DefaultCTypeMapper mapper;
    const auto cmd = makeCommand("glGetString", "const GLubyte *");
    REQUIRE(mapper.mapReturnType(cmd) == "const GLubyte *");
}

TEST_CASE("DefaultCTypeMapper::mapBaseType is identity", "[glaze][codegen][typemapper]") {
    DefaultCTypeMapper mapper;
    REQUIRE(mapper.mapBaseType("GLuint") == "GLuint");
    REQUIRE(mapper.mapBaseType("GLenum") == "GLenum");
}
