#include <catch2/catch_test_macros.hpp>

#include <string>

#include <cmcheck/Glob.h>

using namespace cmcheck;

TEST_CASE("Glob matches literal strings", "[glob]") {
    CHECK(globMatch("src/engine", "src/engine"));
    CHECK_FALSE(globMatch("src/engine", "src/enginex"));
}

TEST_CASE("Glob matches star wildcards", "[glob]") {
    CHECK(globMatch("src/*", "src/engine"));
    CHECK(globMatch("*.cpp", "main.cpp"));
    CHECK_FALSE(globMatch("*.cpp", "main.h"));
}

TEST_CASE("Glob matches double-star patterns", "[glob]") {
    CHECK(globMatch("src/ide/**", "src/ide/widgets/CMakeLists.txt"));
    CHECK(globMatch("src/ide/**", "src/ide/CMakeLists.txt"));
    CHECK(globMatch("build*/**", "build-cmake-check/Release/trace.json"));
    CHECK_FALSE(globMatch("src/ide/**", "src/engine/CMakeLists.txt"));
}

TEST_CASE("Glob matches question marks", "[glob]") {
    CHECK(globMatch("file?.cpp", "file1.cpp"));
    CHECK_FALSE(globMatch("file?.cpp", "file10.cpp"));
}