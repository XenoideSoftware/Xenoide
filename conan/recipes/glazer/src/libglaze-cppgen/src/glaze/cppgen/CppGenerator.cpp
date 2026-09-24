#include "glaze/cppgen/CppGenerator.h"

#include "glaze/codegen/ExtensionFilter.h"
#include "glaze/cppgen/detail/DsaEmitter.h"
#include "glaze/cppgen/detail/EnumClassEmitter.h"
#include "glaze/cppgen/detail/FunctorEmitter.h"
#include "glaze/cppgen/detail/GroupRename.h"
#include "glaze/cppgen/detail/HandleEmitter.h"
#include "glaze/cppgen/detail/ParamFormat.h"
#include "glaze/cppgen/detail/PatternDetector.h"
#include "glaze/cppgen/detail/RaiiCollector.h"
#include "glaze/model/StringUtils.h"

#include <inja/inja.hpp>

#include <algorithm>
#include <cctype>

namespace glaze::cppgen::detail::templates {

// Defined by build-time embedded sources generated via cmake/EmbedTextFile.cmake.
extern const char *gl_hpp_inja;
extern const char *gl_handle_hpp_inja;
extern const char *raii_hpp;

} // namespace glaze::cppgen::detail::templates

namespace glaze::cppgen {

namespace {

inja::Environment makeEnvironment() {
    inja::Environment env;
    env.set_trim_blocks(true);
    env.set_lstrip_blocks(true);
    return env;
}

//! Build cmd_version_map — earliest version for each command in the core snapshot.
std::map<std::string, std::string>
buildCommandVersionMap(const model::Registry &registry,
                       const std::string &api,
                       const std::string &version) {
    std::map<std::string, std::string> map;
    const auto features = registry.collectFeatures(api, version);
    for (const auto &featureRef : features) {
        const auto &feature = featureRef.get();
        for (const auto &require : feature.requireList) {
            for (const auto &cmdRef : require.commands) {
                map.try_emplace(cmdRef.name, feature.number);
            }
        }
    }
    return map;
}

} // namespace

CppGenerator::CppGenerator(const model::Registry &registry,
                           std::shared_ptr<const codegen::NameTransform> nameTransform,
                           std::shared_ptr<const codegen::TypeMapper> typeMapper)
    : registry_(registry),
      nameTransform_(nameTransform ? std::move(nameTransform)
                                   : std::make_shared<codegen::DefaultNameTransform>()),
      typeMapper_(typeMapper ? std::move(typeMapper)
                             : std::make_shared<codegen::DefaultCTypeMapper>()) {}

std::map<std::string, std::string>
CppGenerator::generate(const std::string &api,
                       const std::string &version,
                       const std::set<std::string> &extensionVendors,
                       const std::set<std::string> &extensionNames,
                       const docparser::DocIndex &docs) const {
    codegen::checkApiVersion(registry_, api, version);

    const auto consolidated = registry_.consolidate(api, version);
    const auto extEmissions = codegen::collectExtensionEmissions(
        registry_, api, version, extensionVendors, extensionNames);

    // Union of core + every extension-contributed command/enum name so the
    // downstream discovery and emission passes see one merged view.
    std::set<std::string> unionCommandNames = consolidated.commands;
    std::set<std::string> unionEnumNames = consolidated.enums;
    //! Map each extension-only command back to its originating extension so
    //! we can tag the functor with the guard macro / flag var spellings.
    struct ExtensionMeta {
        std::string shortName;
        std::string guardMacro;
        std::string flagVar;
    };
    std::map<std::string, ExtensionMeta> cmdToExtension;

    for (const auto &emission : extEmissions) {
        for (const auto *cmd : emission.commands) {
            unionCommandNames.insert(cmd->name);
            cmdToExtension[cmd->name] = {emission.shortName, emission.guardMacro,
                                          emission.flagVar};
        }
        for (const auto *e : emission.enums) {
            unionEnumNames.insert(e->name);
        }
    }

    // Discovery pass: walk every core + extension command and gather handle
    // classes and enum group names. Extensions are merged into the same
    // discovery set so extension-only commands contribute their types too.
    std::map<std::string, std::string> handleClasses;
    std::set<std::string> groupSet;
    for (const auto &cmdName : unionCommandNames) {
        const auto *cmd = registry_.findCommand(cmdName);
        if (cmd == nullptr) {
            continue;
        }
        for (const auto &param : cmd->params) {
            if (model::hasClass(param) && detail::isUintHandle(param)) {
                const auto &classStr = *param.classStr;
                if (handleClasses.count(classStr) == 0) {
                    handleClasses[classStr] = nameTransform_->transformHandleTypeName(classStr);
                }
            }
            if (model::hasGroup(param)) {
                if (registry_.enumsForGroup(*param.group) != nullptr) {
                    groupSet.insert(*param.group);
                }
            }
        }
    }

    // Vendor-suffix rename pass.
    const auto groupRename = detail::buildGroupRename(groupSet);

    // Collision resolution: if a handle type name clashes with a clean enum
    // class name, append "Id" to the handle type to disambiguate.
    std::set<std::string> cleanGroupNames;
    for (const auto &g : groupSet) {
        cleanGroupNames.insert(detail::renamedGroup(groupRename, g));
    }
    for (auto &[classStr, handleName] : handleClasses) {
        if (cleanGroupNames.count(handleName) != 0) {
            handleName += "Id";
        }
    }

    // Handle types block: one distinct CamelCase name per handle class, sorted.
    std::set<std::string> handleNameSet;
    for (const auto &[_, name] : handleClasses) {
        handleNameSet.insert(name);
    }
    nlohmann::json handleTypesArr = nlohmann::json::array();
    for (const auto &name : handleNameSet) {
        handleTypesArr.push_back(nlohmann::json{{"name", name}});
    }

    // Location-type usage scan.
    bool needUniformLocation = false;
    bool needAttribLocation = false;
    for (const auto &cmdName : unionCommandNames) {
        const auto *cmd = registry_.findCommand(cmdName);
        if (cmd == nullptr) {
            continue;
        }
        if (detail::isUniformLocationReturn(*cmd)) {
            needUniformLocation = true;
        }
        if (detail::isAttribLocationReturn(*cmd)) {
            needAttribLocation = true;
        }
        for (const auto &param : cmd->params) {
            if (detail::isUniformLocationParam(*cmd, param)) {
                needUniformLocation = true;
            }
            if (detail::isAttribLocationParam(*cmd, param)) {
                needAttribLocation = true;
            }
        }
    }
    nlohmann::json locationTypesArr = nlohmann::json::array();
    if (needAttribLocation) {
        locationTypesArr.push_back(nlohmann::json{{"name", "AttribLocation"}});
    }
    if (needUniformLocation) {
        locationTypesArr.push_back(nlohmann::json{{"name", "UniformLocation"}});
    }

    // Bitmask set (raw group name -> true).
    std::set<std::string> bitmaskGroups;
    for (const auto &g : groupSet) {
        if (registry_.isBitmaskGroup(g)) {
            bitmaskGroups.insert(g);
        }
    }

    // Enum classes block. Walk every referenced group, filter its entries to
    // the consolidated union, and emit only groups that still have at least
    // one entry. Record those raw group names in `emittedGroups` so the
    // downstream emitters know which group substitutions are safe to use
    // (ParamFormat falls back to the raw C type for any group that didn't
    // survive this filter, so functors never reference a type the header
    // hasn't declared).
    std::set<std::string> emittedGroups;
    nlohmann::json enumClassesArr = nlohmann::json::array();
    for (const auto &rawGroup : groupSet) {
        const auto *enumPtrs = registry_.enumsForGroup(rawGroup);
        if (enumPtrs == nullptr) {
            continue;
        }
        std::vector<const model::Enum *> filtered;
        filtered.reserve(enumPtrs->size());
        for (const auto *e : *enumPtrs) {
            if (unionEnumNames.count(e->name) != 0) {
                filtered.push_back(e);
            }
        }
        if (filtered.empty()) {
            continue;
        }
        emittedGroups.insert(rawGroup);
        const auto cleanName = detail::renamedGroup(groupRename, rawGroup);
        const bool isBitmask = bitmaskGroups.count(rawGroup) != 0;
        enumClassesArr.push_back(detail::buildEnumClassContext(
            rawGroup, cleanName, isBitmask, filtered, *nameTransform_));
    }

    // Shared emitter context — built after enum-class emission so it can
    // carry the `emittedGroups` set that ParamFormat needs.
    detail::EmitterContext emitterCtx{
        *nameTransform_,
        api,
        handleClasses,
        groupRename,
        bitmaskGroups,
        std::move(emittedGroups),
        buildCommandVersionMap(registry_, api, version),
        docs,
    };

    // Functor block. Build from the merged core+extension command set so
    // extension-only functors get their own struct + instance, then post-
    // process the result to tag every extension-origin functor with the
    // guard macro / flag var the template uses to wrap them.
    std::vector<const model::Command *> sortedCommands;
    for (const auto &cmdName : unionCommandNames) {
        if (const auto *cmd = registry_.findCommand(cmdName); cmd != nullptr) {
            sortedCommands.push_back(cmd);
        }
    }
    std::sort(sortedCommands.begin(), sortedCommands.end(),
              [](const model::Command *a, const model::Command *b) { return a->name < b->name; });
    auto functorsArr = detail::buildFunctors(sortedCommands, emitterCtx);

    // Tag extension-origin functors with their extension metadata so the
    // template can emit the correct `#ifndef GLAZE_GL_NO_EXT_<short>` block.
    // Also set `has_extension_guard` for inja's truthiness test — plain
    // `{% if fn.extension_short_name %}` on an empty string was unreliable
    // across inja versions, so we use an explicit bool.
    for (auto &fn : functorsArr) {
        const auto glName = fn.at("gl_name").get<std::string>();
        const auto it = cmdToExtension.find(glName);
        if (it == cmdToExtension.end()) {
            fn["has_extension_guard"] = false;
            fn["extension_short_name"] = "";
            fn["guard_macro"] = "";
            fn["flag_var"] = "";
        } else {
            fn["has_extension_guard"] = true;
            fn["extension_short_name"] = it->second.shortName;
            fn["guard_macro"] = it->second.guardMacro;
            fn["flag_var"] = it->second.flagVar;
        }
    }

    // DSA classes.
    const auto dsaClassesArr = detail::buildDsaClasses(registry_, consolidated, emitterCtx);

    // Handle classes (legacy, non-Named*).
    const auto handleClassesArr = detail::buildHandleClasses(registry_, consolidated, emitterCtx);

    // RAII resources.
    const auto raiiResourcesArr =
        detail::collectRaiiResources(registry_, consolidated, emitterCtx);

    // Extensions context: one entry per emission for the gl::exts::* namespace
    // block and the template-side guard/flag macros. Matches the shape
    // produced by glaze/generators/base.py::_collect_extension_emissions.
    nlohmann::json extensionsArr = nlohmann::json::array();
    for (const auto &emission : extEmissions) {
        extensionsArr.push_back(nlohmann::json{
            {"name", emission.name},
            {"short_name", emission.shortName},
            {"guard_macro", emission.guardMacro},
            {"flag_var", emission.flagVar},
        });
    }

    // Handle wrapper RAII list: pair each raii resource with its handle-class
    // wrapper type name so gl_handle.hpp.inja can emit Traits specializations.
    nlohmann::json handleRaiiArr = nlohmann::json::array();
    for (const auto &r : raiiResourcesArr) {
        for (const auto &hc : handleClassesArr) {
            if (hc["handle_type"] == r["handle_type"]) {
                nlohmann::json entry = r;
                entry["class_name"] = hc["class_name"];
                handleRaiiArr.push_back(entry);
                break;
            }
        }
    }

    // Assemble the template contexts.
    std::string apiUpper = api;
    for (auto &c : apiUpper) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    const std::string generationHeader =
        std::string{"Auto-generated by Glaze — DO NOT EDIT.\nAPI: "} + api + " " + version +
        " | Language: C++";
    std::string generationHeaderFormatted;
    generationHeaderFormatted.reserve(generationHeader.size() + 16);
    for (char ch : generationHeader) {
        if (ch == '\n') {
            generationHeaderFormatted.append("\n * ");
        } else {
            generationHeaderFormatted.push_back(ch);
        }
    }

    nlohmann::json hppCtx = {
        {"api", api},
        {"api_upper", apiUpper},
        {"generation_header", generationHeader},
        {"generation_header_formatted", generationHeaderFormatted},
        {"handle_types", handleTypesArr},
        {"location_types", locationTypesArr},
        {"enum_classes", enumClassesArr},
        {"functors", functorsArr},
        {"dsa_classes", dsaClassesArr},
        {"raii_resources", raiiResourcesArr},
        {"extensions", extensionsArr},
    };

    nlohmann::json handleCtx = {
        {"api", api},
        {"api_upper", apiUpper},
        {"generation_header", generationHeader},
        {"generation_header_formatted", generationHeaderFormatted},
        {"handle_classes", handleClassesArr},
        {"handle_raii_resources", handleRaiiArr},
        {"extensions", extensionsArr},
    };

    auto env = makeEnvironment();
    std::map<std::string, std::string> result;
    result["include/glaze/" + api + ".hpp"] =
        env.render(detail::templates::gl_hpp_inja, hppCtx);
    result["include/glaze/" + api + "_handle.hpp"] =
        env.render(detail::templates::gl_handle_hpp_inja, handleCtx);
    result["include/glaze/raii.hpp"] = detail::templates::raii_hpp;
    return result;
}

std::vector<std::string>
CppGenerator::validateExtensionFilters(const std::string &api,
                                       const std::set<std::string> &extensionVendors,
                                       const std::set<std::string> &extensionNames) const {
    return codegen::validateExtensionFilters(registry_, api, extensionVendors, extensionNames);
}

} // namespace glaze::cppgen
