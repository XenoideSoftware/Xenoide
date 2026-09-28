#ifndef XE_CMAKE_IO_BUILD_TREE_READER_H
#define XE_CMAKE_IO_BUILD_TREE_READER_H

#include "FileSystem.h"

#include "xe/cmake/analysis/DirectedDependencyGraph.h"

#include <optional>
#include <string>
#include <string_view>

namespace xe::cmake::io {

    // Reads CMake build-tree facts (trace json-v1 and File API codemodel-v2) into
    // a semantic model and directed dependency graph. Uses nlohmann_json.
    class BuildTreeReader {
    public:
        explicit BuildTreeReader(const IFileSystem &filesystem) : filesystem_(filesystem) {
        }

        // Reads a CMake trace produced with --trace-format=json-v1 and appends the
        // discovered target_link edges to the graph.
        void read_trace(std::string_view trace_path, xe::cmake::analysis::DirectedDependencyGraph &graph) const;

        // Reads a File API codemodel-v2 reply directory and appends the discovered
        // targets and link edges to the graph.
        void read_codemodel(std::string_view codemodel_dir, xe::cmake::analysis::DirectedDependencyGraph &graph) const;

    private:
        const IFileSystem &filesystem_;
    };

} // namespace xe::cmake::io

#endif // XE_CMAKE_IO_BUILD_TREE_READER_H