#pragma once

#include <optional>
#include <string>
#include <vector>

namespace glaze::model {

/**
 * @brief One command parameter. Mirrors glaze/model.py::CommandParam.
 * Extended description: represents a <param> child inside a <command> block.
 * The type is kept as a raw token list exactly as it appears in gl.xml
 * (e.g. {"const", "GLuint", "*"}) so downstream code can reason about const,
 * pointer indirection, and the base type in the same way the Python code
 * does through its @property helpers.
 */
struct CommandParam {
    //! parameter name as declared in gl.xml
    std::string name;
    //! raw type tokens in declaration order (e.g. {"const", "GLuint", "*"})
    std::vector<std::string> typeParts;
    //! optional "group" attribute (enum group, e.g. "BufferTarget")
    std::optional<std::string> group;
    //! optional "class" attribute (GL object class, e.g. "buffer", "program")
    std::optional<std::string> classStr;
    //! optional "len" attribute (parameter length descriptor)
    std::optional<std::string> len;
};

/**
 * @brief Extract the base GL type name from a parameter's typeParts.
 * Mirrors glaze/model.py::CommandParam.type.
 * @param param the parameter to inspect
 * @return the first token that is not "const", "*", "**", or "void"; empty if none
 */
std::optional<std::string> baseType(const CommandParam &param) noexcept;

/**
 * @brief Count the total number of '*' characters across all typeParts tokens.
 * Mirrors glaze/model.py::CommandParam.pointer_indirection.
 */
int pointerIndirection(const CommandParam &param) noexcept;

/**
 * @brief True if "const" appears in typeParts.
 * Mirrors glaze/model.py::CommandParam.is_const.
 */
bool isConst(const CommandParam &param) noexcept;

/**
 * @brief True if pointerIndirection() > 0.
 * Mirrors glaze/model.py::CommandParam.is_pointer.
 */
bool isPointer(const CommandParam &param) noexcept;

/**
 * @brief True if "void" appears in typeParts.
 * Mirrors glaze/model.py::CommandParam.is_void.
 */
bool isVoid(const CommandParam &param) noexcept;

/**
 * @brief True if the parameter has a non-empty "group" attribute.
 * Mirrors glaze/model.py::CommandParam.has_group.
 */
bool hasGroup(const CommandParam &param) noexcept;

/**
 * @brief True if the parameter has a non-empty "class" attribute.
 * Mirrors glaze/model.py::CommandParam.has_class.
 */
bool hasClass(const CommandParam &param) noexcept;

} // namespace glaze::model
