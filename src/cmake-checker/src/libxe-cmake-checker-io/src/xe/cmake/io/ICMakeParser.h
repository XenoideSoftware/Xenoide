#ifndef XE_CMAKE_IO_ICMAKE_PARSER_H
#define XE_CMAKE_IO_ICMAKE_PARSER_H

#include "xe/cmake/core/ConcreteSyntaxTree.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace xe::cmake::io {

    // Narrow parser/loader abstraction. ProjectLoader and BuildTreeReader depend
    // on this interface so the concrete parsing strategy can be swapped (e.g. a
    // future OfficialCmakeParser vendored from upstream CMake) without touching
    // downstream consumers.
    class ICMakeParser {
    public:
        virtual ~ICMakeParser() = default;

        // Parses a single listfile buffer into a lossless concrete syntax tree.
        virtual xe::cmake::core::ConcreteSyntaxTree parse_listfile(std::string_view file_path, std::string_view source) const = 0;

        // Discovers listfile paths under the given directory. Returns them in a
        // deterministic order (sorted).
        virtual std::vector<std::string> discover_listfiles(std::string_view directory) const = 0;
    };

} // namespace xe::cmake::io

#endif // XE_CMAKE_IO_ICMAKE_PARSER_H