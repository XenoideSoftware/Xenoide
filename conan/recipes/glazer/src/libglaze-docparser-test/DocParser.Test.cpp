#include "glaze/docparser/DocParser.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

using glaze::docparser::DocParser;
using glaze::docparser::DocIndex;

namespace {

//! Minimal DocBook 5 refpage shaped exactly like a real OpenGL-Refpages
//! entry. Two <refname>s share the brief and parameter docs so we can
//! verify the "sibling functions share the same payload" path.
constexpr std::string_view kMiniRefpage = R"(<?xml version="1.0"?>
<refentry xmlns="http://docbook.org/ns/docbook" version="5.0" xml:id="glDummy">
    <refnamediv>
        <refname>glDummy</refname>
        <refname>glDummyAlias</refname>
        <refpurpose>
            clear   buffers to   preset values
        </refpurpose>
    </refnamediv>
    <refsect1 xml:id="parameters">
        <title>Parameters</title>
        <variablelist>
            <varlistentry>
                <term><parameter>mask</parameter></term>
                <listitem>
                    <para>
                        Bitwise OR of masks that indicate the buffers to
                        be cleared.
                    </para>
                </listitem>
            </varlistentry>
            <varlistentry>
                <term><parameter>count</parameter></term>
                <listitem>
                    <para>How many items to process.</para>
                </listitem>
            </varlistentry>
        </variablelist>
    </refsect1>
</refentry>
)";

//! A broken refpage missing the <refpurpose>. Should be silently skipped.
constexpr std::string_view kMissingPurpose = R"(<?xml version="1.0"?>
<refentry xmlns="http://docbook.org/ns/docbook" xml:id="glBroken">
    <refnamediv>
        <refname>glBroken</refname>
    </refnamediv>
</refentry>
)";

//! A file whose name does not start with "gl" — must be skipped even if
//! its contents are valid, mirroring the Python `startswith("gl")` guard.
constexpr std::string_view kNotGlPrefixed = R"(<?xml version="1.0"?>
<refentry xmlns="http://docbook.org/ns/docbook" xml:id="notgl">
    <refnamediv>
        <refname>frobnicate</refname>
        <refpurpose>nonsense</refpurpose>
    </refnamediv>
</refentry>
)";

std::filesystem::path makeFixtureDir(const std::string &tag) {
    auto root = std::filesystem::temp_directory_path() /
                ("glaze-docparser-" + tag);
    std::error_code ec;
    std::filesystem::remove_all(root, ec);
    std::filesystem::create_directories(root / "gl4");
    return root;
}

void writeFile(const std::filesystem::path &path, std::string_view body) {
    std::ofstream out{path, std::ios::binary | std::ios::trunc};
    out.write(body.data(), static_cast<std::streamsize>(body.size()));
}

} // namespace

TEST_CASE("DocParser skips a missing refpages directory", "[glaze][docparser]") {
    const auto bogus = std::filesystem::temp_directory_path() /
                       "glaze-docparser-does-not-exist";
    std::error_code ec;
    std::filesystem::remove_all(bogus, ec);

    const DocIndex index = DocParser{}.parseRefpages(bogus, "gl");
    REQUIRE(index.empty());
}

TEST_CASE("DocParser returns empty index when API subdir is missing",
          "[glaze][docparser]") {
    const auto dir = makeFixtureDir("api-subdir");
    // Only gl4 exists; request gles2 (maps to es3.0) which is absent.
    const DocIndex index = DocParser{}.parseRefpages(dir, "gles2");
    REQUIRE(index.empty());
}

TEST_CASE("DocParser extracts brief and params from a refpage",
          "[glaze][docparser]") {
    const auto dir = makeFixtureDir("basic");
    writeFile(dir / "gl4" / "glDummy.xml", kMiniRefpage);

    const DocIndex index = DocParser{}.parseRefpages(dir, "gl");
    REQUIRE(index.size() == 2);
    REQUIRE(index.count("glDummy") == 1);
    REQUIRE(index.count("glDummyAlias") == 1);

    const auto &doc = index.at("glDummy");
    REQUIRE(doc.brief == "clear buffers to preset values");
    REQUIRE(doc.params.size() == 2);
    REQUIRE(doc.params.at("mask") ==
            "Bitwise OR of masks that indicate the buffers to be cleared.");
    REQUIRE(doc.params.at("count") == "How many items to process.");

    // Alias shares the same payload.
    REQUIRE(index.at("glDummyAlias").brief == doc.brief);
    REQUIRE(index.at("glDummyAlias").params == doc.params);
}

TEST_CASE("DocParser silently skips refpages without a refpurpose",
          "[glaze][docparser]") {
    const auto dir = makeFixtureDir("missing-purpose");
    writeFile(dir / "gl4" / "glBroken.xml", kMissingPurpose);
    writeFile(dir / "gl4" / "glDummy.xml", kMiniRefpage);

    const DocIndex index = DocParser{}.parseRefpages(dir, "gl");
    REQUIRE(index.count("glBroken") == 0);
    REQUIRE(index.count("glDummy") == 1);
}

TEST_CASE("DocParser skips files that do not start with gl",
          "[glaze][docparser]") {
    const auto dir = makeFixtureDir("not-gl");
    writeFile(dir / "gl4" / "frobnicate.xml", kNotGlPrefixed);
    writeFile(dir / "gl4" / "glDummy.xml", kMiniRefpage);

    const DocIndex index = DocParser{}.parseRefpages(dir, "gl");
    REQUIRE(index.count("frobnicate") == 0);
    REQUIRE(index.count("glDummy") == 1);
}

TEST_CASE("DocParser maps gles1 to es1.1 subdirectory",
          "[glaze][docparser]") {
    const auto dir = makeFixtureDir("gles1");
    std::filesystem::create_directories(dir / "es1.1");
    writeFile(dir / "es1.1" / "glDummy.xml", kMiniRefpage);

    const DocIndex index = DocParser{}.parseRefpages(dir, "gles1");
    REQUIRE(index.count("glDummy") == 1);
}
