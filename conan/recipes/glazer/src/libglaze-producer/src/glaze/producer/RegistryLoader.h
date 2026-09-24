#pragma once

#include "glaze/model/Registry.h"

#include <string>
#include <string_view>

namespace glaze::producer {

/**
 * @brief Parses the Khronos OpenGL XML registry into an in-memory Registry.
 * Extended description: this is the only class in the port that knows about
 * the underlying XML backend (pugixml). Callers receive a plain-value
 * glaze::model::Registry and never see any DOM types, which lets the
 * backend be swapped without breaking any consumers.
 */
class RegistryLoader {
public:
    /**
     * @brief Load and parse a gl.xml file from disk.
     * @param path filesystem path to the registry file
     * @return fully populated Registry with all indices built
     * @throws glaze::producer::ParseError on I/O or structural failure
     */
    glaze::model::Registry loadFromFile(std::string_view path) const;

    /**
     * @brief Parse an already-loaded XML string.
     * @param xml UTF-8 encoded XML text
     * @return fully populated Registry
     * @throws glaze::producer::ParseError on structural failure
     */
    glaze::model::Registry loadFromString(std::string_view xml) const;
};

} // namespace glaze::producer
