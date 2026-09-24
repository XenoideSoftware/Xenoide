#pragma once

#include "glaze/model/EnumGroup.h"

#include <pugixml.hpp>

#include <vector>

namespace glaze::producer::detail {

/**
 * @brief Parse every <enums> block directly under the <registry> root.
 * Mirrors glaze/parser.py::RegistryParser._parse_enum_groups.
 * @param registry the root <registry> node
 * @return parsed EnumGroup list, in document order
 */
std::vector<glaze::model::EnumGroup> parseEnumGroups(const pugi::xml_node &registry);

} // namespace glaze::producer::detail
