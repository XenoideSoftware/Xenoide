#include "glaze/cppgen/detail/GroupRename.h"

#include <array>
#include <string_view>

namespace glaze::cppgen::detail {

namespace {

//! Matches cpp_generator.py::_VENDOR_SUFFIXES. Ordered longest-first so longer
//! vendor names match before shorter ones (e.g. "INTEL" before "INT").
constexpr std::array<std::string_view, 26> kVendorSuffixes{{
    "MESA", "INTEL", "APPLE", "SGIS", "SGIX", "SUNX", "QCOM", "3DFX", "INGR",
    "REND", "WIN", "ARB", "EXT", "NVX", "KHR", "OES", "AMD", "ATI", "IBM",
    "SUN", "NV", "HP", "IMG", "VIV", "DMP", "FJ",
}};

bool endsWith(std::string_view value, std::string_view suffix) {
    if (suffix.size() > value.size()) {
        return false;
    }
    return value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

} // namespace

std::map<std::string, std::string> buildGroupRename(const std::set<std::string> &groupSet) {
    std::map<std::string, std::string> rename;
    for (const auto &name : groupSet) {
        for (const auto suffix : kVendorSuffixes) {
            if (!endsWith(name, suffix) || name.size() == suffix.size()) {
                continue;
            }
            const auto clean = name.substr(0, name.size() - suffix.size());
            if (clean.empty()) {
                continue;
            }
            if (groupSet.count(clean) == 0) {
                rename.emplace(name, clean);
            }
            break; // Python stops at the first matching suffix regardless.
        }
    }
    return rename;
}

std::string renamedGroup(const std::map<std::string, std::string> &renameMap,
                         const std::string &groupName) {
    const auto it = renameMap.find(groupName);
    return it == renameMap.end() ? groupName : it->second;
}

} // namespace glaze::cppgen::detail
