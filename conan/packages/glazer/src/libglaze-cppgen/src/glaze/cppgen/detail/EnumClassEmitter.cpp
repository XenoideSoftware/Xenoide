#include "glaze/cppgen/detail/EnumClassEmitter.h"

namespace glaze::cppgen::detail {

nlohmann::json buildEnumClassContext(const std::string &groupName,
                                     const std::string &cleanGroupName,
                                     bool isBitmask,
                                     const std::vector<const model::Enum *> &enums,
                                     const codegen::NameTransform &nameTransform) {
    nlohmann::json entries = nlohmann::json::array();
    // Deduplicate entries by C++ identifier — GL enum aliases may share a
    // value and a group, and emitting duplicates would produce a compile error
    // in the generated enum class.
    std::set<std::string> seenIdentifiers;
    for (const auto *e : enums) {
        auto identifier = nameTransform.transformEnumEntry(cleanGroupName, e->name);
        if (!seenIdentifiers.insert(identifier).second) {
            continue;
        }
        nlohmann::json entry{
            {"name", std::move(identifier)},
            {"value", e->name}, // emit the GL_* macro from the C header
        };
        if (e->comment) {
            entry["comment"] = *e->comment;
        }
        entries.push_back(std::move(entry));
    }

    return nlohmann::json{
        {"group_name", cleanGroupName},
        {"raw_group_name", groupName},
        {"base_type", isBitmask ? "GLbitfield" : "GLenum"},
        {"is_bitmask", isBitmask},
        {"entries", entries},
    };
}

} // namespace glaze::cppgen::detail
