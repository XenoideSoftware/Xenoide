#pragma once

#include "glaze/model/Command.h"
#include "glaze/model/Enum.h"
#include "glaze/model/Extension.h"
#include "glaze/model/Registry.h"

#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace glaze::codegen {

/**
 * @brief One extension selected for emission by a generator.
 * Extended description: carries the Khronos extension name plus language-
 * agnostic derivatives (short_name, guard macro, flag variable) and pointers
 * to the enums and commands it uniquely contributes after deduplication
 * against the core feature snapshot. Mirrors the dict returned by
 * glaze/generators/base.py::Generator._collect_extension_emissions.
 */
struct ExtensionEmission {
    //! Full Khronos name, e.g. "GL_ARB_buffer_storage"
    std::string name;
    //! Name with the leading "GL_" stripped, e.g. "ARB_buffer_storage"
    std::string shortName;
    //! Guard macro for opting out of this extension at compile time
    std::string guardMacro;
    //! int flag variable set at runtime by glazeLoadExtensions
    std::string flagVar;
    //! Enums this extension contributes that are not already in core
    std::vector<const model::Enum *> enums;
    //! Commands this extension contributes that are not already in core
    std::vector<const model::Command *> commands;
};

/**
 * @brief Extract the vendor token from a Khronos extension name.
 * Mirrors glaze/generators/base.py::_vendor_of.
 * @return e.g. "ARB" for "GL_ARB_buffer_storage"; nullopt for bad input.
 */
std::optional<std::string> vendorOf(std::string_view extensionName);

/**
 * @brief Filter extensions by vendor and full-name sets (additive, opt-in).
 * Mirrors glaze/generators/base.py::Generator._filter_extensions.
 * Extended description: when both sets are empty returns an empty list —
 * extensions are opt-in by default. Otherwise returns extensions whose full
 * name is in extensionNames OR whose vendor is in extensionVendors.
 */
std::vector<model::Extension> filterExtensions(const std::vector<model::Extension> &extensions,
                                               const std::set<std::string> &extensionVendors,
                                               const std::set<std::string> &extensionNames);

/**
 * @brief Build the list of extension emissions for a given api/version.
 * Mirrors glaze/generators/base.py::Generator._collect_extension_emissions.
 * Extended description: walks filtered extensions in registry order and,
 * for each, gathers the enums and commands that are NOT already required
 * by the core feature snapshot at the given version. Extensions whose
 * contributions are entirely covered by core are still returned so callers
 * can still emit runtime flags and macros for them.
 */
std::vector<ExtensionEmission>
collectExtensionEmissions(const model::Registry &registry,
                          std::string_view api,
                          std::string_view version,
                          const std::set<std::string> &extensionVendors,
                          const std::set<std::string> &extensionNames);

/**
 * @brief Validate that the requested vendor and name filters resolve against
 *        a given api's real extension list.
 * Mirrors glaze/generators/base.py::Generator.validate_extension_filters.
 * @return a list of human-readable error strings; empty when everything resolves
 */
std::vector<std::string>
validateExtensionFilters(const model::Registry &registry,
                         std::string_view api,
                         const std::set<std::string> &extensionVendors,
                         const std::set<std::string> &extensionNames);

/**
 * @brief Validate that a given api/version pair exists in the registry.
 * Mirrors glaze/generators/base.py::Generator._check_api_version.
 * @throws std::invalid_argument when the api or version is not known
 */
void checkApiVersion(const model::Registry &registry,
                     std::string_view api,
                     std::string_view version);

} // namespace glaze::codegen
