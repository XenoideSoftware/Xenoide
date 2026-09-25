#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>

#include <cmcheck/FileApiParser.h>

using namespace cmcheck;

namespace {

    struct Fixture {
        std::filesystem::path root;
        std::filesystem::path replyDir;

        Fixture() {
            root = std::filesystem::temp_directory_path() / "cmcheck-test";
            std::filesystem::remove_all(root);
            std::filesystem::create_directories(root / "project");
            replyDir = root / "project" / "build" / ".cmake" / "api" / "v1" / "reply";
            std::filesystem::create_directories(replyDir);
        }

        ~Fixture() {
            std::error_code error;
            std::filesystem::remove_all(root, error);
        }

        void write(const std::filesystem::path &relative, const std::string &content) {
            const std::filesystem::path full = root / "project" / relative;
            std::filesystem::create_directories(full.parent_path());
            std::ofstream stream(full);
            stream << content;
        }

        void writeReply(const std::string &name, const std::string &content) {
            std::ofstream stream(replyDir / name);
            stream << content;
        }
    };

    std::string json(const std::string &value) {
        return "\"" + value + "\"";
    }

} // namespace

TEST_CASE("FileApiParser builds folders, targets and listfiles from replies", "[fileapi]") {
    Fixture fixture;
    const std::string projectSource = (fixture.root / "project").string();

    fixture.write("CMakeLists.txt", "project(xe)\n");
    fixture.write("libxe-core/CMakeLists.txt", "add_library(xe-core src/xe-core.cpp)\n");
    fixture.write("libxe-core/src/xe-core.cpp", "int main() {}\n");

    fixture.writeReply(
        "index-test.json",
        R"({"objects":[
        {"jsonFile":"codemodel-v2-abc.json","kind":"codemodel","version":{"major":2}},
        {"jsonFile":"cmakeFiles-v1-abc.json","kind":"cmakeFiles","version":{"major":1}}
    ]})"
    );

    fixture.writeReply(
        "codemodel-v2-abc.json",
        "{\"paths\":{\"source\":" + json(projectSource) + ",\"build\":" + json(projectSource + "/build") + "},\"configurations\":[{\"directories\":[" +
            "{\"source\":\".\",\"targetIndexes\":[]}," + "{\"source\":\"libxe-core\",\"targetIndexes\":[0]}" +
            "],\"targets\":[{\"name\":\"xe-core\",\"directoryIndex\":1,\"jsonFile\":\"target-xe-core.json\"}],\"projects\":[{\"name\":\"xe\",\"directoryIndexes\":[0,1],"
            "\"targetIndexes\":[0]}]}]}"
    );

    fixture.writeReply("target-xe-core.json", "{\"name\":\"xe-core\",\"type\":\"STATIC_LIBRARY\",\"paths\":{\"source\":\"libxe-core\"}}");

    fixture.writeReply(
        "cmakeFiles-v1-abc.json",
        "{\"paths\":{\"source\":" + json(projectSource) + "},\"inputs\":[" + "{\"path\":\"CMakeLists.txt\"}," + "{\"path\":\"libxe-core/CMakeLists.txt\"}," +
            "{\"isExternal\":true,\"path\":\"/usr/share/CMakeDetermineSystem.cmake\"}," + "{\"isGenerated\":true,\"path\":\"build/CMakeFiles/x.cmake\"}" + "]}"
    );

    const FileApiParser::Result result = FileApiParser().parse(fixture.replyDir.string());

    REQUIRE(result.error.empty());
    CHECK(result.model.root == projectSource);
    CHECK(result.model.targets.size() == 1);
    CHECK(result.model.targets[0].name == "xe-core");
    CHECK(result.model.targets[0].type == "STATIC_LIBRARY");
    CHECK(result.model.targets[0].source_dir == projectSource + "/libxe-core");
    CHECK(result.model.folders.size() == 2);
    CHECK(result.model.listfiles.size() == 2);
}

TEST_CASE("FileApiParser marks alias targets and reports missing replies", "[fileapi]") {
    Fixture fixture;
    const std::string projectSource = (fixture.root / "project").string();
    fixture.write("CMakeLists.txt", "project(xe)\n");
    fixture.write("libxe-core/CMakeLists.txt", "add_library(xe-core INTERFACE)\nadd_library(xe::core ALIAS xe-core)\n");

    fixture.writeReply(
        "index-test.json",
        R"({"objects":[
        {"jsonFile":"codemodel-v2-abc.json","kind":"codemodel","version":{"major":2}},
        {"jsonFile":"cmakeFiles-v1-abc.json","kind":"cmakeFiles","version":{"major":1}}
    ]})"
    );

    fixture.writeReply(
        "codemodel-v2-abc.json",
        "{\"paths\":{\"source\":" + json(projectSource) + "},\"configurations\":[{\"directories\":[" + "{\"source\":\".\",\"targetIndexes\":[]}," +
            "{\"source\":\"libxe-core\",\"targetIndexes\":[0,1]}" + "],\"targets\":[" + "{\"name\":\"xe-core\",\"directoryIndex\":1,\"jsonFile\":\"target-a.json\"}," +
            "{\"name\":\"xe::core\",\"directoryIndex\":1,\"jsonFile\":\"target-b.json\"}" + "],\"projects\":[]}]}"
    );

    fixture.writeReply("target-a.json", "{\"name\":\"xe-core\",\"type\":\"INTERFACE_LIBRARY\"}");
    fixture.writeReply("target-b.json", "{\"name\":\"xe::core\",\"type\":\"ALIAS_LIBRARY\"}");
    fixture.writeReply(
        "cmakeFiles-v1-abc.json",
        "{\"paths\":{\"source\":" + json(projectSource) + "},\"inputs\":[" + "{\"path\":\"CMakeLists.txt\"}," + "{\"path\":\"libxe-core/CMakeLists.txt\"}" + "]}"
    );

    const FileApiParser::Result result = FileApiParser().parse(fixture.replyDir.string());

    REQUIRE(result.error.empty());
    REQUIRE(result.model.targets.size() == 2);
    CHECK(result.model.targets[0].is_alias == false);
    CHECK(result.model.targets[1].is_alias == true);

    const FileApiParser::Result missing = FileApiParser().parse((fixture.root / "nope").string());
    CHECK_FALSE(missing.error.empty());
}