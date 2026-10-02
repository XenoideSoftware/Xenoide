#pragma once

#include <set>
#include <string>

namespace glaze::model {

/**
 * @brief Flat sets of enum/command names for a specific API version.
 * Mirrors glaze/model.py::ConsolidatedRequire.
 * Extended description: produced by Registry::consolidate after flattening
 * all <feature> requires up to a given version and applying any <remove>
 * entries whose profile is not in ApiProfile::skipRemoveProfiles. The sets
 * are used by generators to filter which commands and enums to emit.
 */
struct ConsolidatedRequire {
    //! set of enum constant names active at the target version
    std::set<std::string> enums;
    //! set of command names active at the target version
    std::set<std::string> commands;
};

} // namespace glaze::model
