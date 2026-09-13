#include "glaze/codegen/TypeMapper.h"

#include <sstream>

namespace glaze::codegen {

namespace {

//! Join a parameter's raw typeParts tokens back into a C declaration string.
//! The Python parser preserves whitespace between tokens loosely, so we
//! rebuild it by inserting a space between all non-"*" tokens, and gluing
//! the "*" onto the preceding token. The result matches what the C generator
//! produces today for typedefs like PFNGLCLEARPROC and parameter lists.
std::string joinTypeParts(const model::CommandParam &param) {
    std::string out;
    for (std::size_t i = 0; i < param.typeParts.size(); ++i) {
        const auto &part = param.typeParts[i];
        if (part.empty()) {
            continue;
        }
        if (!out.empty() && part != "*" && part != "**") {
            out.push_back(' ');
        }
        out.append(part);
    }
    return out;
}

} // namespace

std::string DefaultCTypeMapper::mapParamDecl(const model::CommandParam &param,
                                             const model::Command & /*command*/) const {
    auto decl = joinTypeParts(param);
    if (!decl.empty()) {
        decl.push_back(' ');
    }
    decl.append(param.name);
    return decl;
}

std::string DefaultCTypeMapper::mapReturnType(const model::Command &command) const {
    return command.returnTypeStr;
}

std::string DefaultCTypeMapper::mapBaseType(std::string_view glType) const {
    return std::string{glType};
}

} // namespace glaze::codegen
