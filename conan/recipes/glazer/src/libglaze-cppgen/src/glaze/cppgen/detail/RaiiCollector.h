#pragma once

#include "glaze/cppgen/detail/FunctorEmitter.h"
#include "glaze/model/ConsolidatedRequire.h"
#include "glaze/model/Registry.h"

#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace glaze::cppgen::detail {

/**
 * @brief Walk the registry for each discovered handle class and build the
 *        creator/deleter pair descriptors consumed by the RAII Traits
 *        specialization block at the bottom of gl.hpp.inja.
 *
 * Extended description: mirrors cpp_generator.py::_collect_raii_resources in
 * spirit, simplified to the two common patterns:
 *   - Multi-object: (GLsizei n, GLuint * out) / (GLsizei n, const GLuint * in)
 *     e.g. glGenBuffers / glDeleteBuffers
 *   - Singular:     GLuint createX() / void deleteX(GLuint)
 *     e.g. glCreateProgram / glDeleteProgram
 *
 * Returns one JSON object per resource with fields:
 *   alias, handle_type, gen_gl_name, gen_func_name,
 *   delete_gl_name, delete_func_name, create_params_str, create_call_args_str.
 *
 * @param registry the Registry to scan
 * @param consolidated consolidated set of allowed commands (core feature snapshot)
 * @param handleClasses mapping from GL class_ string to C++ handle type name
 */
nlohmann::json collectRaiiResources(const model::Registry &registry,
                                    const model::ConsolidatedRequire &consolidated,
                                    const EmitterContext &ctx);

} // namespace glaze::cppgen::detail
