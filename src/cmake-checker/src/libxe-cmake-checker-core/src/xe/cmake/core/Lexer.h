#ifndef XE_CMAKE_CORE_LEXER_H
#define XE_CMAKE_CORE_LEXER_H

#include "ConcreteSyntaxTree.h"

#include <string_view>

namespace xe::cmake::core {

    // A minimal, lossless CMake lexer/parser. It recognizes commands of the form
    //
    //   name(arg1 arg2 ...)
    //
    // together with their arguments (raw, quoted, or bracket quoted) and preserves
    // all trivia (comments, blank lines, whitespace) between statements.
    class Lexer {
    public:
        // Parses the given listfile buffer into a lossless concrete syntax tree.
        // The file_path is stored verbatim on every produced command.
        static ConcreteSyntaxTree parse(std::string_view file_path, std::string_view source);

    private:
        Lexer() = delete;
    };

} // namespace xe::cmake::core

#endif // XE_CMAKE_CORE_LEXER_H