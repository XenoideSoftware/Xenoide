#pragma once

#include "glaze/docparser/FunctionDoc.h"

#include <map>
#include <string>

namespace glaze::docparser {

/**
 * @brief Lookup table mapping GL function name → FunctionDoc.
 *
 * Built by `DocParser::parseRefpages`.  An empty index is a valid sentinel
 * meaning "no documentation available" — consumers should treat a missing
 * entry as "skip doc emission for this command" rather than an error.
 */
using DocIndex = std::map<std::string, FunctionDoc>;

} // namespace glaze::docparser
