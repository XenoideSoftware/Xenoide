#include "xe/cmake/io/FileSystem.h"
#include "xe/cmake/io/SyncWriter.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake::io {

    TEST_CASE("SyncWriter applies edits to the in-memory filesystem") {
        InMemoryFileSystem fs;
        fs.write_file("/v/CMakeLists.txt", "set (target \"old\")\n");
        xe::cmake::core::WorkspaceEdit workspace;
        workspace.add_edit("/v/CMakeLists.txt", xe::cmake::core::TextEdit::replace(xe::cmake::core::SourceSpan{13, 16, 1, 1, 1, 1}, "new"));
        const SyncWriter writer(fs);
        const std::vector<std::string> written = writer.apply(workspace);
        REQUIRE(written.size() == 1);
        std::string content;
        REQUIRE(fs.read_file("/v/CMakeLists.txt", content));
        REQUIRE(content == "set (target \"new\")\n");
    }

    TEST_CASE("SyncWriter skips files with no content change") {
        InMemoryFileSystem fs;
        fs.write_file("/v/CMakeLists.txt", "set (target \"same\")\n");
        xe::cmake::core::WorkspaceEdit workspace;
        // An edit that produces identical content must not be written.
        workspace.add_edit("/v/CMakeLists.txt", xe::cmake::core::TextEdit::replace(xe::cmake::core::SourceSpan{13, 17, 1, 1, 1, 1}, "same"));
        const SyncWriter writer(fs);
        const std::vector<std::string> written = writer.apply(workspace);
        REQUIRE(written.empty());
    }

    TEST_CASE("SyncWriter skips files that do not exist") {
        InMemoryFileSystem fs;
        xe::cmake::core::WorkspaceEdit workspace;
        workspace.add_edit("/missing/CMakeLists.txt", xe::cmake::core::TextEdit::replace(xe::cmake::core::SourceSpan{0, 1, 1, 1, 1, 1}, "x"));
        const SyncWriter writer(fs);
        REQUIRE(writer.apply(workspace).empty());
    }

    TEST_CASE("SyncWriter applies multi-file edits atomically") {
        InMemoryFileSystem fs;
        fs.write_file("/a/CMakeLists.txt", "aaa");
        fs.write_file("/b/CMakeLists.txt", "bbb");
        xe::cmake::core::WorkspaceEdit workspace;
        workspace.add_edit("/a/CMakeLists.txt", xe::cmake::core::TextEdit::replace(xe::cmake::core::SourceSpan{0, 3, 1, 1, 1, 1}, "A"));
        workspace.add_edit("/b/CMakeLists.txt", xe::cmake::core::TextEdit::replace(xe::cmake::core::SourceSpan{0, 3, 1, 1, 1, 1}, "B"));
        const SyncWriter writer(fs);
        REQUIRE(writer.apply(workspace).size() == 2);
    }

} // namespace xe::cmake::io