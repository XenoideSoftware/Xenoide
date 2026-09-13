#include "MiniRegistry.h"

#include "glaze/BindingGenerator.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>

using glaze::ApiRequest;
using glaze::BindingGenerator;
using glaze::GenerateOptions;
using glaze::Language;
using glaze::testfixtures::kMiniXml;

namespace {

//! One hand-crafted DocBook refpage that documents `glClear` so the doc
//! parser has something to hand to the generators during this test.
constexpr std::string_view kClearRefpage = R"(<?xml version="1.0"?>
<refentry xmlns="http://docbook.org/ns/docbook" version="5.0" xml:id="glClear">
    <refnamediv>
        <refname>glClear</refname>
        <refpurpose>clear   buffers   to preset values</refpurpose>
    </refnamediv>
    <refsect1 xml:id="parameters">
        <title>Parameters</title>
        <variablelist>
            <varlistentry>
                <term><parameter>mask</parameter></term>
                <listitem>
                    <para>Bitwise OR of the buffers to clear.</para>
                </listitem>
            </varlistentry>
        </variablelist>
    </refsect1>
</refentry>
)";

std::filesystem::path makeDir(const std::string &tag) {
    auto dir = std::filesystem::temp_directory_path() /
               ("glaze-docbrief-" + tag);
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
    std::filesystem::create_directories(dir);
    return dir;
}

std::string slurp(const std::filesystem::path &path) {
    std::ifstream in{path, std::ios::binary};
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

} // namespace

TEST_CASE("BindingGenerator attaches doc briefs when --refpages-dir is set",
          "[glaze][core][docs]") {
    // Stage: registry XML + a refpages tree with a single glClear entry.
    const auto workdir = makeDir("attached");
    const auto registryPath = workdir / "gl.xml";
    {
        std::ofstream out{registryPath, std::ios::binary | std::ios::trunc};
        out.write(kMiniXml.data(), static_cast<std::streamsize>(kMiniXml.size()));
    }
    const auto refpagesDir = workdir / "refpages";
    std::filesystem::create_directories(refpagesDir / "gl4");
    {
        std::ofstream out{refpagesDir / "gl4" / "glClear.xml",
                          std::ios::binary | std::ios::trunc};
        out.write(kClearRefpage.data(),
                  static_cast<std::streamsize>(kClearRefpage.size()));
    }
    const auto outDir = workdir / "out";

    GenerateOptions options;
    options.registryPath = registryPath;
    options.outputDir = outDir;
    options.apis = {ApiRequest{"gl", "4.5"}};
    options.languages = {Language::C, Language::Cpp};
    options.refpagesDir = refpagesDir;

    const auto result = BindingGenerator{}.generate(options);
    REQUIRE(result.writtenFiles.size() == 5);

    const auto hpp = slurp(outDir / "include" / "glaze" / "gl.hpp");
    const auto h = slurp(outDir / "include" / "glaze" / "gl.h");
    REQUIRE(hpp.find("/** @brief clear buffers to preset values */") !=
            std::string::npos);
    REQUIRE(h.find("/** @brief clear buffers to preset values */") !=
            std::string::npos);
}

TEST_CASE("BindingGenerator emits no Doxygen blocks when refpagesDir is empty",
          "[glaze][core][docs]") {
    const auto workdir = makeDir("empty");
    const auto registryPath = workdir / "gl.xml";
    {
        std::ofstream out{registryPath, std::ios::binary | std::ios::trunc};
        out.write(kMiniXml.data(), static_cast<std::streamsize>(kMiniXml.size()));
    }
    const auto outDir = workdir / "out";

    GenerateOptions options;
    options.registryPath = registryPath;
    options.outputDir = outDir;
    options.apis = {ApiRequest{"gl", "4.5"}};
    options.languages = {Language::Cpp};
    // refpagesDir left empty on purpose.

    BindingGenerator{}.generate(options);
    const auto hpp = slurp(outDir / "include" / "glaze" / "gl.hpp");

    // No English brief for glClear should appear; the functor is still
    // emitted, but without a preceding /** @brief ... */ comment.
    REQUIRE(hpp.find("struct ClearFn") != std::string::npos);
    REQUIRE(hpp.find("clear buffers to preset values") == std::string::npos);
}
