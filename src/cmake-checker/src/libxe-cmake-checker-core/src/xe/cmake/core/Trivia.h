#ifndef XE_CMAKE_CORE_TRIVIA_H
#define XE_CMAKE_CORE_TRIVIA_H

#include "SourceSpan.h"

#include <cstdint>

namespace xe::cmake::core {

    // Lossless trivia attached to statements: comments, blank lines and stray
    // whitespace that does not belong to any command.
    enum class TriviaKind : uint8_t {
        Comment,
        Whitespace,
        Newline,
        BlankLine,
    };

    struct TriviaNode {
        TriviaKind kind = TriviaKind::Whitespace;
        SourceSpan span;
    };

} // namespace xe::cmake::core

#endif // XE_CMAKE_CORE_TRIVIA_H