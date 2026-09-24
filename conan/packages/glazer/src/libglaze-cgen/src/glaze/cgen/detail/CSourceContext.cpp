#include "glaze/cgen/detail/CSourceContext.h"

#include "glaze/cgen/detail/CCommandFormat.h"
#include "glaze/model/StringUtils.h"
#include "glaze/model/TypeDecl.h"

#include <string>
#include <unordered_set>

namespace glaze::cgen::detail {

namespace {

nlohmann::json buildDebugWrapperContext(const model::Command &command,
                                        const codegen::TypeMapper &typeMapper) {
    const auto returnTypeStr = model::toCString(command.returnType);
    const bool isVoid = returnTypeStr == "void";

    std::string paramsStr;
    std::string argsStr;
    bool first = true;
    for (const auto &param : command.params) {
        if (!first) {
            paramsStr.append(", ");
            argsStr.append(", ");
        }
        paramsStr.append(typeMapper.mapParamDecl(param, command));
        argsStr.append(param.name);
        first = false;
    }
    if (paramsStr.empty()) {
        paramsStr = "void";
    }

    return nlohmann::json{
        {"debug_name", "glaze_debug_" + command.name},
        {"raw_name", "glaze_" + command.name},
        {"return_type", returnTypeStr},
        {"is_void", isVoid},
        {"params_str", paramsStr},
        {"args_str", argsStr},
        {"param_count", static_cast<int>(command.params.size())},
        {"name", command.name},
    };
}

} // namespace

nlohmann::json buildSourceContext(
    const model::Registry &registry,
    const std::vector<std::reference_wrapper<const model::Feature>> &features,
    const std::vector<codegen::ExtensionEmission> &extensionEmissions,
    const std::string &api,
    const std::string &version,
    const codegen::TypeMapper &typeMapper) {
    nlohmann::json featuresArr = nlohmann::json::array();
    nlohmann::json loaderFeaturesArr = nlohmann::json::array();
    nlohmann::json debugWrapperFeaturesArr = nlohmann::json::array();

    // Track command names whose debug wrapper body has already been emitted.
    // Commands promoted across multiple GL feature versions are requested
    // again in each feature's requireList; since the per-feature
    // "#if GLAZE_GL_VERSION >= N" guards all expand when the compile-time
    // floor is higher, emitting the wrapper body more than once yields a
    // C redefinition error. Pointer declarations remain duplicated — they
    // are legal C tentative definitions.
    std::unordered_set<std::string> emittedWrapperNames;

    for (const auto &featureRef : features) {
        const auto &feature = featureRef.get();
        const int verInt = model::versionToInt(feature.number);

        nlohmann::json definitionsArr = nlohmann::json::array();
        nlohmann::json entriesArr = nlohmann::json::array();
        nlohmann::json wrappersArr = nlohmann::json::array();

        for (const auto &require : feature.requireList) {
            for (const auto &cmdRef : require.commands) {
                const auto *cmd = registry.findCommand(cmdRef.name);
                if (cmd == nullptr) {
                    continue;
                }
                const auto rawName = "glaze_" + cmd->name;
                const auto ptrType = commandPtrTypeName(cmd->name);
                definitionsArr.push_back("GLAZE_API " + ptrType + " " + rawName + ";");
                entriesArr.push_back(nlohmann::json{
                    {"ptr_var", rawName},
                    {"ptr_type", ptrType},
                    {"gl_name", cmd->name},
                });
                if (emittedWrapperNames.insert(cmd->name).second) {
                    wrappersArr.push_back(buildDebugWrapperContext(*cmd, typeMapper));
                }
            }
        }

        featuresArr.push_back(nlohmann::json{
            {"name", feature.name},
            {"version_int", verInt},
            {"definitions", definitionsArr},
        });
        loaderFeaturesArr.push_back(nlohmann::json{
            {"name", feature.name},
            {"version_int", verInt},
            {"entries", entriesArr},
        });
        debugWrapperFeaturesArr.push_back(nlohmann::json{
            {"name", feature.name},
            {"version_int", verInt},
            {"wrappers", wrappersArr},
        });
    }

    nlohmann::json extensionsArr = nlohmann::json::array();
    for (const auto &emission : extensionEmissions) {
        nlohmann::json definitionsArr = nlohmann::json::array();
        nlohmann::json entriesArr = nlohmann::json::array();
        nlohmann::json wrappersArr = nlohmann::json::array();
        for (const auto *cmd : emission.commands) {
            const auto rawName = "glaze_" + cmd->name;
            const auto ptrType = commandPtrTypeName(cmd->name);
            definitionsArr.push_back("GLAZE_API " + ptrType + " " + rawName + ";");
            entriesArr.push_back(nlohmann::json{
                {"ptr_var", rawName},
                {"ptr_type", ptrType},
                {"gl_name", cmd->name},
            });
            if (emittedWrapperNames.insert(cmd->name).second) {
                wrappersArr.push_back(buildDebugWrapperContext(*cmd, typeMapper));
            }
        }
        extensionsArr.push_back(nlohmann::json{
            {"name", emission.name},
            {"short_name", emission.shortName},
            {"guard_macro", emission.guardMacro},
            {"flag_var", emission.flagVar},
            {"definitions", definitionsArr},
            {"entries", entriesArr},
            {"wrappers", wrappersArr},
        });
    }

    const std::string generationHeader =
        std::string{"Auto-generated by Glaze — DO NOT EDIT.\nAPI: "} + api + " " + version +
        " | Language: C";

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
        {"features", featuresArr},
        {"loader_features", loaderFeaturesArr},
        {"debug_wrapper_features", debugWrapperFeaturesArr},
        {"extensions", extensionsArr},
        {"api", api},
        {"version_floor_int", model::versionToInt(version)},
        {"generation_header", generationHeader},
        {"generation_header_formatted", generationHeaderFormatted},
    };
}

} // namespace glaze::cgen::detail
