#include "glaze/model/Command.h"

#include <catch2/catch_test_macros.hpp>

using namespace glaze::model;

TEST_CASE("getClass returns the first parameter's class attribute", "[glaze][model][command]") {
    Command cmd;
    cmd.name = "glBindBuffer";
    cmd.returnType = TypeDecl{"void", false, false};
    cmd.returnTypeStr = "void";

    CommandParam target;
    target.name = "target";
    target.typeParts = {"GLenum"};
    target.group = "BufferTarget";

    CommandParam buffer;
    buffer.name = "buffer";
    buffer.typeParts = {"GLuint"};
    buffer.classStr = "buffer";

    cmd.params = {target, buffer};

    // The first param lacks a class attribute -> no class reported.
    REQUIRE_FALSE(getClass(cmd).has_value());

    cmd.params = {buffer, target};
    REQUIRE(getClass(cmd).has_value());
    REQUIRE(*getClass(cmd) == "buffer");
}

TEST_CASE("getClass returns nullopt for empty params", "[glaze][model][command]") {
    Command cmd;
    cmd.name = "glClear";
    cmd.returnType = TypeDecl{"void", false, false};
    cmd.returnTypeStr = "void";
    REQUIRE_FALSE(getClass(cmd).has_value());
}
