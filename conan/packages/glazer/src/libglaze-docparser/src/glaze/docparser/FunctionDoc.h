#pragma once

#include <map>
#include <string>

namespace glaze::docparser {

/**
 * @brief Documentation extracted from one OpenGL-Refpages refpage.
 *
 * Mirrors the Python dataclass in `conan/packages/glaze/glaze/doc_parser.py`:
 * a single brief (from `<refpurpose>`) plus a parameter name → description
 * map (from the `<refsect1 xml:id="parameters">` varlistentries).  Shared
 * verbatim across every `<refname>` declared in the refpage.
 */
struct FunctionDoc {
    //! One-line description taken from the refpage's `<refpurpose>` node.
    std::string brief;
    //! Parameter name -> first-paragraph description.
    std::map<std::string, std::string> params;
};

} // namespace glaze::docparser
