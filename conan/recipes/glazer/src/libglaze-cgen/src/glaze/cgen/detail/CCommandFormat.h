#pragma once

#include "glaze/codegen/TypeMapper.h"
#include "glaze/model/Command.h"

#include <string>

namespace glaze::cgen::detail {

/**
 * @brief Return the PFN*PROC typedef name for a GL command.
 * Mirrors c_generator.py::_command_ptr_type_name.
 * @example "glClear" -> "PFNGLCLEARPROC"
 */
std::string commandPtrTypeName(const std::string &commandName);

/**
 * @brief Build the typedef line for a command's function pointer type.
 * Mirrors c_generator.py::_generate_command_ptr_typedef.
 */
std::string generateCommandPtrTypedef(const model::Command &command,
                                      const codegen::TypeMapper &typeMapper);

/**
 * @brief Build the extern declaration for the raw glaze_* function pointer.
 * @example "extern GLAZE_API PFNGLCLEARPROC glaze_glClear;"
 */
std::string generateCommandPtrExternRaw(const model::Command &command);

/**
 * @brief Build the debug wrapper function declaration.
 * Mirrors c_generator.py::_generate_debug_wrapper_decl.
 */
std::string generateDebugWrapperDecl(const model::Command &command,
                                     const codegen::TypeMapper &typeMapper);

} // namespace glaze::cgen::detail
