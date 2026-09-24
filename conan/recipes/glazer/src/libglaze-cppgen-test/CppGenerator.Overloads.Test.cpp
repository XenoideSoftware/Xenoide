#include "MiniRegistry.h"

#include "glaze/cppgen/CppGenerator.h"

#include <catch2/catch_test_macros.hpp>

#include <string>

using glaze::cppgen::CppGenerator;
using glaze::testfixtures::loadMiniRegistry;

namespace {

std::string generateGlHpp() {
    const auto registry = loadMiniRegistry();
    CppGenerator gen{registry};
    const auto files = gen.generate("gl", "4.5");
    return files.at("include/glaze/gl.hpp");
}

std::string generateGlHandleHpp() {
    const auto registry = loadMiniRegistry();
    CppGenerator gen{registry};
    const auto files = gen.generate("gl", "4.5");
    return files.at("include/glaze/gl_handle.hpp");
}

bool contains(const std::string &haystack, std::string_view needle) {
    return haystack.find(needle) != std::string::npos;
}

} // namespace

TEST_CASE("gl.hpp emits a template ArrayView overload for glBufferData uploads",
          "[glaze][cppgen][overloads]") {
    const auto hpp = generateGlHpp();
    REQUIRE(contains(hpp, "ArrayView"));
    REQUIRE(contains(hpp, "template <typename T>"));
    // The overload signature replaces the (GLsizeiptr size, const void *data)
    // pair with a single ArrayView parameter.
    REQUIRE(contains(hpp, "const ArrayView<T>& data"));
    REQUIRE(contains(hpp, "data.size_bytes()"));
    REQUIRE(contains(hpp, "data.data()"));
}

TEST_CASE("gl.hpp emits a scalar-query overload returning GLint for glGetShaderiv",
          "[glaze][cppgen][overloads]") {
    const auto hpp = generateGlHpp();
    // The scalar query overload returns GLint by value (it drops the trailing
    // GLint* params pointer from the signature). Look for the distinctive
    // "GLint result{}" prelude we emit.
    REQUIRE(contains(hpp, "GetShaderivFn"));
    REQUIRE(contains(hpp, "GLint result{}"));
    REQUIRE(contains(hpp, "::glGetShaderiv"));
}

TEST_CASE("gl.hpp emits a zero-arg info-log self-query overload",
          "[glaze][cppgen][overloads]") {
    const auto hpp = generateGlHpp();
    // The InfoLog self-query overload queries GL_INFO_LOG_LENGTH internally
    // via the paired Get*iv function and returns std::string.
    REQUIRE(contains(hpp, "GetShaderInfoLogFn"));
    REQUIRE(contains(hpp, "GL_INFO_LOG_LENGTH"));
    REQUIRE(contains(hpp, "::glGetShaderiv"));
    REQUIRE(contains(hpp, "std::string result"));
}

TEST_CASE("gl.hpp emits a std::string-returning overload for GLchar* output buffers",
          "[glaze][cppgen][overloads]") {
    const auto hpp = generateGlHpp();
    // The canonical (raw pointer) overload AND the string overload coexist.
    REQUIRE(contains(hpp, "GetShaderInfoLogFn"));
    // A std::string overload exists — look for a std::string return anchored
    // inside a `GetShaderInfoLog` context. The exact spelling lives in our
    // pre-built body string.
    const auto pos = hpp.find("GetShaderInfoLogFn");
    REQUIRE(pos != std::string::npos);
    // The struct body extends beyond the class header — look downstream for
    // the "result.resize" guard that proves the string overload is present.
    REQUIRE(hpp.find("result.resize", pos) != std::string::npos);
}

TEST_CASE("gl.hpp emits singular genBuffer/deleteBuffer helpers",
          "[glaze][cppgen][overloads]") {
    const auto hpp = generateGlHpp();
    REQUIRE(contains(hpp, "GenBuffersFn"));
    REQUIRE(contains(hpp, "DeleteBuffersFn"));
    // The singular create overload instantiates the handle wrapper locally
    // and returns it by value. The MiniRegistry has no name collision so
    // the handle class stays as `Buffer`; the real gl.xml renames it to
    // `BufferId` to avoid clashing with the `Buffer` enum group.
    REQUIRE(contains(hpp, "Buffer obj{}"));
    REQUIRE(contains(hpp, "return obj;"));
    // The singular delete overload takes the wrapper and calls the underlying
    // glDeleteBuffers with a pointer to the scalar.
    REQUIRE(contains(hpp, "reinterpret_cast<const GLuint*>(&obj)"));
}

TEST_CASE("gl_handle.hpp emits overloaded handle methods that delegate to functors",
          "[glaze][cppgen][overloads]") {
    const auto handle = generateGlHandleHpp();
    // handle::Shader has canonical + scalar + string + infolog-self-query
    // overloads for getShaderInfoLog. Verify the zero-arg variant (no params)
    // exists and delegates to ::gl::getShaderInfoLog(m_id).
    REQUIRE(contains(handle, "class Shader"));
    // The canonical getShaderiv takes the scalar; the scalar-query variant
    // has signature `GLint getShaderiv(GLenum pname) const`. Look for the
    // GLint return type on a getShaderiv method.
    REQUIRE(contains(handle, "GLint getShaderiv"));
    // The zero-arg info-log overload.
    REQUIRE(contains(handle, "std::string getShaderInfoLog() const"));
    REQUIRE(contains(handle, "::gl::getShaderInfoLog(m_id)"));
}
