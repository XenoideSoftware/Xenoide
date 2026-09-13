#include "MiniRegistry.h"

#include "glaze/producer/ParseError.h"
#include "glaze/producer/RegistryLoader.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>

using glaze::producer::ParseError;
using glaze::producer::RegistryLoader;
using glaze::testfixtures::kMiniXml;

namespace {

std::filesystem::path writeTempXml() {
    auto path = std::filesystem::temp_directory_path() / "glaze-mini.xml";
    std::ofstream out{path, std::ios::binary | std::ios::trunc};
    out.write(kMiniXml.data(), static_cast<std::streamsize>(kMiniXml.size()));
    return path;
}

} // namespace

TEST_CASE("loadFromFile parses a file identical to loadFromString",
          "[glaze][producer][loader]") {
    const auto path = writeTempXml();
    const auto registry = RegistryLoader{}.loadFromFile(path.string());
    REQUIRE(registry.types().size() == 9);
    REQUIRE(registry.features().size() == 6);
    REQUIRE(registry.extensions().size() == 4);
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

TEST_CASE("loadFromFile throws ParseError on missing file", "[glaze][producer][loader][errors]") {
    REQUIRE_THROWS_AS(
        RegistryLoader{}.loadFromFile("Z:\\definitely\\not\\here\\gl.xml"),
        ParseError);
}
