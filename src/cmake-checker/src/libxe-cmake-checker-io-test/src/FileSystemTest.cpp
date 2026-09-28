#include "xe/cmake/io/FileSystem.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

namespace xe::cmake::io {

    TEST_CASE("InMemoryFileSystem write then read round-trips") {
        InMemoryFileSystem fs;
        std::string content;
        REQUIRE_FALSE(fs.exists("/v/a.txt"));
        REQUIRE(fs.write_file("/v/a.txt", "hello"));
        REQUIRE(fs.exists("/v/a.txt"));
        REQUIRE(fs.read_file("/v/a.txt", content));
        REQUIRE(content == "hello");
    }

    TEST_CASE("InMemoryFileSystem directories") {
        InMemoryFileSystem fs;
        REQUIRE(fs.create_directories("/v/lib"));
        REQUIRE(fs.is_directory("/v/lib"));
        REQUIRE_FALSE(fs.is_file("/v/lib"));
        REQUIRE_FALSE(fs.is_directory("/v/missing"));
    }

    TEST_CASE("InMemoryFileSystem lists directory entries") {
        InMemoryFileSystem fs;
        fs.create_directories("/v");
        fs.write_file("/v/CMakeLists.txt", "");
        fs.write_file("/v/src/a.cpp", "");
        fs.create_directories("/v/lib");
        const std::vector<std::string> entries = fs.list_directory("/v");
        REQUIRE(entries.size() == 3);
    }

    TEST_CASE("InMemoryFileSystem cannot read a missing file") {
        InMemoryFileSystem fs;
        std::string content;
        REQUIRE_FALSE(fs.read_file("/nope.txt", content));
    }

    TEST_CASE("NativeFileSystem reflects disk state") {
        NativeFileSystem fs;
        REQUIRE(fs.exists("/tmp"));
        REQUIRE(fs.is_directory("/tmp"));
    }

    TEST_CASE("NativeFileSystem read/write round-trips to a real file") {
        NativeFileSystem fs;
        const std::string path = "/tmp/xe-cmake-fs-test.bin";
        std::error_code ec;
        std::filesystem::remove(std::filesystem::path(path), ec);
        REQUIRE(fs.write_file(path, "hello native"));
        std::string content;
        REQUIRE(fs.read_file(path, content));
        REQUIRE(content == "hello native");
        REQUIRE(fs.is_file(path));
        std::filesystem::remove(std::filesystem::path(path), ec);
    }

    TEST_CASE("NativeFileSystem create_directories and list") {
        NativeFileSystem fs;
        const std::string dir = "/tmp/xe-cmake-fs-dir";
        std::error_code ec;
        std::filesystem::remove_all(std::filesystem::path(dir), ec);
        REQUIRE(fs.create_directories(dir));
        fs.write_file(dir + "/a.txt", "");
        fs.write_file(dir + "/b.txt", "");
        const std::vector<std::string> entries = fs.list_directory(dir);
        REQUIRE(entries.size() == 2);
        std::filesystem::remove_all(std::filesystem::path(dir), ec);
    }

    TEST_CASE("NativeFileSystem read missing file returns false") {
        NativeFileSystem fs;
        std::string content;
        REQUIRE_FALSE(fs.read_file("/nonexistent-xe-cmake-file", content));
    }

    TEST_CASE("NativeFileSystem missing file is neither file nor dir") {
        NativeFileSystem fs;
        REQUIRE_FALSE(fs.is_file("/nonexistent-xe-cmake-file"));
        REQUIRE_FALSE(fs.is_directory("/nonexistent-xe-cmake-file"));
        REQUIRE_FALSE(fs.exists("/nonexistent-xe-cmake-file"));
    }

} // namespace xe::cmake::io