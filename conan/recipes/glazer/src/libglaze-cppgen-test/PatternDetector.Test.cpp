#include "glaze/cppgen/detail/PatternDetector.h"

#include "glaze/model/Command.h"
#include "glaze/model/CommandParam.h"

#include <catch2/catch_test_macros.hpp>

using namespace glaze::cppgen::detail;
using glaze::model::Command;
using glaze::model::CommandParam;
using glaze::model::TypeDecl;

namespace {

//! Tiny builder so each test case reads as a single declarative statement.
struct ParamBuilder {
    CommandParam p;
    ParamBuilder(std::string name) { p.name = std::move(name); }
    ParamBuilder &type(std::initializer_list<std::string> parts) {
        p.typeParts = parts;
        return *this;
    }
    ParamBuilder &len(std::string v) {
        p.len = std::move(v);
        return *this;
    }
    ParamBuilder &classStr(std::string v) {
        p.classStr = std::move(v);
        return *this;
    }
    ParamBuilder &group(std::string v) {
        p.group = std::move(v);
        return *this;
    }
    CommandParam build() const { return p; }
};

Command makeCommand(std::string name, std::vector<CommandParam> params,
                    TypeDecl ret = {}) {
    Command c;
    c.name = std::move(name);
    c.params = std::move(params);
    c.returnType = std::move(ret);
    return c;
}

TypeDecl voidType() {
    TypeDecl t;
    t.name = "void";
    return t;
}

} // namespace

TEST_CASE("scalarQueryTypes has the seven numeric scalar types",
          "[glaze][cppgen][patterns]") {
    const auto &types = scalarQueryTypes();
    REQUIRE(types.size() == 7);
    REQUIRE(types.count("GLint") == 1);
    REQUIRE(types.count("GLuint") == 1);
    REQUIRE(types.count("GLfloat") == 1);
    REQUIRE(types.count("GLdouble") == 1);
    REQUIRE(types.count("GLboolean") == 1);
    REQUIRE(types.count("GLint64") == 1);
    REQUIRE(types.count("GLuint64") == 1);
}

TEST_CASE("infoLogSelfQueryMap maps InfoLog commands to their iv query",
          "[glaze][cppgen][patterns]") {
    const auto &map = infoLogSelfQueryMap();
    REQUIRE(map.at("glGetShaderInfoLog") == "::glGetShaderiv");
    REQUIRE(map.at("glGetProgramInfoLog") == "::glGetProgramiv");
}

TEST_CASE("findScalarQueryParam matches glGetShaderiv", "[glaze][cppgen][patterns]") {
    const auto cmd = makeCommand(
        "glGetShaderiv",
        {
            ParamBuilder{"shader"}.type({"GLuint"}).classStr("shader").build(),
            ParamBuilder{"pname"}.type({"GLenum"}).group("ShaderParameterName").build(),
            ParamBuilder{"params"}.type({"GLint", "*"}).build(),
        },
        voidType());

    const auto *scalar = findScalarQueryParam(cmd);
    REQUIRE(scalar != nullptr);
    REQUIRE(scalar->name == "params");
}

TEST_CASE("findScalarQueryParam rejects glGetIntegerv (no handle first-param)",
          "[glaze][cppgen][patterns]") {
    const auto cmd = makeCommand(
        "glGetIntegerv",
        {
            ParamBuilder{"pname"}.type({"GLenum"}).group("GetPName").build(),
            ParamBuilder{"data"}.type({"GLint", "*"}).build(),
        },
        voidType());
    REQUIRE(findScalarQueryParam(cmd) == nullptr);
}

TEST_CASE("findScalarQueryParam rejects commands with multiple out-pointers",
          "[glaze][cppgen][patterns]") {
    const auto cmd = makeCommand(
        "glGetProgramBinary",
        {
            ParamBuilder{"program"}.type({"GLuint"}).classStr("program").build(),
            ParamBuilder{"bufSize"}.type({"GLsizei"}).build(),
            ParamBuilder{"length"}.type({"GLsizei", "*"}).len("1").build(),
            ParamBuilder{"binaryFormat"}.type({"GLenum", "*"}).build(),
            ParamBuilder{"binary"}.type({"void", "*"}).build(),
        },
        voidType());
    REQUIRE(findScalarQueryParam(cmd) == nullptr);
}

TEST_CASE("findStringOutputParam matches infoLog buffers",
          "[glaze][cppgen][patterns]") {
    const auto cmd = makeCommand(
        "glGetShaderInfoLog",
        {
            ParamBuilder{"shader"}.type({"GLuint"}).classStr("shader").build(),
            ParamBuilder{"bufSize"}.type({"GLsizei"}).build(),
            ParamBuilder{"length"}.type({"GLsizei", "*"}).len("1").build(),
            ParamBuilder{"infoLog"}.type({"GLchar", "*"}).len("bufSize").build(),
        },
        voidType());
    const auto *str = findStringOutputParam(cmd);
    REQUIRE(str != nullptr);
    REQUIRE(str->name == "infoLog");
}

TEST_CASE("findStringOutputParam rejects const GLchar* parameters",
          "[glaze][cppgen][patterns]") {
    const auto cmd = makeCommand(
        "glShaderSource",
        {
            ParamBuilder{"shader"}.type({"GLuint"}).classStr("shader").build(),
            ParamBuilder{"count"}.type({"GLsizei"}).build(),
            ParamBuilder{"string"}.type({"const", "GLchar", "*", "*"}).len("count").build(),
        },
        voidType());
    REQUIRE(findStringOutputParam(cmd) == nullptr);
}

TEST_CASE("findLengthParam matches a GLsizei* len='1' output sink",
          "[glaze][cppgen][patterns]") {
    const auto cmd = makeCommand(
        "glGetShaderInfoLog",
        {
            ParamBuilder{"shader"}.type({"GLuint"}).classStr("shader").build(),
            ParamBuilder{"bufSize"}.type({"GLsizei"}).build(),
            ParamBuilder{"length"}.type({"GLsizei", "*"}).len("1").build(),
            ParamBuilder{"infoLog"}.type({"GLchar", "*"}).len("bufSize").build(),
        },
        voidType());
    const auto *lp = findLengthParam(cmd);
    REQUIRE(lp != nullptr);
    REQUIRE(lp->name == "length");
}

TEST_CASE("findDataUploadParams matches glBufferData-style uploads",
          "[glaze][cppgen][patterns]") {
    const auto cmd = makeCommand(
        "glBufferData",
        {
            ParamBuilder{"target"}.type({"GLenum"}).group("BufferTarget").build(),
            ParamBuilder{"size"}.type({"GLsizeiptr"}).build(),
            ParamBuilder{"data"}.type({"const", "void", "*"}).len("size").build(),
            ParamBuilder{"usage"}.type({"GLenum"}).group("BufferUsage").build(),
        },
        voidType());
    const auto pair = findDataUploadParams(cmd);
    REQUIRE(pair.has_value());
    REQUIRE(pair->data->name == "data");
    REQUIRE(pair->size->name == "size");
}

TEST_CASE("findDataUploadParams rejects commands without a const void* data param",
          "[glaze][cppgen][patterns]") {
    const auto cmd = makeCommand(
        "glDrawArrays",
        {
            ParamBuilder{"mode"}.type({"GLenum"}).build(),
            ParamBuilder{"first"}.type({"GLint"}).build(),
            ParamBuilder{"count"}.type({"GLsizei"}).build(),
        },
        voidType());
    REQUIRE(!findDataUploadParams(cmd).has_value());
}

TEST_CASE("isInfoLogCommand matches only the two known info-log queries",
          "[glaze][cppgen][patterns]") {
    Command shader;
    shader.name = "glGetShaderInfoLog";
    Command program;
    program.name = "glGetProgramInfoLog";
    Command other;
    other.name = "glGetShaderiv";

    REQUIRE(isInfoLogCommand(shader));
    REQUIRE(isInfoLogCommand(program));
    REQUIRE_FALSE(isInfoLogCommand(other));
}
