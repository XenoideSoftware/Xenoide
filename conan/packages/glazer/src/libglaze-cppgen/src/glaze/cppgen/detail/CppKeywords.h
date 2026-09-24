#pragma once

#include <string>
#include <string_view>
#include <unordered_set>

namespace glaze::cppgen::detail {

/**
 * @brief Set of C++ keywords that must never appear as a bare method name.
 *
 * Mirrors the `_CPP_KEYWORDS` frozenset in
 * `conan/packages/glaze/glaze/generators/cpp_generator.py`. Any name
 * derived from a GL command that collides with one of these gets a
 * trailing underscore appended so the emitted header still compiles.
 */
const std::unordered_set<std::string> &cppKeywords();

//! Return `name + "_"` when `name` is a reserved C++ keyword, otherwise
//! return `name` unchanged. The trailing-underscore convention matches
//! the Python generator's handling of keyword-colliding members.
std::string sanitizeMethodName(std::string name);

//! Non-mutating overload for callers that already hold a string_view.
std::string sanitizeMethodName(std::string_view name);

} // namespace glaze::cppgen::detail
