#include "glaze/cgen/detail/CHeaderContext.h"

#include "glaze/cgen/detail/CCommandFormat.h"
#include "glaze/model/StringUtils.h"

#include <set>
#include <string>
#include <unordered_set>

namespace glaze::cgen::detail {

namespace {

//! Build the per-command dict consumed by gl.h.inja. Mirrors
//! glaze/generators/c_generator.py::_build_command_header_entry.
nlohmann::json buildCommandEntry(const model::Command &command,
                                 const std::string &verTag,
                                 const codegen::TypeMapper &typeMapper,
                                 const docparser::DocIndex &docs) {
    nlohmann::json entry;
    entry["typedef"] = generateCommandPtrTypedef(command, typeMapper);
    entry["extern_raw"] = generateCommandPtrExternRaw(command);
    entry["debug_decl"] = generateDebugWrapperDecl(command, typeMapper);
    entry["debug_name"] = "glaze_debug_" + command.name;
    entry["release_alias"] = "#define " + command.name + " glaze_" + command.name;
    entry["debug_alias"] = "#define " + command.name + " glaze_debug_" + command.name;
    entry["name"] = command.name;
    // Prefer the refpage brief when available; fall back to the version tag
    // so downstream templates always render a minimal /** @brief */ comment.
    std::string brief = verTag;
    if (const auto it = docs.find(command.name); it != docs.end() && !it->second.brief.empty()) {
        brief = it->second.brief;
    }
    entry["doc_brief"] = std::move(brief);

    nlohmann::json docParams = nlohmann::json::array();
    if (const auto it = docs.find(command.name); it != docs.end()) {
        for (const auto &[paramName, desc] : it->second.params) {
            docParams.push_back(nlohmann::json{{"name", paramName}, {"desc", desc}});
        }
    }
    entry["doc_params"] = std::move(docParams);

    return entry;
}

//! Collect every type name referenced by a command's params + return type.
void collectCommandTypes(const model::Command &command, std::set<std::string> &typeNames) {
    if (command.returnType.name != "void") {
        typeNames.insert(command.returnType.name);
    }
    for (const auto &param : command.params) {
        const auto base = model::baseType(param);
        if (base) {
            typeNames.insert(*base);
        }
    }
}

} // namespace

nlohmann::json buildHeaderContext(
    const model::Registry &registry,
    const std::vector<std::reference_wrapper<const model::Feature>> &features,
    const std::vector<codegen::ExtensionEmission> &extensionEmissions,
    const std::string &api,
    const std::string &version,
    const codegen::TypeMapper &typeMapper,
    const docparser::DocIndex &docs) {
    // 1. Collect the union of type names referenced by core and extension commands
    std::set<std::string> allTypeNames;
    for (const auto &featureRef : features) {
        const auto &feature = featureRef.get();
        for (const auto &require : feature.requireList) {
            for (const auto &cmdRef : require.commands) {
                if (const auto *cmd = registry.findCommand(cmdRef.name); cmd != nullptr) {
                    collectCommandTypes(*cmd, allTypeNames);
                }
            }
            for (const auto &typeRef : require.types) {
                allTypeNames.insert(typeRef.name);
            }
        }
    }
    for (const auto &emission : extensionEmissions) {
        for (const auto *cmd : emission.commands) {
            collectCommandTypes(*cmd, allTypeNames);
        }
    }

    // 2. Emit types in registry declaration order, filtered to the union set.
    nlohmann::json typesArray = nlohmann::json::array();
    for (const auto &type : registry.types()) {
        if (allTypeNames.count(type.name) != 0) {
            typesArray.push_back(type.cDefinition);
        }
    }

    // 3. Build per-feature contexts.
    nlohmann::json featuresArray = nlohmann::json::array();
    std::string apiUpper = api;
    for (auto &c : apiUpper) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    for (const auto &featureRef : features) {
        const auto &feature = featureRef.get();
        const int verInt = model::versionToInt(feature.number);

        nlohmann::json enumsArr = nlohmann::json::array();
        nlohmann::json commandsArr = nlohmann::json::array();
        for (const auto &require : feature.requireList) {
            for (const auto &enumRef : require.enums) {
                if (const auto *e = registry.findEnum(enumRef.name); e != nullptr) {
                    nlohmann::json enumEntry{{"name", e->name}, {"value", e->value}};
                    if (e->comment) {
                        enumEntry["comment"] = *e->comment;
                    }
                    enumsArr.push_back(std::move(enumEntry));
                }
            }
            for (const auto &cmdRef : require.commands) {
                const auto *cmd = registry.findCommand(cmdRef.name);
                if (cmd == nullptr) {
                    continue;
                }
                const std::string verTag = "[" + apiUpper + " " + feature.number + "]";
                commandsArr.push_back(buildCommandEntry(*cmd, verTag, typeMapper, docs));
            }
        }

        featuresArray.push_back(nlohmann::json{
            {"name", feature.name},
            {"version_int", verInt},
            {"version_major", verInt / 10},
            {"version_minor", verInt % 10},
            {"enums", enumsArr},
            {"commands", commandsArr},
        });
    }

    // 4. Build extension context list.
    nlohmann::json extensionsArray = nlohmann::json::array();
    for (const auto &emission : extensionEmissions) {
        nlohmann::json enumsArr = nlohmann::json::array();
        for (const auto *e : emission.enums) {
            nlohmann::json enumEntry{{"name", e->name}, {"value", e->value}};
            if (e->comment) {
                enumEntry["comment"] = *e->comment;
            }
            enumsArr.push_back(std::move(enumEntry));
        }
        nlohmann::json commandsArr = nlohmann::json::array();
        for (const auto *cmd : emission.commands) {
            const std::string verTag = "[" + emission.name + "]";
            commandsArr.push_back(buildCommandEntry(*cmd, verTag, typeMapper, docs));
        }
        extensionsArray.push_back(nlohmann::json{
            {"name", emission.name},
            {"short_name", emission.shortName},
            {"guard_macro", emission.guardMacro},
            {"flag_var", emission.flagVar},
            {"khronos_define", emission.name},
            {"enums", enumsArr},
            {"commands", commandsArr},
        });
    }

    const std::string generationHeader =
        std::string{"Auto-generated by Glaze — DO NOT EDIT.\nAPI: "} + api + " " + version +
        " | Language: C";

    // Inja has no pipe-filter syntax so we pre-format the header line breaks
    // here instead of letting the template call `replace('\n', '\n * ')`.
    std::string generationHeaderFormatted;
    generationHeaderFormatted.reserve(generationHeader.size() + 16);
    for (char ch : generationHeader) {
        if (ch == '\n') {
            generationHeaderFormatted.append("\n * ");
        } else {
            generationHeaderFormatted.push_back(ch);
        }
    }

    return nlohmann::json{
        {"types", typesArray},
        {"features", featuresArray},
        {"extensions", extensionsArray},
        {"api", api},
        {"api_upper", apiUpper},
        {"version_floor_int", model::versionToInt(version)},
        {"generation_header", generationHeader},
        {"generation_header_formatted", generationHeaderFormatted},
    };
}

} // namespace glaze::cgen::detail
