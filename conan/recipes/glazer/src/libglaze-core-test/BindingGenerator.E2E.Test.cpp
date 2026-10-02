#include "glaze/BindingGenerator.h"

#include "MiniRegistry.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

using glaze::ApiRequest;
using glaze::BindingGenerator;
using glaze::GenerateOptions;
using glaze::Language;
using glaze::testfixtures::kMiniXml;

namespace {

//! Write MINI_XML to a per-test temp file and return its path. Each call
//! picks a unique suffix so parallel test runs do not collide.
std::filesystem::path writeMiniXml(const std::string &tag) {
    auto path = std::filesystem::temp_directory_path() / ("glaze-core-" + tag + ".xml");
    std::ofstream out{path, std::ios::binary | std::ios::trunc};
    out.write(kMiniXml.data(), static_cast<std::streamsize>(kMiniXml.size()));
    return path;
}

//! Build an isolated output directory for each test case. std::filesystem
//! happily overwrites existing files so repeat runs stay deterministic.
std::filesystem::path makeOutputDir(const std::string &tag) {
    auto dir = std::filesystem::temp_directory_path() / ("glaze-core-out-" + tag);
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
    std::filesystem::create_directories(dir);
    return dir;
}

} // namespace

TEST_CASE("BindingGenerator generates both C and C++ outputs", "[glaze][core][e2e]") {
    const auto xmlPath = writeMiniXml("e2e");
    const auto outDir = makeOutputDir("e2e");

    GenerateOptions options;
    options.registryPath = xmlPath;
    options.outputDir = outDir;
    options.apis = {ApiRequest{"gl", "4.5"}};
    options.languages = {Language::C, Language::Cpp};

    const auto result = BindingGenerator{}.generate(options);
    REQUIRE(result.writtenFiles.size() == 5); // gl.h + gl.c + gl.hpp + gl_handle.hpp + raii.hpp

    REQUIRE(std::filesystem::exists(outDir / "include" / "glaze" / "gl.h"));
    REQUIRE(std::filesystem::exists(outDir / "src" / "gl.c"));
    REQUIRE(std::filesystem::exists(outDir / "include" / "glaze" / "gl.hpp"));
    REQUIRE(std::filesystem::exists(outDir / "include" / "glaze" / "gl_handle.hpp"));
    REQUIRE(std::filesystem::exists(outDir / "include" / "glaze" / "raii.hpp"));

    // All files must be non-empty.
    for (const auto &path : result.writtenFiles) {
        REQUIRE(std::filesystem::file_size(path) > 0);
    }

    std::filesystem::remove(xmlPath);
}

TEST_CASE("BindingGenerator::listApis returns gl and gles2", "[glaze][core][list]") {
    const auto xmlPath = writeMiniXml("listapis");
    const auto apis = BindingGenerator{}.listApis(xmlPath);
    REQUIRE(apis.find("gl") != apis.end());
    REQUIRE(apis.find("gles2") != apis.end());
    REQUIRE(apis.find("gl_compat") != apis.end()); // virtual api resolves
    std::filesystem::remove(xmlPath);
}

TEST_CASE("BindingGenerator propagates custom NameTransform into the C++ output",
          "[glaze][core][extension]") {
    struct MyNames : public glaze::codegen::NameTransform {
        std::string transformCommandName(std::string_view glName) const override {
            return std::string{"myfacade_"} +
                   glaze::codegen::DefaultNameTransform{}.transformCommandName(glName);
        }
    };

    const auto xmlPath = writeMiniXml("names");
    const auto outDir = makeOutputDir("names");

    GenerateOptions options;
    options.registryPath = xmlPath;
    options.outputDir = outDir;
    options.apis = {ApiRequest{"gl", "4.5"}};
    options.languages = {Language::Cpp};
    options.nameTransform = std::make_shared<MyNames>();

    BindingGenerator{}.generate(options);

    std::ifstream in{outDir / "include" / "glaze" / "gl.hpp"};
    std::string content{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
    REQUIRE(content.find("myfacade_clear") != std::string::npos);

    std::filesystem::remove(xmlPath);
}
