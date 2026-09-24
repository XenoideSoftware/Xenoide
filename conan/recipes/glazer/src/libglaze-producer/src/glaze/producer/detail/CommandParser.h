#pragma once

#include "glaze/model/Command.h"

#include <pugixml.hpp>

#include <vector>

namespace glaze::producer::detail {

/**
 * @brief Parse every <command> child across all <commands> blocks.
 * Mirrors glaze/parser.py::RegistryParser._parse_commands.
 * @param registry the root <registry> node
 * @return parsed Command list, preserving document order
 */
std::vector<glaze::model::Command> parseCommands(const pugi::xml_node &registry);

} // namespace glaze::producer::detail
