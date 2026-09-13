#include "glaze/cli/ArgParse.h"

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <vector>

using glaze::Language;
using glaze::cli::parseGenerate;
using glaze::cli::parseListApisRegistryPath;

namespace {

//! Build the pointer array cxxopts expects, leaking nothing into the heap.
struct Argv {
    std::vector<std::string> args;
    std::vector<const char *> ptrs;

    Argv(std::initializer_list<std::string> a) : args(a) {
        ptrs.reserve(args.size());
        for (const auto &s : args) {
            ptrs.push_back(s.c_str());
        }
    }
    int argc() const { return static_cast<int>(ptrs.size()); }
    const char *const *argv() const { return ptrs.data(); }
};

} // namespace

TEST_CASE("parseGenerate accepts a minimal colon-form invocation",
          "[glaze][core][cli]") {
    const Argv argv{{"glaze", "--registry", "foo.xml", "--api", "gl:3.3", "--lang", "cpp"}};
    const auto options = parseGenerate(argv.argc(), argv.argv());
    REQUIRE(options.has_value());
    REQUIRE(options->registryPath.string() == "foo.xml");
    REQUIRE(options->apis.size() == 1);
    REQUIRE(options->apis.front().api == "gl");
    REQUIRE(options->apis.front().version == "3.3");
    REQUIRE(options->languages.size() == 1);
    REQUIRE(options->languages.front() == Language::Cpp);
}

TEST_CASE("parseGenerate handles multiple --lang and --api flags",
          "[glaze][core][cli]") {
    const Argv argv{{
        "glaze",
        "--api", "gl:3.3",
        "--api", "gles2:3.2",
        "--lang", "c",
        "--lang", "cpp",
        "--output-dir", "out",
    }};
    const auto options = parseGenerate(argv.argc(), argv.argv());
    REQUIRE(options.has_value());
    REQUIRE(options->apis.size() == 2);
    REQUIRE(options->languages.size() == 2);
    REQUIRE(options->outputDir.string() == "out");
}

TEST_CASE("parseGenerate splits comma-separated extension filters",
          "[glaze][core][cli]") {
    const Argv argv{{
        "glaze",
        "--api", "gl:4.6",
        "--lang", "c",
        "--extension-vendors", "ARB,KHR",
        "--extensions", "GL_NV_foo,GL_ATI_bar",
    }};
    const auto options = parseGenerate(argv.argc(), argv.argv());
    REQUIRE(options.has_value());
    REQUIRE(options->extensionVendors.count("ARB") == 1);
    REQUIRE(options->extensionVendors.count("KHR") == 1);
    REQUIRE(options->extensionNames.count("GL_NV_foo") == 1);
    REQUIRE(options->extensionNames.count("GL_ATI_bar") == 1);
}

TEST_CASE("parseGenerate rejects a missing colon in --api", "[glaze][core][cli][errors]") {
    const Argv argv{{"glaze", "--api", "gl-3.3", "--lang", "c"}};
    REQUIRE_THROWS_AS(parseGenerate(argv.argc(), argv.argv()), std::invalid_argument);
}

TEST_CASE("parseGenerate requires at least one --api and --lang",
          "[glaze][core][cli][errors]") {
    const Argv noApi{{"glaze", "--lang", "c"}};
    REQUIRE_THROWS_AS(parseGenerate(noApi.argc(), noApi.argv()), std::invalid_argument);
    const Argv noLang{{"glaze", "--api", "gl:3.3"}};
    REQUIRE_THROWS_AS(parseGenerate(noLang.argc(), noLang.argv()), std::invalid_argument);
}

TEST_CASE("parseGenerate accepts --refpages-dir",
          "[glaze][core][cli]") {
    const Argv argv{{
        "glaze",
        "--api", "gl:4.6",
        "--lang", "cpp",
        "--refpages-dir", "docs/OpenGL-Refpages",
    }};
    const auto options = parseGenerate(argv.argc(), argv.argv());
    REQUIRE(options.has_value());
    REQUIRE(options->refpagesDir == "docs/OpenGL-Refpages");
}

TEST_CASE("parseGenerate leaves refpagesDir empty by default",
          "[glaze][core][cli]") {
    const Argv argv{{
        "glaze",
        "--api", "gl:4.6",
        "--lang", "cpp",
    }};
    const auto options = parseGenerate(argv.argc(), argv.argv());
    REQUIRE(options.has_value());
    REQUIRE(options->refpagesDir.empty());
}

TEST_CASE("parseListApisRegistryPath defaults to gl.xml", "[glaze][core][cli]") {
    const Argv defaultArgv{{"glaze"}};
    REQUIRE(parseListApisRegistryPath(defaultArgv.argc(), defaultArgv.argv()) == "gl.xml");

    const Argv overrideArgv{{"glaze", "--registry", "alt.xml"}};
    REQUIRE(parseListApisRegistryPath(overrideArgv.argc(), overrideArgv.argv()) == "alt.xml");
}
