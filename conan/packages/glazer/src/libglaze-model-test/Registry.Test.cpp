#include "glaze/model/Registry.h"

#include <catch2/catch_test_macros.hpp>

#include <string>

using namespace glaze::model;

namespace {

//! Build a hand-crafted Registry that mirrors the Python MINI_XML fixture.
//! Two feature versions for gl (1.0 and 3.3), two commands, two enum groups,
//! one extension covering ARB. Used by most tests in this file.
Registry makeFixture() {
    std::vector<Type> types{
        Type{"GLenum", "typedef unsigned int GLenum;", std::nullopt, std::nullopt},
        Type{"GLuint", "typedef unsigned int GLuint;", std::nullopt, std::nullopt},
        Type{"GLbitfield", "typedef unsigned int GLbitfield;", std::nullopt, std::nullopt},
    };

    Enum colorBit{"GL_COLOR_BUFFER_BIT", "0x00004000",
                  {"ClearBufferMask"}, std::nullopt, std::nullopt};
    Enum depthBit{"GL_DEPTH_BUFFER_BIT", "0x00000100",
                  {"ClearBufferMask"}, std::nullopt, std::nullopt};

    EnumGroup clearMask;
    clearMask.namespaceName = "GL";
    clearMask.group = "ClearBufferMask";
    clearMask.enumGroupType = "bitmask";
    clearMask.enums = {{"GL_COLOR_BUFFER_BIT", colorBit},
                      {"GL_DEPTH_BUFFER_BIT", depthBit}};

    std::vector<EnumGroup> enumGroups{clearMask};

    CommandParam clearMaskParam;
    clearMaskParam.name = "mask";
    clearMaskParam.typeParts = {"GLbitfield"};
    clearMaskParam.group = "ClearBufferMask";

    Command glClear;
    glClear.name = "glClear";
    glClear.returnType = TypeDecl{"void", false, false};
    glClear.returnTypeStr = "void";
    glClear.params = {clearMaskParam};

    CommandParam bufferParam;
    bufferParam.name = "buffer";
    bufferParam.typeParts = {"GLuint"};
    bufferParam.classStr = "buffer";

    CommandParam targetParam;
    targetParam.name = "target";
    targetParam.typeParts = {"GLenum"};
    targetParam.group = "BufferTarget";

    Command glBindBuffer;
    glBindBuffer.name = "glBindBuffer";
    glBindBuffer.returnType = TypeDecl{"void", false, false};
    glBindBuffer.returnTypeStr = "void";
    glBindBuffer.params = {targetParam, bufferParam};

    Command glDeleteBuffers;
    glDeleteBuffers.name = "glDeleteBuffers";
    glDeleteBuffers.returnType = TypeDecl{"void", false, false};
    glDeleteBuffers.returnTypeStr = "void";
    CommandParam n;
    n.name = "n";
    n.typeParts = {"GLsizei"};
    CommandParam buffers;
    buffers.name = "buffers";
    buffers.typeParts = {"const", "GLuint", "*"};
    buffers.classStr = "buffer";
    glDeleteBuffers.params = {n, buffers};

    std::vector<Command> commands{glClear, glBindBuffer, glDeleteBuffers};

    Feature gl10;
    gl10.api = "gl";
    gl10.name = "GL_VERSION_1_0";
    gl10.number = "1.0";
    Require r10;
    r10.enums.push_back(EnumRef{"GL_COLOR_BUFFER_BIT", std::nullopt});
    r10.enums.push_back(EnumRef{"GL_DEPTH_BUFFER_BIT", std::nullopt});
    r10.commands.push_back(CommandRef{"glClear", std::nullopt});
    gl10.requireList.push_back(r10);

    Feature gl15;
    gl15.api = "gl";
    gl15.name = "GL_VERSION_1_5";
    gl15.number = "1.5";
    Require r15;
    r15.commands.push_back(CommandRef{"glBindBuffer", std::nullopt});
    r15.commands.push_back(CommandRef{"glDeleteBuffers", std::nullopt});
    gl15.requireList.push_back(r15);

    Feature gl32;
    gl32.api = "gl";
    gl32.name = "GL_VERSION_3_2";
    gl32.number = "3.2";
    Remove coreRemove;
    coreRemove.profile = "core";
    coreRemove.enums.push_back(EnumRef{"GL_DEPTH_BUFFER_BIT", std::nullopt});
    gl32.removeList.push_back(coreRemove);

    std::vector<Feature> features{gl10, gl15, gl32};

    Extension ext;
    ext.name = "GL_ARB_buffer_storage";
    ext.supported = {"gl"};
    std::vector<Extension> extensions{ext};

    return Registry{std::move(types), std::move(enumGroups), std::move(commands),
                    std::move(features), std::move(extensions)};
}

} // namespace

TEST_CASE("Registry builds type/enum/command indices", "[glaze][model][registry]") {
    const auto registry = makeFixture();
    REQUIRE(registry.findType("GLuint") != nullptr);
    REQUIRE(registry.findType("NoSuch") == nullptr);
    REQUIRE(registry.findEnum("GL_COLOR_BUFFER_BIT") != nullptr);
    REQUIRE(registry.findCommand("glClear") != nullptr);
    REQUIRE(registry.findCommand("glNonexistent") == nullptr);
}

TEST_CASE("Registry flags bitmask groups", "[glaze][model][registry]") {
    const auto registry = makeFixture();
    REQUIRE(registry.isBitmaskGroup("ClearBufferMask"));
    REQUIRE_FALSE(registry.isBitmaskGroup("BufferTarget"));
}

TEST_CASE("Registry indexes commands by first-param class", "[glaze][model][registry]") {
    const auto registry = makeFixture();
    // In the fixture every command's FIRST parameter lacks a class attribute,
    // so object_dict is empty. This matches Python Registry.object_dict, which
    // keys by Command.get_class() -> first param's class_ attr.
    REQUIRE(registry.commandsForObjectClass("buffer") == nullptr);
    REQUIRE(registry.commandsForObjectClass("nonexistent") == nullptr);
}

TEST_CASE("Registry indexes commands whose first param does carry a class",
          "[glaze][model][registry]") {
    // Build a minimal fixture where glClearBufferData's first param is a
    // buffer handle — this exercises the object_dict indexing path.
    Command glClearBufferData;
    glClearBufferData.name = "glClearBufferData";
    glClearBufferData.returnType = TypeDecl{"void", false, false};
    glClearBufferData.returnTypeStr = "void";
    CommandParam bufferParam;
    bufferParam.name = "buffer";
    bufferParam.typeParts = {"GLuint"};
    bufferParam.classStr = "buffer";
    glClearBufferData.params = {bufferParam};

    Registry registry{{}, {}, {glClearBufferData}, {}, {}};
    const auto *bufferCommands = registry.commandsForObjectClass("buffer");
    REQUIRE(bufferCommands != nullptr);
    REQUIRE(bufferCommands->size() == 1);
    REQUIRE((*bufferCommands)[0]->name == "glClearBufferData");
}

TEST_CASE("Registry::availableApis lists sorted versions", "[glaze][model][registry]") {
    const auto registry = makeFixture();
    const auto apis = registry.availableApis();
    REQUIRE(apis.find("gl") != apis.end());
    const auto &glVersions = apis.at("gl");
    REQUIRE(glVersions.size() == 3);
    REQUIRE(glVersions.front() == "1.0");
    REQUIRE(glVersions.back() == "3.2");
    // gl_compat is a virtual API mirroring gl.
    REQUIRE(apis.find("gl_compat") != apis.end());
    REQUIRE(apis.at("gl_compat") == glVersions);
}

TEST_CASE("Registry::featuresForApi returns the raw feature list", "[glaze][model][registry]") {
    const auto registry = makeFixture();
    const auto *features = registry.featuresForApi("gl");
    REQUIRE(features != nullptr);
    REQUIRE(features->size() == 3);
    REQUIRE(features->front().number == "1.0");
}

TEST_CASE("Registry::collectFeatures filters by version", "[glaze][model][registry]") {
    const auto registry = makeFixture();
    const auto collected = registry.collectFeatures("gl", "1.5");
    REQUIRE(collected.size() == 2);
    REQUIRE(collected[0].get().number == "1.0");
    REQUIRE(collected[1].get().number == "1.5");
}

TEST_CASE("Registry::extensionsForApi resolves virtual APIs", "[glaze][model][registry]") {
    const auto registry = makeFixture();
    const auto real = registry.extensionsForApi("gl");
    REQUIRE(real.size() == 1);
    REQUIRE(real.front().name == "GL_ARB_buffer_storage");

    const auto virtualProfile = registry.extensionsForApi("gl_compat");
    REQUIRE(virtualProfile.size() == 1);
    REQUIRE(virtualProfile.front().name == "GL_ARB_buffer_storage");
}

TEST_CASE("Registry::consolidate flattens requires up to version",
          "[glaze][model][registry][consolidate]") {
    const auto registry = makeFixture();
    const auto snap = registry.consolidate("gl", "1.5");
    REQUIRE(snap.commands.count("glClear") == 1);
    REQUIRE(snap.commands.count("glBindBuffer") == 1);
    REQUIRE(snap.commands.count("glDeleteBuffers") == 1);
    REQUIRE(snap.enums.count("GL_COLOR_BUFFER_BIT") == 1);
    REQUIRE(snap.enums.count("GL_DEPTH_BUFFER_BIT") == 1);
}

TEST_CASE("Registry::consolidate strips core removes on real gl",
          "[glaze][model][registry][consolidate]") {
    const auto registry = makeFixture();
    const auto snap = registry.consolidate("gl", "3.2");
    // The fixture removes GL_DEPTH_BUFFER_BIT at 3.2 under the core profile.
    REQUIRE(snap.enums.count("GL_COLOR_BUFFER_BIT") == 1);
    REQUIRE(snap.enums.count("GL_DEPTH_BUFFER_BIT") == 0);
}

TEST_CASE("Registry::consolidate preserves symbols on gl_compat",
          "[glaze][model][registry][consolidate][gl_compat]") {
    const auto registry = makeFixture();
    const auto snap = registry.consolidate("gl_compat", "3.2");
    // gl_compat skips <remove profile="core">, so GL_DEPTH_BUFFER_BIT stays.
    REQUIRE(snap.enums.count("GL_DEPTH_BUFFER_BIT") == 1);
    REQUIRE(snap.enums.count("GL_COLOR_BUFFER_BIT") == 1);
}

TEST_CASE("Registry::resolveApi returns profiles for virtual APIs", "[glaze][model][registry]") {
    const auto registry = makeFixture();
    const auto real = registry.resolveApi("gl");
    REQUIRE_FALSE(real.has_value());
    const auto virt = registry.resolveApi("gl_compat");
    REQUIRE(virt.has_value());
    REQUIRE(virt->registryApi == "gl");
    REQUIRE(virt->skipRemoveProfiles.count("core") == 1);
}
