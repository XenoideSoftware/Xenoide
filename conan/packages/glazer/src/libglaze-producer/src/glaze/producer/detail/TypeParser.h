#pragma once

#include "glaze/model/Type.h"

#include <pugixml.hpp>

#include <vector>

namespace glaze::producer::detail {

/**
 * @brief Parse every <type> child of the <types> block inside a <registry>.
 * Mirrors glaze/parser.py::RegistryParser._parse_types.
 * @param registry the root <registry> node
 * @return parsed Type list, empty if no <types> block
 */
std::vector<glaze::model::Type> parseTypes(const pugi::xml_node &registry);

} // namespace glaze::producer::detail
