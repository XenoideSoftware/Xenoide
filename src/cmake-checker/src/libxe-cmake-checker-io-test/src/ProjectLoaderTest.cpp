#include "xe/cmake/io/DefaultCmakeParser.h"
#include "xe/cmake/io/FileSystem.h"
#include "xe/cmake/io/ProjectLoader.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake::io {

    TEST_CASE("ProjectLoader discovers and parses all listfiles") {
        InMemoryFileSystem fs;
        fs.create_directories("/v");
        fs.write_file("/v/CMakeLists.txt", "project (Test)\n");
        fs.write_file("/v/src/CMakeLists.txt", "set (target \"lib\")\n");
        fs.write_file("/v/src/a.cpp", "int main() {}\n");

        const DefaultCmakeParser parser(fs);
        const ProjectLoader loader(fs, parser);
        const std::vector<xe::cmake::core::ConcreteSyntaxTree> files = loader.load_project("/v");
        REQUIRE(files.size() == 2);
    }

    TEST_CASE("ProjectLoader loads a single listfile") {
        InMemoryFileSystem fs;
        fs.write_file("/v/CMakeLists.txt", "set (target \"lib\")\n");
        const DefaultCmakeParser parser(fs);
        const ProjectLoader loader(fs, parser);
        const auto tree = loader.load_listfile("/v/CMakeLists.txt");
        REQUIRE(tree.has_value());
        REQUIRE(tree->commands().size() == 1);
        REQUIRE(tree->commands()[0]->name == "set");
    }

    TEST_CASE("ProjectLoader returns nullopt for a missing file") {
        InMemoryFileSystem fs;
        const DefaultCmakeParser parser(fs);
        const ProjectLoader loader(fs, parser);
        REQUIRE_FALSE(loader.load_listfile("/missing/CMakeLists.txt").has_value());
    }

    TEST_CASE("ProjectLoader returns empty for a missing project") {
        InMemoryFileSystem fs;
        const DefaultCmakeParser parser(fs);
        const ProjectLoader loader(fs, parser);
        REQUIRE(loader.load_project("/missing").empty());
    }

} // namespace xe::cmake::io