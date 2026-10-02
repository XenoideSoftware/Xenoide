#include "CstSyntheticGenerator.h"

#include "xe/cmake/core/Lexer.h"

namespace xe::cmake::testing {

    xe::cmake::core::ConcreteSyntaxTree parse_cst(std::string_view path, std::string_view source) {
        return xe::cmake::core::Lexer::parse(path, source);
    }

} // namespace xe::cmake::testing