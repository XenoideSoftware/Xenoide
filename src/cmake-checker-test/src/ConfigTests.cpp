#include <catch2/catch_test_macros.hpp>

#include <string>

#include <cmcheck/Config.h>
#include <cmcheck/ConfigLoader.h>

using namespace cmcheck;

TEST_CASE("Config loads line_length, indent, rules and exclude globs", "[config]") {
    const std::string yaml = R"(
line_length: 200
indent: 4
rules:
  R1.one_target_per_folder: error
  L3.line_length: off
  L6.no_commented_out_code: warn
exclude:
  - "src/ide/**"
  - "build*/**"
)";

    const Config config = loadConfigFromString(yaml, "/repo");

    CHECK(config.line_length == 200);
    CHECK(config.indent == 4);
    CHECK(config.rule_severities.at("R1.one_target_per_folder") == Severity::Error);
    CHECK(config.rule_severities.at("L3.line_length") == Severity::Off);
    CHECK(config.rule_severities.at("L6.no_commented_out_code") == Severity::Warn);
    REQUIRE(config.exclude_globs.size() == 2);
    CHECK(config.exclude_globs[0] == "src/ide/**");
}

TEST_CASE("Config defaults apply when YAML is empty", "[config]") {
    const Config config = loadConfigFromString("", "/repo");
    CHECK(config.line_length == 180);
    CHECK(config.indent == 4);
    CHECK(config.rule_severities.empty());
    CHECK(config.exclude_globs.empty());
}

TEST_CASE("Config tolerates malformed YAML", "[config]") {
    const Config config = loadConfigFromString("line_length: [unclosed", "/repo");
    CHECK(config.line_length == 180);
    CHECK(config.exclude_globs.empty());
}

TEST_CASE("Exclude globs match absolute and relative paths", "[config]") {
    Config config = loadConfigFromString("exclude:\n  - \"src/ide/**\"\n  - \"build*/**\"", "/repo");

    CHECK(isExcluded(config, "/repo/src/ide/widgets/CMakeLists.txt"));
    CHECK(isExcluded(config, "/repo/src/ide/CMakeLists.txt"));
    CHECK(isExcluded(config, "/repo/build-cmake-check/Release/trace.json"));
    CHECK_FALSE(isExcluded(config, "/repo/src/engine/CMakeLists.txt"));
    CHECK_FALSE(isExcluded(config, "/repo/src/idefault/CMakeLists.txt"));
}