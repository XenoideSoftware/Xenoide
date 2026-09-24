#include "glaze/codegen/NameTransform.h"

#include "glaze/model/StringUtils.h"

#include <array>
#include <cctype>
#include <string>

namespace glaze::codegen {

// Default-method bodies live on the base class so subclass implementations
// can call up to them explicitly when they only want to override one
// transform. These simply forward to the DefaultNameTransform statics — both
// paths share the same logic, preventing subtle drift.

std::string NameTransform::transformCommandName(std::string_view glName) const {
    return DefaultNameTransform{}.transformCommandName(glName);
}

std::string NameTransform::transformHandleTypeName(std::string_view classStr) const {
    return DefaultNameTransform{}.transformHandleTypeName(classStr);
}

std::string NameTransform::transformEnumEntry(std::string_view groupName,
                                              std::string_view constantName) const {
    return DefaultNameTransform{}.transformEnumEntry(groupName, constantName);
}

std::string NameTransform::transformEnumGroupName(std::string_view group,
                                                  const std::set<std::string> &allGroups) const {
    return DefaultNameTransform{}.transformEnumGroupName(group, allGroups);
}

// --- Default implementation --------------------------------------------------

std::string DefaultNameTransform::transformCommandName(std::string_view glName) const {
    // Strip "gl" prefix and lowercase the first remaining character.
    // Mirrors cpp_generator.py::_convert_function_name.
    if (glName.size() < 3 || glName[0] != 'g' || glName[1] != 'l') {
        return std::string{glName};
    }
    std::string out;
    out.reserve(glName.size() - 2);
    out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(glName[2]))));
    out.append(glName.substr(3));
    return out;
}

std::string DefaultNameTransform::transformHandleTypeName(std::string_view classStr) const {
    return model::camelCase(classStr);
}

namespace {

//! Recognized vendor suffixes from cpp_generator.py::_VENDOR_SUFFIXES. Used
//! by transformEnumGroupName to collapse group names like "BufferTargetARB"
//! down to "BufferTarget" when doing so would not collide.
constexpr std::array<std::string_view, 17> kVendorSuffixes{{
    "MESA", "INTEL", "ARM", "AMD", "ATI", "IBM", "SGIX", "SGIS", "SGI",
    "APPLE", "OES", "ARB", "EXT", "NV", "KHR", "OVR", "QCOM",
}};

bool endsWith(std::string_view value, std::string_view suffix) {
    if (suffix.size() > value.size()) {
        return false;
    }
    return value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

//! Split a GL_* constant into parts by underscore, dropping the leading GL_.
//! Python's string_utils.camel_case operates on a space-delimited phrase, so
//! we just join the remaining segments with single spaces before feeding it.
std::string splitEnumConstant(std::string_view constantName) {
    std::string_view rest = constantName;
    if (rest.size() > 3 && rest.substr(0, 3) == "GL_") {
        rest.remove_prefix(3);
    }

    std::string joined;
    joined.reserve(rest.size());
    bool first = true;
    std::string current;
    auto flush = [&]() {
        if (current.empty()) {
            return;
        }
        if (!first) {
            joined.push_back(' ');
        }
        joined.append(current);
        current.clear();
        first = false;
    };
    for (char ch : rest) {
        if (ch == '_') {
            flush();
        } else {
            current.push_back(ch);
        }
    }
    flush();
    return joined;
}

} // namespace

std::string DefaultNameTransform::transformEnumEntry(std::string_view /*groupName*/,
                                                     std::string_view constantName) const {
    // The default enum-entry transform produces "e<CamelCase>" by stripping
    // "GL_" and title-casing each underscore-separated word. More elaborate
    // "strip group prefix" behavior lives in cppgen's EnumClassEmitter, which
    // may subclass NameTransform when finer control is needed.
    const auto joined = splitEnumConstant(constantName);
    return "e" + model::camelCase(joined);
}

std::string DefaultNameTransform::transformEnumGroupName(
    std::string_view group,
    const std::set<std::string> &allGroups) const {
    const std::string groupStr{group};
    for (const auto &suffix : kVendorSuffixes) {
        if (!endsWith(groupStr, suffix) || groupStr.size() == suffix.size()) {
            continue;
        }
        std::string candidate = groupStr.substr(0, groupStr.size() - suffix.size());
        if (candidate.empty()) {
            continue;
        }
        // Preserve the leading upper-case letter on candidates; GL group
        // names are always PascalCase so this check is a sanity guard.
        if (std::isupper(static_cast<unsigned char>(candidate.front())) == 0) {
            continue;
        }
        if (allGroups.find(candidate) == allGroups.end() || candidate == groupStr) {
            return candidate;
        }
    }
    return groupStr;
}

} // namespace glaze::codegen
