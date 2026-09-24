#include "glaze/codegen/ExtensionFilter.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

namespace glaze::codegen {

std::optional<std::string> vendorOf(std::string_view extensionName) {
    constexpr std::string_view prefix = "GL_";
    if (extensionName.size() <= prefix.size() || extensionName.substr(0, prefix.size()) != prefix) {
        return std::nullopt;
    }
    const auto rest = extensionName.substr(prefix.size());
    const auto us = rest.find('_');
    if (us == std::string_view::npos) {
        return std::nullopt;
    }
    std::string vendor{rest.substr(0, us)};
    for (auto &c : vendor) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return vendor;
}

std::vector<model::Extension> filterExtensions(const std::vector<model::Extension> &extensions,
                                               const std::set<std::string> &extensionVendors,
                                               const std::set<std::string> &extensionNames) {
    if (extensionVendors.empty() && extensionNames.empty()) {
        return {};
    }

    // Normalize vendor set to upper-case to match vendorOf output.
    std::set<std::string> vendors;
    for (const auto &v : extensionVendors) {
        std::string upper = v;
        for (auto &c : upper) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
        vendors.insert(std::move(upper));
    }

    std::vector<model::Extension> result;
    for (const auto &ext : extensions) {
        if (extensionNames.count(ext.name) != 0) {
            result.push_back(ext);
            continue;
        }
        const auto vendor = vendorOf(ext.name);
        if (vendor && vendors.count(*vendor) != 0) {
            result.push_back(ext);
        }
    }
    return result;
}

std::vector<ExtensionEmission>
collectExtensionEmissions(const model::Registry &registry,
                          std::string_view api,
                          std::string_view version,
                          const std::set<std::string> &extensionVendors,
                          const std::set<std::string> &extensionNames) {
    const auto consolidated = registry.consolidate(api, version);
    const auto filtered =
        filterExtensions(registry.extensionsForApi(api), extensionVendors, extensionNames);

    std::vector<ExtensionEmission> emissions;
    emissions.reserve(filtered.size());

    for (const auto &ext : filtered) {
        ExtensionEmission emission;
        emission.name = ext.name;
        emission.shortName = (ext.name.rfind("GL_", 0) == 0) ? ext.name.substr(3) : ext.name;
        emission.guardMacro = "GLAZE_GL_NO_EXT_" + emission.shortName;
        emission.flagVar = "GLAZE_EXT_" + ext.name;

        std::unordered_set<std::string> seenEnums;
        std::unordered_set<std::string> seenCommands;
        for (const auto &require : ext.requireList) {
            for (const auto &enumRef : require.enums) {
                if (consolidated.enums.count(enumRef.name) != 0) {
                    continue;
                }
                if (!seenEnums.insert(enumRef.name).second) {
                    continue;
                }
                if (const auto *e = registry.findEnum(enumRef.name); e != nullptr) {
                    emission.enums.push_back(e);
                }
            }
            for (const auto &cmdRef : require.commands) {
                if (consolidated.commands.count(cmdRef.name) != 0) {
                    continue;
                }
                if (!seenCommands.insert(cmdRef.name).second) {
                    continue;
                }
                if (const auto *c = registry.findCommand(cmdRef.name); c != nullptr) {
                    emission.commands.push_back(c);
                }
            }
        }
        emissions.push_back(std::move(emission));
    }
    return emissions;
}

std::vector<std::string>
validateExtensionFilters(const model::Registry &registry,
                         std::string_view api,
                         const std::set<std::string> &extensionVendors,
                         const std::set<std::string> &extensionNames) {
    const auto extensions = registry.extensionsForApi(api);
    std::set<std::string> knownVendors;
    std::set<std::string> knownNames;
    for (const auto &ext : extensions) {
        knownNames.insert(ext.name);
        if (const auto vendor = vendorOf(ext.name); vendor) {
            knownVendors.insert(*vendor);
        }
    }

    std::vector<std::string> errors;

    std::set<std::string> upperVendors;
    for (const auto &v : extensionVendors) {
        std::string upper = v;
        for (auto &c : upper) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
        upperVendors.insert(std::move(upper));
    }

    for (const auto &v : upperVendors) {
        if (knownVendors.count(v) == 0) {
            std::ostringstream os;
            os << "unknown extension vendor '" << v << "' for api '" << api
               << "'. Known vendors: ";
            bool first = true;
            for (const auto &known : knownVendors) {
                if (!first) {
                    os << ", ";
                }
                os << known;
                first = false;
            }
            errors.push_back(os.str());
        }
    }
    for (const auto &n : extensionNames) {
        if (knownNames.count(n) == 0) {
            std::ostringstream os;
            os << "unknown extension '" << n << "' for api '" << api << "'.";
            errors.push_back(os.str());
        }
    }
    return errors;
}

void checkApiVersion(const model::Registry &registry,
                     std::string_view api,
                     std::string_view version) {
    const auto available = registry.availableApis();
    const auto apiIt = available.find(std::string{api});
    if (apiIt == available.end()) {
        std::ostringstream os;
        os << "API '" << api << "' not found. Available: ";
        bool first = true;
        for (const auto &[name, _] : available) {
            if (!first) {
                os << ", ";
            }
            os << name;
            first = false;
        }
        throw std::invalid_argument(os.str());
    }
    const auto &versions = apiIt->second;
    const auto versionStr = std::string{version};
    if (std::find(versions.begin(), versions.end(), versionStr) == versions.end()) {
        std::ostringstream os;
        os << "Version '" << version << "' not found for '" << api << "'. Available: ";
        bool first = true;
        for (const auto &v : versions) {
            if (!first) {
                os << ", ";
            }
            os << v;
            first = false;
        }
        throw std::invalid_argument(os.str());
    }
}

} // namespace glaze::codegen
