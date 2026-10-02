#ifndef XE_CMAKE_IO_DEFAULT_CMAKE_PARSER_H
#define XE_CMAKE_IO_DEFAULT_CMAKE_PARSER_H

#include "FileSystem.h"
#include "ICMakeParser.h"

namespace xe::cmake::io {

    // Concrete ICMakeParser backed by the in-house minimal Lexer/CST parser. This
    // is the current default; a future OfficialCmakeParser can replace it behind
    // the same interface.
    class DefaultCmakeParser final : public ICMakeParser {
    public:
        explicit DefaultCmakeParser(const IFileSystem &filesystem) : filesystem_(filesystem) {
        }

        xe::cmake::core::ConcreteSyntaxTree parse_listfile(std::string_view file_path, std::string_view source) const override;

        std::vector<std::string> discover_listfiles(std::string_view directory) const override;

    private:
        const IFileSystem &filesystem_;
    };

} // namespace xe::cmake::io

#endif // XE_CMAKE_IO_DEFAULT_CMAKE_PARSER_H