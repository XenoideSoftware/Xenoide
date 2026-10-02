#include "MiniRegistry.h"

#include <catch2/catch_test_macros.hpp>

using glaze::testfixtures::loadMiniRegistry;

TEST_CASE("Feature parser produces five gl features and one gles2 feature",
          "[glaze][producer][features]") {
    const auto registry = loadMiniRegistry();
    const auto *glFeatures = registry.featuresForApi("gl");
    REQUIRE(glFeatures != nullptr);
    REQUIRE(glFeatures->size() == 5);
    REQUIRE(glFeatures->front().number == "1.0");
    REQUIRE(glFeatures->back().number == "4.5");

    const auto *glesFeatures = registry.featuresForApi("gles2");
    REQUIRE(glesFeatures != nullptr);
    REQUIRE(glesFeatures->size() == 1);
    REQUIRE(glesFeatures->front().number == "2.0");
}

TEST_CASE("Feature parser captures the GL_VERSION_3_1 core remove",
          "[glaze][producer][features]") {
    const auto registry = loadMiniRegistry();
    const auto *glFeatures = registry.featuresForApi("gl");
    REQUIRE(glFeatures != nullptr);

    bool found31 = false;
    for (const auto &feature : *glFeatures) {
        if (feature.number == "3.1") {
            found31 = true;
            REQUIRE(feature.removeList.size() == 1);
            REQUIRE(feature.removeList.front().profile == "core");
            REQUIRE(feature.removeList.front().enums.size() == 1);
            REQUIRE(feature.removeList.front().enums.front().name == "GL_FLOAT");
        }
    }
    REQUIRE(found31);
}

TEST_CASE("Registry::consolidate flattens requires up to gl 4.5",
          "[glaze][producer][features]") {
    const auto registry = loadMiniRegistry();
    const auto snap = registry.consolidate("gl", "4.5");
    REQUIRE(snap.commands.count("glClear") == 1);
    REQUIRE(snap.commands.count("glBindBuffer") == 1);
    REQUIRE(snap.commands.count("glNamedBufferData") == 1);
    // GL_FLOAT is core-removed at 3.1 so the core gl profile drops it.
    REQUIRE(snap.enums.count("GL_FLOAT") == 0);
    // gl_compat should preserve it.
    const auto compatSnap = registry.consolidate("gl_compat", "4.5");
    REQUIRE(compatSnap.enums.count("GL_FLOAT") == 1);
}
