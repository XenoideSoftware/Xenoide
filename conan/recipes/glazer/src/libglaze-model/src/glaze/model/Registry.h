#pragma once

#include "glaze/model/ApiProfile.h"
#include "glaze/model/Command.h"
#include "glaze/model/ConsolidatedRequire.h"
#include "glaze/model/Enum.h"
#include "glaze/model/EnumGroup.h"
#include "glaze/model/Extension.h"
#include "glaze/model/Feature.h"
#include "glaze/model/Type.h"

#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace glaze::model {

/**
 * @brief Indexed in-memory OpenGL registry. Mirrors glaze/model.py::Registry.
 * Extended description: the main IR anchor of the Glaze port. Owns the raw
 * parsed vectors of types, enum groups, commands, features, and extensions,
 * and exposes a set of derived indices (by name, by api, by object class)
 * that are rebuilt once inside the constructor. The Registry is immutable
 * after construction: no public mutators are exposed and copy-construction
 * is disabled so the indices never dangle under accidental shallow copies.
 * Move-construction is allowed.
 *
 * Indices are stored as std::unordered_map<std::string, std::size_t> pointing
 * at entries inside the owned vectors — keeping indices (not raw pointers)
 * makes the class trivially movable and avoids pointer-stability pitfalls.
 */
class Registry {
public:
    /**
     * @brief Build a Registry from raw parsed data. Rebuilds all indices.
     * @param types parsed <type> entries
     * @param enumGroups parsed <enums> blocks
     * @param commands parsed <command> entries
     * @param features parsed <feature> entries
     * @param extensions parsed <extension> entries
     */
    Registry(std::vector<Type> types,
             std::vector<EnumGroup> enumGroups,
             std::vector<Command> commands,
             std::vector<Feature> features,
             std::vector<Extension> extensions);

    Registry(const Registry &) = delete;
    Registry &operator=(const Registry &) = delete;
    Registry(Registry &&) = default;
    Registry &operator=(Registry &&) = default;
    ~Registry() = default;

    //! Raw parsed <type> entries.
    const std::vector<Type> &types() const noexcept { return types_; }
    //! Raw parsed <enums> blocks.
    const std::vector<EnumGroup> &enumGroups() const noexcept { return enumGroups_; }
    //! Raw parsed <command> entries.
    const std::vector<Command> &commands() const noexcept { return commands_; }
    //! Raw parsed <feature> entries.
    const std::vector<Feature> &features() const noexcept { return features_; }
    //! Raw parsed <extension> entries.
    const std::vector<Extension> &extensions() const noexcept { return extensions_; }

    /**
     * @brief Look up a type by name.
     * @return pointer to the Type or nullptr if not found
     */
    const Type *findType(std::string_view name) const noexcept;

    /**
     * @brief Look up an enum constant by name across all enum groups.
     * @return pointer to the Enum or nullptr if not found
     */
    const Enum *findEnum(std::string_view name) const noexcept;

    /**
     * @brief Look up a command by name.
     * @return pointer to the Command or nullptr if not found
     */
    const Command *findCommand(std::string_view name) const noexcept;

    /**
     * @brief Return the list of enums that belong to a named group.
     * Mirrors glaze/model.py::Registry.group_to_enums.
     * @return pointer to a vector of enum pointers, or nullptr if no such group
     */
    const std::vector<const Enum *> *enumsForGroup(std::string_view group) const noexcept;

    /**
     * @brief True if a group was declared with type="bitmask".
     */
    bool isBitmaskGroup(std::string_view group) const noexcept;

    /**
     * @brief Return the features belonging to a specific API name.
     * Mirrors glaze/model.py::Registry.features_by_api.
     * @return pointer to the feature vector, or nullptr if no such api
     */
    const std::vector<Feature> *featuresForApi(std::string_view api) const noexcept;

    /**
     * @brief Return the extensions that list a given API in their supported attribute.
     * Resolves virtual APIs like "gl_compat" to their real backing api first.
     * Mirrors glaze/model.py::Registry.extensions_for_api.
     */
    std::vector<Extension> extensionsForApi(std::string_view api) const;

    /**
     * @brief Return the commands whose first parameter has the given class attribute.
     * Mirrors glaze/model.py::Registry.object_dict.
     * @return pointer to the command pointer vector, or nullptr if no such class
     */
    const std::vector<const Command *> *commandsForObjectClass(std::string_view cls) const noexcept;

    /**
     * @brief Resolve a possibly virtual api name to its ApiProfile.
     * Mirrors glaze/model.py::Registry.resolve_api.
     * @return the profile if the api is virtual (e.g. "gl_compat"), nullopt otherwise
     */
    std::optional<ApiProfile> resolveApi(std::string_view api) const;

    /**
     * @brief List all APIs and their sorted version strings.
     * Mirrors glaze/model.py::Registry.available_apis.
     * @return a map from api name to sorted list of version strings
     */
    std::map<std::string, std::vector<std::string>> availableApis() const;

    /**
     * @brief Flatten all features for api up to version into a ConsolidatedRequire.
     * Mirrors glaze/model.py::Registry.consolidate.
     * Extended description: iterates features in order, unions all <require>
     * entries whose feature.number <= version, then subtracts any <remove>
     * entries whose profile is not in ApiProfile::skipRemoveProfiles.
     * @param api target api name (may be virtual, e.g. "gl_compat")
     * @param version MAJOR.MINOR string (e.g. "3.3")
     */
    ConsolidatedRequire consolidate(std::string_view api, std::string_view version) const;

    /**
     * @brief Return all features for api whose number <= version, in registry order.
     * Mirrors glaze/model.py::Registry.collect_features.
     */
    std::vector<std::reference_wrapper<const Feature>>
    collectFeatures(std::string_view api, std::string_view version) const;

private:
    void buildIndices();
    std::string registryApi(std::string_view api) const;

    std::vector<Type> types_;
    std::vector<EnumGroup> enumGroups_;
    std::vector<Command> commands_;
    std::vector<Feature> features_;
    std::vector<Extension> extensions_;

    //! flat list of every Enum across all groups (stable addresses for index pointers)
    std::vector<Enum> flatEnums_;

    std::unordered_map<std::string, std::size_t> typeByName_;
    std::unordered_map<std::string, std::size_t> enumByName_;
    std::unordered_map<std::string, std::size_t> commandByName_;
    std::unordered_map<std::string, std::vector<const Enum *>> groupToEnums_;
    std::unordered_set<std::string> bitmaskGroups_;
    std::unordered_map<std::string, std::vector<Feature>> featuresByApi_;
    std::unordered_map<std::string, std::vector<Extension>> extensionsByApi_;
    //! std::map so iteration order is deterministic for generator output
    std::map<std::string, std::vector<const Command *>> objectDict_;
};

} // namespace glaze::model
