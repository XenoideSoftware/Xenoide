#pragma once

#include <map>
#include <set>
#include <string>

namespace glaze::cppgen::detail {

/**
 * @brief Build a mapping from raw XML group names to C++ enum class names.
 * Mirrors glaze/generators/cpp_generator.py::_build_group_rename.
 * Extended description: iterates a list of known vendor suffixes (ARB, KHR,
 * EXT, NV, ...) and strips the first one that leaves a non-colliding "clean"
 * name behind. Names that would collide with another group in groupSet are
 * left unchanged. The longest vendor suffix that matches wins first so
 * "INTEL" beats any shorter embedded prefix.
 *
 * @param groupSet every group name currently in use — used for collision detection
 * @return old_name -> clean_name entries for every rename; groups with no
 *         suffix or a colliding strip are absent from the map
 */
std::map<std::string, std::string> buildGroupRename(const std::set<std::string> &groupSet);

/**
 * @brief Resolve a group name through the rename map, returning the
 *        clean C++ enum class name or the input unchanged when no rename exists.
 */
std::string renamedGroup(const std::map<std::string, std::string> &renameMap,
                         const std::string &groupName);

} // namespace glaze::cppgen::detail
