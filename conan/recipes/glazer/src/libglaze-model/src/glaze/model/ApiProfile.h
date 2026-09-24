#pragma once

#include <set>
#include <string>

namespace glaze::model {

/**
 * @brief Virtual API profile mapping. Mirrors glaze/model.py::ApiProfile.
 * Extended description: encodes virtual API names (like "gl_compat") that
 * don't exist in gl.xml but resolve to a real registry API with profile
 * behavior. The skipRemoveProfiles set names profiles whose <remove> entries
 * are ignored when consolidating features — this is how gl_compat preserves
 * symbols that the core profile strips.
 */
struct ApiProfile {
    //! real registry api name (e.g. "gl")
    std::string registryApi;
    //! profile names whose <remove> entries are ignored during consolidation
    std::set<std::string> skipRemoveProfiles;
};

} // namespace glaze::model
