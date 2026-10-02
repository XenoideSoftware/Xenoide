#pragma once

#include "glaze/cppgen/detail/FunctorEmitter.h"
#include "glaze/model/Command.h"

#include <nlohmann/json.hpp>

namespace glaze::cppgen::detail {

/**
 * @brief Builds the specialized operator() overloads that wrap a canonical
 *        functor with more ergonomic C++ shapes.
 *
 * Ports the six overload-context helpers from `cpp_generator.py` that turn
 * a single GL command into additional C++ overloads:
 *
 *   - `_string_overload_context`            → std::string return for
 *     commands with (GLchar*, GLsizei*) buffer-and-length output params.
 *   - `_scalar_query_overload_context`      → drops the trailing scalar
 *     output pointer and returns the scalar by value.
 *   - `_infolog_selfquery_overload_context` → zero-extra-arg std::string
 *     overload for glGetShaderInfoLog / glGetProgramInfoLog that queries
 *     GL_INFO_LOG_LENGTH internally via the paired Get*iv.
 *   - `_array_view_overload_context`        → template<T> overload
 *     accepting a const ArrayView<T>& for data-upload commands.
 *   - `_single_object_creation_context`     → singular helper for
 *     multi-object creators (glGenBuffers → genBuffer()).
 *   - `_single_object_deletion_context`     → singular helper for
 *     multi-object deleters (glDeleteBuffers → deleteBuffer(obj)).
 *
 * Each overload is returned as a JSON object shaped identically to the
 * canonical overload produced by FunctorEmitter (return_type, params_str,
 * call_args_str, body, …) so the template can emit them with a single
 * `{% for ov in fn.overloads %}` loop.
 */
nlohmann::json buildExtraOverloads(const model::Command &command,
                                   const EmitterContext &ctx);

} // namespace glaze::cppgen::detail
