#pragma once

#include "glaze/docparser/DocIndex.h"

#include <filesystem>
#include <string_view>

namespace glaze::docparser {

/**
 * @brief Builds a DocIndex from an OpenGL-Refpages tree.
 *
 * Port of `conan/packages/glaze/glaze/doc_parser.py`.  Each API has its own
 * refpages subdirectory (gl→gl4, gles1→es1.1, gles2→es3.0, glsc2→gl4);
 * DocParser walks the appropriate subdir, reads every `gl*.xml` refpage,
 * and collects the brief + parameter docs into a map keyed by GL function
 * name.  The underlying XML parser is kept strictly private so consumers
 * link against a clean interface with no pugixml leakage.
 */
class DocParser {
public:
    /**
     * @brief Parse every refpage under the API-specific subdirectory.
     *
     * @param refpagesDir Root of the OpenGL-Refpages checkout.
     * @param api        Glaze API name ("gl", "gles1", "gles2", "glsc2").
     *                   Unknown APIs fall back to the "gl4" subdirectory
     *                   just like the Python reference.
     * @return A DocIndex populated with every successfully-parsed entry.
     *         Returns an empty index when the subdirectory is missing —
     *         this is the opt-in "no docs" sentinel and never an error.
     */
    DocIndex parseRefpages(const std::filesystem::path &refpagesDir,
                           std::string_view api = "gl") const;
};

} // namespace glaze::docparser
