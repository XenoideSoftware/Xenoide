#pragma once

#include <pugixml.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace glaze::producer::detail {

/**
 * @brief Collect all text content under a node recursively.
 * Mirrors glaze/parser.py::RegistryParser._collect_text — walks every
 * descendant and concatenates text nodes. Used to assemble the C source
 * text of a <type> block (e.g. "typedef unsigned int GLuint;").
 */
std::string collectText(const pugi::xml_node &node);

/**
 * @brief Split a mixed-content element's text/ptype stream into tokens.
 * Extended description: Python uses str.split() on raw text nodes and
 * appends <ptype> element contents as whole tokens. We replicate that
 * behavior here so both return types and parameter types parse the same
 * way. Empty segments are dropped.
 */
std::vector<std::string> tokenizeMixedContent(const pugi::xml_node &node);

/**
 * @brief Return the text content of the first <name> child of a node.
 * Used for command proto and param parsing.
 */
std::optional<std::string> firstChildName(const pugi::xml_node &node);

/**
 * @brief Return an optional string from an attribute — nullopt if missing
 *        or empty, otherwise the attribute value.
 */
std::optional<std::string> optionalAttr(const pugi::xml_node &node, const char *name);

/**
 * @brief Split a delimited attribute value into a list of strings, skipping
 *        any empty segments.
 */
std::vector<std::string> splitDelimited(std::string_view value, char delim);

} // namespace glaze::producer::detail
