#pragma once

#include "glaze/codegen/NameTransform.h"
#include "glaze/model/Enum.h"

#include <nlohmann/json.hpp>

#include <set>
#include <string>
#include <vector>

namespace glaze::cppgen::detail {

/**
 * @brief Build a JSON context describing a single C++ enum class.
 * Extended description: produces {group_name, base_type, is_bitmask, entries:
 * [{name, value}]} where each entry name has been passed through
 * NameTransform::transformEnumEntry. Mirrors the data consumed by the
 * gl.hpp.inja template's enum class loop.
 *
 * @param groupName the raw XML group name
 * @param cleanGroupName the post-rename C++ enum class name
 * @param isBitmask true when the enum group was declared type="bitmask"
 * @param enums the enums selected for this group (already filtered to the union)
 * @param nameTransform the naming policy to apply to each enum entry
 */
nlohmann::json buildEnumClassContext(const std::string &groupName,
                                     const std::string &cleanGroupName,
                                     bool isBitmask,
                                     const std::vector<const model::Enum *> &enums,
                                     const codegen::NameTransform &nameTransform);

} // namespace glaze::cppgen::detail
