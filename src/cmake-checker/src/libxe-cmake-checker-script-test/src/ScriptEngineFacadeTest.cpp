#include "xe/cmake/script/ScriptEngineFacade.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake::script {

    TEST_CASE("ScriptEngineFacade evaluates simple snippets") {
        ScriptEngineFacade facade;
        REQUIRE_NOTHROW(facade.eval("var x = 1 + 2;"));
    }

    TEST_CASE("ScriptEngineFacade detects defined functions") {
        ScriptEngineFacade facade;
        facade.eval("def hello() { return 1; }");
        REQUIRE(facade.has_function("hello"));
        REQUIRE_FALSE(facade.has_function("missing_function"));
    }

    TEST_CASE("ScriptEngineFacade registers bindings") {
        ScriptEngineFacade facade;
        bool invoked = false;
        facade.register_bindings([&invoked](void *) { invoked = true; });
        REQUIRE(invoked);
    }

    TEST_CASE("ScriptEngineFacade supports move semantics") {
        ScriptEngineFacade first;
        first.eval("var x = 42;");
        ScriptEngineFacade second = std::move(first);
        REQUIRE_NOTHROW(second.eval("var y = 1;"));
    }

    TEST_CASE("ScriptEngineFacade move assignment") {
        ScriptEngineFacade first;
        first.eval("var a = 1;");
        ScriptEngineFacade second;
        second = std::move(first);
        REQUIRE_NOTHROW(second.eval("var b = 2;"));
    }

    TEST_CASE("ScriptEngineFacade register_bindings receives the engine pointer") {
        ScriptEngineFacade facade;
        void *received = nullptr;
        facade.register_bindings([&received](void *engine) { received = engine; });
        REQUIRE(received != nullptr);
    }

    TEST_CASE("ScriptEngineFacade has_function on plain names") {
        ScriptEngineFacade facade;
        facade.eval("def named_hook() { return 0; }");
        REQUIRE(facade.has_function("named_hook"));
        REQUIRE_FALSE(facade.has_function("nonexistent_hook"));
    }

} // namespace xe::cmake::script