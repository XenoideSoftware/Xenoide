#pragma once

#include <optional>
#include <string>
#include <vector>

namespace glaze::model {

/**
 * @brief A single enumerator from gl.xml. Mirrors glaze/model.py::Enum.
 * Extended description: represents one <enum> child inside an <enums> block.
 * The value field is kept as the raw string (hex or decimal) exactly as it
 * appears in the registry so the C generator can emit it verbatim.
 */
struct Enum {
    //! GL_* constant name (e.g. "GL_COLOR_BUFFER_BIT")
    std::string name;
    //! raw value string as it appears in gl.xml (e.g. "0x00004000", "0x1")
    std::string value;
    //! zero or more enum-group names this constant belongs to
    std::vector<std::string> groups;
    //! optional alias pointing at another enum by name
    std::optional<std::string> alias;
    //! optional comment attribute
    std::optional<std::string> comment;
};

} // namespace glaze::model
