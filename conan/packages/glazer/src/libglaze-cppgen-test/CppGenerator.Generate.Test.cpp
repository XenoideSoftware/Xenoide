#include "glaze/cppgen/CppGenerator.h"

#include "MiniRegistry.h"

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

using glaze::cppgen::CppGenerator;
using glaze::testfixtures::loadMiniRegistry;

TEST_CASE("CppGenerator produces three output files", "[glaze][cppgen][generate]") {
    const auto registry = loadMiniRegistry();
    CppGenerator gen{registry};
    const auto files = gen.generate("gl", "4.5");
    REQUIRE(files.size() == 3);
    REQUIRE(files.count("include/glaze/gl.hpp") == 1);
    REQUIRE(files.count("include/glaze/gl_handle.hpp") == 1);
    REQUIRE(files.count("include/glaze/raii.hpp") == 1);
}

TEST_CASE("Main gl.hpp contains the expected top-level structure",
          "[glaze][cppgen][generate]") {
    const auto registry = loadMiniRegistry();
    CppGenerator gen{registry};
    const auto files = gen.generate("gl", "4.5");
    const auto &hpp = files.at("include/glaze/gl.hpp");

    REQUIRE(hpp.find("namespace gl") != std::string::npos);
    REQUIRE(hpp.find("namespace dsa") != std::string::npos);
    REQUIRE(hpp.find("namespace glaze") != std::string::npos);
}

TEST_CASE("gl.hpp contains strong handle types discovered from MINI_XML",
          "[glaze][cppgen][handles]") {
    const auto registry = loadMiniRegistry();
    CppGenerator gen{registry};
    const auto files = gen.generate("gl", "4.5");
    const auto &hpp = files.at("include/glaze/gl.hpp");

    // Buffer, Program, Shader are the three handle classes the fixture exposes
    // via its first-param class_ attributes on DSA / delete commands.
    REQUIRE(hpp.find("struct Buffer") != std::string::npos);
    REQUIRE(hpp.find("struct Program") != std::string::npos);
    REQUIRE(hpp.find("struct Shader") != std::string::npos);
}

TEST_CASE("gl.hpp contains enum classes and the bitmask bridge",
          "[glaze][cppgen][enums]") {
    const auto registry = loadMiniRegistry();
    CppGenerator gen{registry};
    const auto files = gen.generate("gl", "4.5");
    const auto &hpp = files.at("include/glaze/gl.hpp");

    REQUIRE(hpp.find("enum class ClearBufferMask") != std::string::npos);
    REQUIRE(hpp.find("eColorBufferBit") != std::string::npos);
    // The ARB suffix on BufferTargetARB should get stripped away since no
    // collision exists in MINI_XML.
    REQUIRE(hpp.find("enum class BufferTarget") != std::string::npos);
}

TEST_CASE("gl.hpp contains functors that bridge to the C function pointers",
          "[glaze][cppgen][functors]") {
    const auto registry = loadMiniRegistry();
    CppGenerator gen{registry};
    const auto files = gen.generate("gl", "4.5");
    const auto &hpp = files.at("include/glaze/gl.hpp");

    REQUIRE(hpp.find("struct ClearFn") != std::string::npos);
    REQUIRE(hpp.find("glaze_glClear") != std::string::npos);
    REQUIRE(hpp.find("inline ClearFn clear") != std::string::npos);
}

TEST_CASE("gl.hpp emits DSA classes for Named* commands", "[glaze][cppgen][dsa]") {
    const auto registry = loadMiniRegistry();
    CppGenerator gen{registry};
    const auto files = gen.generate("gl", "4.5");
    const auto &hpp = files.at("include/glaze/gl.hpp");

    // glNamedBufferData is in MINI_XML at GL 4.5 — so we expect a dsa::Buffer
    // with a `data` method.
    REQUIRE(hpp.find("class Buffer") != std::string::npos);
}

TEST_CASE("gl_handle.hpp exposes the handle::* namespace", "[glaze][cppgen][handle]") {
    const auto registry = loadMiniRegistry();
    CppGenerator gen{registry};
    const auto files = gen.generate("gl", "4.5");
    const auto &handle = files.at("include/glaze/gl_handle.hpp");

    REQUIRE(handle.find("namespace handle") != std::string::npos);
    // glLinkProgram's first param has class="program" and the name does not
    // start with glNamed, so a legacy handle::Program::linkProgram method
    // should be synthesized.
    REQUIRE(handle.find("class Program") != std::string::npos);
}

TEST_CASE("raii.hpp is shipped verbatim with the expected header", "[glaze][cppgen][raii]") {
    const auto registry = loadMiniRegistry();
    CppGenerator gen{registry};
    const auto files = gen.generate("gl", "4.5");
    const auto &raii = files.at("include/glaze/raii.hpp");

    REQUIRE(raii.find("#pragma once") != std::string::npos);
    REQUIRE(raii.find("namespace glaze") != std::string::npos);
    REQUIRE(raii.find("Traits") != std::string::npos);
}

TEST_CASE("CustomNameTransform retargets emitted functor names",
          "[glaze][cppgen][extension]") {
    struct MyNames : public glaze::codegen::NameTransform {
        std::string transformCommandName(std::string_view glName) const override {
            // Prefix every functor name with "my_" to prove the extension
            // mechanism propagates into the generated output.
            return std::string{"my_"} +
                   glaze::codegen::DefaultNameTransform{}.transformCommandName(glName);
        }
    };

    const auto registry = loadMiniRegistry();
    CppGenerator gen{registry, std::make_shared<MyNames>()};
    const auto files = gen.generate("gl", "4.5");
    const auto &hpp = files.at("include/glaze/gl.hpp");
    REQUIRE(hpp.find("my_clear") != std::string::npos);
}

TEST_CASE("CppGenerator throws on unknown api/version", "[glaze][cppgen][errors]") {
    const auto registry = loadMiniRegistry();
    CppGenerator gen{registry};
    REQUIRE_THROWS_AS(gen.generate("gl", "99.99"), std::invalid_argument);
}
