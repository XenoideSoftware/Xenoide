#include "glaze/cgen/CGenerator.h"
#include "glaze/producer/RegistryLoader.h"

#include "MiniRegistry.h"

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

using glaze::cgen::CGenerator;
using glaze::testfixtures::loadMiniRegistry;

TEST_CASE("CGenerator produces gl.h and gl.c keys", "[glaze][cgen][generate]") {
    const auto registry = loadMiniRegistry();
    CGenerator gen{registry};
    const auto files = gen.generate("gl", "4.5");
    REQUIRE(files.size() == 2);
    REQUIRE(files.count("include/glaze/gl.h") == 1);
    REQUIRE(files.count("src/gl.c") == 1);
}

TEST_CASE("CGenerator header includes enum macros and command typedefs",
          "[glaze][cgen][generate]") {
    const auto registry = loadMiniRegistry();
    CGenerator gen{registry};
    const auto files = gen.generate("gl", "4.5");
    const auto &header = files.at("include/glaze/gl.h");

    REQUIRE(header.find("#define GL_COLOR_BUFFER_BIT 0x00004000") != std::string::npos);
    REQUIRE(header.find("PFNGLCLEARPROC") != std::string::npos);
    REQUIRE(header.find("glaze_glClear") != std::string::npos);
    REQUIRE(header.find("typedef void") != std::string::npos);
    REQUIRE(header.find("glazeLoadFunctions") != std::string::npos);
}

TEST_CASE("CGenerator source defines function pointers and the loader",
          "[glaze][cgen][generate]") {
    const auto registry = loadMiniRegistry();
    CGenerator gen{registry};
    const auto files = gen.generate("gl", "4.5");
    const auto &source = files.at("src/gl.c");

    REQUIRE(source.find("GLAZE_API PFNGLCLEARPROC glaze_glClear") != std::string::npos);
    REQUIRE(source.find("glazeLoadFunctions") != std::string::npos);
    REQUIRE(source.find("glazeLoadExtensions") != std::string::npos);
    REQUIRE(source.find("glazeHasExtension") != std::string::npos);
}

TEST_CASE("CGenerator opt-in extensions emit guard and flag",
          "[glaze][cgen][extensions]") {
    const auto registry = loadMiniRegistry();
    CGenerator gen{registry};
    const auto files = gen.generate("gl", "4.5", /*vendors*/ {"ARB"});
    const auto &header = files.at("include/glaze/gl.h");

    REQUIRE(header.find("GL_ARB_buffer_storage") != std::string::npos);
    REQUIRE(header.find("GLAZE_GL_NO_EXT_ARB_buffer_storage") != std::string::npos);
    REQUIRE(header.find("GLAZE_EXT_GL_ARB_buffer_storage") != std::string::npos);
}

TEST_CASE("CGenerator omits extensions when neither filter is set",
          "[glaze][cgen][extensions]") {
    const auto registry = loadMiniRegistry();
    CGenerator gen{registry};
    const auto files = gen.generate("gl", "4.5");
    const auto &header = files.at("include/glaze/gl.h");
    REQUIRE(header.find("GL_ARB_buffer_storage") == std::string::npos);
}

TEST_CASE("CGenerator throws on unknown api/version", "[glaze][cgen][errors]") {
    const auto registry = loadMiniRegistry();
    CGenerator gen{registry};
    REQUIRE_THROWS_AS(gen.generate("gl", "99.99"), std::invalid_argument);
    REQUIRE_THROWS_AS(gen.generate("not-a-real-api", "1.0"), std::invalid_argument);
}

TEST_CASE("CGenerator::validateExtensionFilters flags unknown vendors",
          "[glaze][cgen][validate]") {
    const auto registry = loadMiniRegistry();
    CGenerator gen{registry};
    const auto errors = gen.validateExtensionFilters("gl", {"FOO"}, {});
    REQUIRE_FALSE(errors.empty());
}
