#include "glaze/model/Registry.h"

#include "glaze/model/StringUtils.h"

#include <algorithm>
#include <set>
#include <stdexcept>
#include <utility>

namespace glaze::model {

namespace {

//! Ordered list of virtual API profiles. Mirrors glaze/model.py::API_PROFILES.
//! Keeping it as a function-local static avoids the static-init-order dance.
const std::map<std::string, ApiProfile> &apiProfiles() {
    static const std::map<std::string, ApiProfile> table{
        {"gl_compat", ApiProfile{"gl", std::set<std::string>{"core"}}},
    };
    return table;
}

//! Lexicographic comparison of MAJOR.MINOR version strings that matches
//! Python's str-comparison semantics for single-digit components (which is
//! all GL ever uses). Parses the numeric parts and compares (major, minor).
//! A non-comparable input is treated as greater than everything else so
//! callers filtering "version <= target" still terminate cleanly.
int compareVersion(std::string_view lhs, std::string_view rhs) {
    try {
        const int a = versionToInt(lhs);
        const int b = versionToInt(rhs);
        if (a < b) {
            return -1;
        }
        if (a > b) {
            return 1;
        }
        return 0;
    } catch (const std::invalid_argument &) {
        if (lhs == rhs) {
            return 0;
        }
        return lhs < rhs ? -1 : 1;
    }
}

} // namespace

Registry::Registry(std::vector<Type> types,
                   std::vector<EnumGroup> enumGroups,
                   std::vector<Command> commands,
                   std::vector<Feature> features,
                   std::vector<Extension> extensions)
    : types_(std::move(types)),
      enumGroups_(std::move(enumGroups)),
      commands_(std::move(commands)),
      features_(std::move(features)),
      extensions_(std::move(extensions)) {
    buildIndices();
}

void Registry::buildIndices() {
    typeByName_.clear();
    for (std::size_t i = 0; i < types_.size(); ++i) {
        typeByName_[types_[i].name] = i;
    }

    // Flatten every enum across all groups into a single vector so we can
    // hand out stable pointers. Python's dict-of-dicts layout lets the
    // indices point at live dict entries; we replicate that by copying the
    // Enum values into flatEnums_ once.
    flatEnums_.clear();
    std::size_t totalEnums = 0;
    for (const auto &group : enumGroups_) {
        totalEnums += group.enums.size();
    }
    flatEnums_.reserve(totalEnums);
    enumByName_.clear();
    for (const auto &group : enumGroups_) {
        for (const auto &[name, enumValue] : group.enums) {
            enumByName_[name] = flatEnums_.size();
            flatEnums_.push_back(enumValue);
        }
    }

    commandByName_.clear();
    for (std::size_t i = 0; i < commands_.size(); ++i) {
        commandByName_[commands_[i].name] = i;
    }

    groupToEnums_.clear();
    for (const auto &flat : flatEnums_) {
        for (const auto &groupName : flat.groups) {
            if (!groupName.empty()) {
                groupToEnums_[groupName].push_back(&flat);
            }
        }
    }

    bitmaskGroups_.clear();
    for (const auto &group : enumGroups_) {
        if (group.group && group.enumGroupType && *group.enumGroupType == "bitmask") {
            bitmaskGroups_.insert(*group.group);
        }
    }

    featuresByApi_.clear();
    for (const auto &feature : features_) {
        featuresByApi_[feature.api].push_back(feature);
    }

    extensionsByApi_.clear();
    for (const auto &extension : extensions_) {
        for (const auto &apiName : extension.supported) {
            extensionsByApi_[apiName].push_back(extension);
        }
    }

    objectDict_.clear();
    for (const auto &command : commands_) {
        const auto cls = getClass(command);
        if (cls && !cls->empty()) {
            objectDict_[*cls].push_back(&command);
        }
    }
}

const Type *Registry::findType(std::string_view name) const noexcept {
    const auto it = typeByName_.find(std::string{name});
    if (it == typeByName_.end()) {
        return nullptr;
    }
    return &types_[it->second];
}

const Enum *Registry::findEnum(std::string_view name) const noexcept {
    const auto it = enumByName_.find(std::string{name});
    if (it == enumByName_.end()) {
        return nullptr;
    }
    return &flatEnums_[it->second];
}

const Command *Registry::findCommand(std::string_view name) const noexcept {
    const auto it = commandByName_.find(std::string{name});
    if (it == commandByName_.end()) {
        return nullptr;
    }
    return &commands_[it->second];
}

const std::vector<const Enum *> *Registry::enumsForGroup(std::string_view group) const noexcept {
    const auto it = groupToEnums_.find(std::string{group});
    if (it == groupToEnums_.end()) {
        return nullptr;
    }
    return &it->second;
}

bool Registry::isBitmaskGroup(std::string_view group) const noexcept {
    return bitmaskGroups_.find(std::string{group}) != bitmaskGroups_.end();
}

const std::vector<Feature> *Registry::featuresForApi(std::string_view api) const noexcept {
    const auto it = featuresByApi_.find(std::string{api});
    if (it == featuresByApi_.end()) {
        return nullptr;
    }
    return &it->second;
}

std::vector<Extension> Registry::extensionsForApi(std::string_view api) const {
    const auto resolved = registryApi(api);
    const auto it = extensionsByApi_.find(resolved);
    if (it == extensionsByApi_.end()) {
        return {};
    }
    return it->second;
}

const std::vector<const Command *> *
Registry::commandsForObjectClass(std::string_view cls) const noexcept {
    const auto it = objectDict_.find(std::string{cls});
    if (it == objectDict_.end()) {
        return nullptr;
    }
    return &it->second;
}

std::optional<ApiProfile> Registry::resolveApi(std::string_view api) const {
    const auto &table = apiProfiles();
    const auto it = table.find(std::string{api});
    if (it == table.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::string Registry::registryApi(std::string_view api) const {
    const auto profile = resolveApi(api);
    if (profile) {
        return profile->registryApi;
    }
    return std::string{api};
}

std::map<std::string, std::vector<std::string>> Registry::availableApis() const {
    std::map<std::string, std::vector<std::string>> result;
    for (const auto &[api, features] : featuresByApi_) {
        std::vector<std::string> versions;
        versions.reserve(features.size());
        for (const auto &feature : features) {
            versions.push_back(feature.number);
        }
        std::sort(versions.begin(), versions.end(),
                  [](const std::string &a, const std::string &b) {
                      return compareVersion(a, b) < 0;
                  });
        result.emplace(api, std::move(versions));
    }

    // Surface virtual APIs so callers can list them alongside real ones.
    for (const auto &[alias, profile] : apiProfiles()) {
        const auto real = result.find(profile.registryApi);
        if (real != result.end()) {
            result.emplace(alias, real->second);
        }
    }

    return result;
}

ConsolidatedRequire Registry::consolidate(std::string_view api,
                                          std::string_view version) const {
    const auto profile = resolveApi(api);
    const std::string realApi = profile ? profile->registryApi : std::string{api};
    const std::set<std::string> skipProfiles = profile ? profile->skipRemoveProfiles
                                                       : std::set<std::string>{};

    const auto it = featuresByApi_.find(realApi);
    ConsolidatedRequire result;
    if (it == featuresByApi_.end()) {
        return result;
    }

    const auto &features = it->second;

    for (const auto &feature : features) {
        if (compareVersion(feature.number, version) > 0) {
            continue;
        }
        for (const auto &require : feature.requireList) {
            for (const auto &enumRef : require.enums) {
                result.enums.insert(enumRef.name);
            }
            for (const auto &cmdRef : require.commands) {
                result.commands.insert(cmdRef.name);
            }
        }
    }

    for (const auto &feature : features) {
        if (compareVersion(feature.number, version) > 0) {
            continue;
        }
        for (const auto &remove : feature.removeList) {
            if (skipProfiles.find(remove.profile) != skipProfiles.end()) {
                continue;
            }
            for (const auto &enumRef : remove.enums) {
                result.enums.erase(enumRef.name);
            }
            for (const auto &cmdRef : remove.commands) {
                result.commands.erase(cmdRef.name);
            }
        }
    }

    return result;
}

std::vector<std::reference_wrapper<const Feature>>
Registry::collectFeatures(std::string_view api, std::string_view version) const {
    std::vector<std::reference_wrapper<const Feature>> result;
    const auto realApi = registryApi(api);
    const auto it = featuresByApi_.find(realApi);
    if (it == featuresByApi_.end()) {
        return result;
    }
    for (const auto &feature : it->second) {
        if (compareVersion(feature.number, version) <= 0) {
            result.emplace_back(feature);
        }
    }
    return result;
}

} // namespace glaze::model
