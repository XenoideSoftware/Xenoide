#pragma once

#include <string>

namespace glaze::model {

/**
 * @brief Parsed C type declaration fragment. Mirrors glaze/model.py::TypeDecl.
 * Extended description: a minimal three-field struct describing a C type as
 * parsed from gl.xml proto/param entries — a base type name plus optional
 * const and pointer modifiers. Used primarily for command return types.
 */
struct TypeDecl {
    //! base GL type name (e.g. "GLuint", "void", "GLboolean")
    std::string name;
    //! true when the 'const' modifier appears in the declaration
    bool isConst = false;
    //! true when a '*' modifier appears in the declaration
    bool isPointer = false;
};

/**
 * @brief Render a TypeDecl back to its C spelling.
 * Mirrors glaze/model.py::TypeDecl.to_c_string.
 * @param decl the declaration to render
 * @return a string like "const GLuint*", "void", etc.
 */
std::string toCString(const TypeDecl &decl);

} // namespace glaze::model
