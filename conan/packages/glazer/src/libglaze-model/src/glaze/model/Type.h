#pragma once

#include <optional>
#include <string>

namespace glaze::model {

/**
 * @brief One <type> entry from gl.xml. Mirrors glaze/model.py::Type.
 * Extended description: represents a typedef/struct declaration block as
 * parsed from the <types> section of the Khronos registry. The cDefinition
 * field holds the full C source text (e.g. "typedef unsigned int GLuint;")
 * and is emitted verbatim by the C generator into its <types> output block.
 */
struct Type {
    //! identifier (e.g. "GLuint")
    std::string name;
    //! full C source text of the type declaration
    std::string cDefinition;
    //! optional "requires" attribute pointing at another type
    std::optional<std::string> requiresName;
    //! optional comment attribute
    std::optional<std::string> comment;
};

} // namespace glaze::model
