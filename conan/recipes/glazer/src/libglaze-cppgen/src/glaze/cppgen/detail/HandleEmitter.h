#pragma once

#include "glaze/cppgen/detail/FunctorEmitter.h"
#include "glaze/model/ConsolidatedRequire.h"
#include "glaze/model/Registry.h"

#include <nlohmann/json.hpp>

namespace glaze::cppgen::detail {

/**
 * @brief Build the legacy handle-wrapper class contexts.
 * Extended description: for each discovered handle class, groups every core
 * NON-Named* command whose first parameter is that class into a method on
 * the wrapper. Mirrors cpp_generator.py::_build_handle_classes. Method
 * bodies delegate to the corresponding functor, so strong return types
 * (e.g. UniformLocation) propagate automatically.
 */
nlohmann::json buildHandleClasses(const model::Registry &registry,
                                  const model::ConsolidatedRequire &consolidated,
                                  const EmitterContext &ctx);

} // namespace glaze::cppgen::detail
