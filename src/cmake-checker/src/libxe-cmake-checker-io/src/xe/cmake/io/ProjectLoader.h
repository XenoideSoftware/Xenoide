#ifndef XE_CMAKE_IO_PROJECT_LOADER_H
#define XE_CMAKE_IO_PROJECT_LOADER_H

#include "FileSystem.h"
#include "ICMakeParser.h"

#include "xe/cmake/core/ConcreteSyntaxTree.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace xe::cmake::io {

    // Discovers and parses all listfiles of a CMake project. Uses constructor
    // dependency injection over IFileSystem and ICMakeParser for testability.
    class ProjectLoader {
    public:
        explicit ProjectLoader(const IFileSystem &filesystem, const ICMakeParser &parser) : filesystem_(filesystem), parser_(parser) {
        }

        // Returns the project listfiles (CMakeLists.txt and *.cmake modules) under
        // the given directory, parsed into concrete syntax trees.
        std::vector<xe::cmake::core::ConcreteSyntaxTree> load_project(std::string_view directory) const;

        // Loads a single listfile by path.
        std::optional<xe::cmake::core::ConcreteSyntaxTree> load_listfile(std::string_view path) const;

    private:
        const IFileSystem &filesystem_;
        const ICMakeParser &parser_;
    };

} // namespace xe::cmake::io

#endif // XE_CMAKE_IO_PROJECT_LOADER_H