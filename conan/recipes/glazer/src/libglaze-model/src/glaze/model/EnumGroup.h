#pragma once

#include "glaze/model/Enum.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace glaze::model {

/**
 * @brief One <enums> block from gl.xml. Mirrors glaze/model.py::EnumGroup.
 * Extended description: represents an <enums> container element with its
 * namespace, optional group/type attributes, and the enums declared inside.
 * The enums vector preserves gl.xml declaration order — iteration matches
 * Python's insertion-ordered dict so generator output stays deterministic.
 */
struct EnumGroup {
    //! namespace attribute ("GL" in practice)
    std::string namespaceName;
    //! optional group name attribute (e.g. "BufferTarget")
    std::optional<std::string> group;
    //! optional type attribute: "enum" or "bitmask"
    std::optional<std::string> enumGroupType;
    //! (name, enum) pairs in declaration order
    std::vector<std::pair<std::string, Enum>> enums;
    //! optional vendor attribute (e.g. "ARB", "NV")
    std::optional<std::string> vendor;
    //! optional comment attribute
    std::optional<std::string> comment;
};

/**
 * @brief Find an enum inside an EnumGroup by name.
 * @param group the group to search
 * @param name the GL_* constant name to look up
 * @return pointer to the Enum, or nullptr if not found
 */
const Enum *findEnum(const EnumGroup &group, std::string_view name) noexcept;

} // namespace glaze::model
