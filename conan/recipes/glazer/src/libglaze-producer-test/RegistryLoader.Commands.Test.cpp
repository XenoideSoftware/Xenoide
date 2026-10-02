#include "MiniRegistry.h"

#include <catch2/catch_test_macros.hpp>

using glaze::model::getClass;
using glaze::testfixtures::loadMiniRegistry;

TEST_CASE("Command parser reads glClear and its group", "[glaze][producer][commands]") {
    const auto registry = loadMiniRegistry();
    const auto *clear = registry.findCommand("glClear");
    REQUIRE(clear != nullptr);
    REQUIRE(clear->returnType.name == "void");
    REQUIRE_FALSE(clear->returnType.isPointer);
    REQUIRE(clear->params.size() == 1);
    REQUIRE(clear->params.front().name == "mask");
    REQUIRE(clear->params.front().group.has_value());
    REQUIRE(*clear->params.front().group == "ClearBufferMask");
}

TEST_CASE("Command parser reads glGetString with pointer return type",
          "[glaze][producer][commands]") {
    const auto registry = loadMiniRegistry();
    const auto *getString = registry.findCommand("glGetString");
    REQUIRE(getString != nullptr);
    REQUIRE(getString->returnType.name == "GLubyte");
    REQUIRE(getString->returnType.isConst);
    REQUIRE(getString->returnType.isPointer);
}

TEST_CASE("Command parser picks up param class attributes",
          "[glaze][producer][commands]") {
    const auto registry = loadMiniRegistry();
    const auto *bindBuffer = registry.findCommand("glBindBuffer");
    REQUIRE(bindBuffer != nullptr);
    REQUIRE(bindBuffer->params.size() == 2);
    REQUIRE(bindBuffer->params[1].classStr.has_value());
    REQUIRE(*bindBuffer->params[1].classStr == "buffer");
}

TEST_CASE("commandsForObjectClass indexes by first-param class",
          "[glaze][producer][commands]") {
    const auto registry = loadMiniRegistry();
    // glDeleteProgram's first param has class="program", so it's indexed.
    const auto *programCommands = registry.commandsForObjectClass("program");
    REQUIRE(programCommands != nullptr);
    REQUIRE_FALSE(programCommands->empty());

    // glNamedBufferData's first param has class="buffer".
    const auto *bufferCommands = registry.commandsForObjectClass("buffer");
    REQUIRE(bufferCommands != nullptr);
    bool foundNamedBufferData = false;
    for (const auto *cmd : *bufferCommands) {
        if (cmd->name == "glNamedBufferData") {
            foundNamedBufferData = true;
            break;
        }
    }
    REQUIRE(foundNamedBufferData);
}

TEST_CASE("Commands across multiple <commands> blocks are parsed",
          "[glaze][producer][commands]") {
    const auto registry = loadMiniRegistry();
    // glMakeBufferResidentNV lives in the second <commands> block after the
    // gles2 feature — make sure the parser picks up both blocks.
    REQUIRE(registry.findCommand("glMakeBufferResidentNV") != nullptr);
    REQUIRE(registry.findCommand("glDebugMessageCallbackKHR") != nullptr);
}
