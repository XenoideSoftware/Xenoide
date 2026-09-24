#include "MiniRegistry.h"

#include <algorithm>

#include <catch2/catch_test_macros.hpp>

using glaze::testfixtures::loadMiniRegistry;

TEST_CASE("Extension parser produces four extensions", "[glaze][producer][extensions]") {
    const auto registry = loadMiniRegistry();
    REQUIRE(registry.extensions().size() == 4);

    const auto has = [&](const std::string &name) {
        return std::any_of(registry.extensions().begin(), registry.extensions().end(),
                           [&](const auto &ext) { return ext.name == name; });
    };
    REQUIRE(has("GL_ARB_buffer_storage"));
    REQUIRE(has("GL_ARB_draw_instanced"));
    REQUIRE(has("GL_NV_shader_buffer_load"));
    REQUIRE(has("GL_KHR_debug"));
}

TEST_CASE("Extension parser splits 'supported' on the pipe delimiter",
          "[glaze][producer][extensions]") {
    const auto registry = loadMiniRegistry();
    for (const auto &ext : registry.extensions()) {
        if (ext.name == "GL_ARB_buffer_storage") {
            REQUIRE(ext.supported.size() == 2);
            REQUIRE(ext.supported[0] == "gl");
            REQUIRE(ext.supported[1] == "gles2");
        }
        if (ext.name == "GL_NV_shader_buffer_load") {
            REQUIRE(ext.supported.size() == 1);
            REQUIRE(ext.supported.front() == "gl");
        }
    }
}

TEST_CASE("extensionsForApi returns gl-supported extensions",
          "[glaze][producer][extensions]") {
    const auto registry = loadMiniRegistry();
    const auto glExts = registry.extensionsForApi("gl");
    REQUIRE(glExts.size() == 4);
    const auto glesExts = registry.extensionsForApi("gles2");
    REQUIRE(glesExts.size() == 2);
}
