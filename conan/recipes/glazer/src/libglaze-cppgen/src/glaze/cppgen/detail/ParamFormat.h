#pragma once

#include "glaze/codegen/NameTransform.h"
#include "glaze/model/Command.h"
#include "glaze/model/CommandParam.h"

#include <map>
#include <set>
#include <string>

namespace glaze::cppgen::detail {

/**
 * @brief Render a command parameter as a C++ type spelling.
 * Extended description: mirrors the substitution pass from
 * cpp_generator.py::_param_type_str. Replaces:
 *   - "GLuint" with the strong handle type when the param has class_="buffer"/...
 *   - "GLenum" with the enum class (or Flags<E> for bitmask groups)
 *   - "GLint"/"location" with UniformLocation when matched
 * The remaining tokens fall through unchanged.
 *
 * @param param the parameter to render
 * @param command the parent command (used for location-param detection)
 * @param handleClasses map from gl class_ name to C++ handle type name
 * @param groupRename map from raw group name to clean C++ enum class name
 * @param bitmaskGroups set of group names that were declared type="bitmask"
 * @return a C++ type spelling, e.g. "BufferId", "Flags<ClearBufferMask>", etc.
 */
std::string paramTypeStr(const model::CommandParam &param,
                         const model::Command &command,
                         const std::map<std::string, std::string> &handleClasses,
                         const std::map<std::string, std::string> &groupRename,
                         const std::set<std::string> &bitmaskGroups,
                         const std::set<std::string> &emittedGroups);

/**
 * @brief Render a full parameter declaration: "<type> <name>".
 * Mirrors cpp_generator.py::_generate_param_decl.
 */
std::string generateParamDecl(const model::CommandParam &param,
                              const model::Command &command,
                              const std::map<std::string, std::string> &handleClasses,
                              const std::map<std::string, std::string> &groupRename,
                              const std::set<std::string> &bitmaskGroups,
                              const std::set<std::string> &emittedGroups);

/**
 * @brief Render the call argument expression for passing a wrapped parameter
 *        back to the underlying C function pointer. Strong handle types are
 *        extracted via .id, enum classes via static_cast<GLenum>, flags via
 *        .value(). Mirrors cpp_generator.py::_generate_call_arg.
 */
std::string generateCallArg(const model::CommandParam &param,
                            const model::Command &command,
                            const std::map<std::string, std::string> &handleClasses,
                            const std::map<std::string, std::string> &groupRename,
                            const std::set<std::string> &bitmaskGroups,
                            const std::set<std::string> &emittedGroups);

/**
 * @brief Render the return type for a command as a C++ type spelling.
 * For commands that return a handle (GLuint) via a Create* function, substitutes
 * the handle type. For location returns, substitutes Uniform/AttribLocation.
 * For string returns, substitutes std::string. Otherwise renders the raw C type.
 */
std::string returnTypeStr(const model::Command &command,
                          const std::map<std::string, std::string> &handleClasses);

} // namespace glaze::cppgen::detail
