#pragma once

#include "glaze/model/CommandParam.h"
#include "glaze/model/TypeDecl.h"

#include <optional>
#include <string>
#include <vector>

namespace glaze::model {

/**
 * @brief A single GL function. Mirrors glaze/model.py::Command.
 * Extended description: represents a <command> element with its proto
 * (return type + name) and params. The namespaceName field is "GL" in
 * practice but kept explicit so future non-GL APIs stay describable.
 */
struct Command {
    //! full GL function name (e.g. "glBindBuffer")
    std::string name;
    //! structured return type (name + const/pointer flags)
    TypeDecl returnType;
    //! return type as a whitespace-joined token list (e.g. "const GLubyte *")
    std::string returnTypeStr;
    //! parameters in declaration order
    std::vector<CommandParam> params;
    //! command namespace attribute ("GL" in practice)
    std::string namespaceName = "GL";
    //! optional "group" attribute (rarely set)
    std::optional<std::string> group;
};

/**
 * @brief Return the "class" attribute of the first parameter, if any.
 * Mirrors glaze/model.py::Command.get_class.
 * @param command the command to inspect
 * @return the first param's class attribute or nullopt if missing
 */
std::optional<std::string> getClass(const Command &command);

} // namespace glaze::model
