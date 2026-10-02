#include "glaze/cppgen/detail/DsaEmitter.h"

#include "glaze/cppgen/detail/CppKeywords.h"
#include "glaze/cppgen/detail/ParamFormat.h"
#include "glaze/cppgen/detail/PatternDetector.h"
#include "glaze/model/StringUtils.h"
#include "glaze/model/TypeDecl.h"

#include <cctype>
#include <string>

namespace glaze::cppgen::detail {

namespace {

//! Turn a "Named*" command name into a lowerCamel method name, stripping both
//! the "gl" prefix and the class portion. Mirrors
//! cpp_generator.py::_dsa_method_name.
//! Example: glNamedBufferData with classStr="buffer" -> "data".
std::string dsaMethodName(const std::string &glName, const std::string &classStr) {
    std::string name = glName;
    if (name.rfind("gl", 0) == 0) {
        name = name.substr(2);
    }
    const std::string named = "Named";
    if (name.rfind(named, 0) == 0) {
        name = name.substr(named.size());
    }
    // Strip capitalized class portion, e.g. "BufferData" -> "Data" for class "buffer"
    std::string capClass = classStr;
    if (!capClass.empty()) {
        capClass.front() =
            static_cast<char>(std::toupper(static_cast<unsigned char>(capClass.front())));
    }
    if (name.rfind(capClass, 0) == 0) {
        name = name.substr(capClass.size());
    }
    if (!name.empty()) {
        name.front() = static_cast<char>(std::tolower(static_cast<unsigned char>(name.front())));
    }
    return sanitizeMethodName(std::move(name));
}

//! Return true when a command is a DSA method for the given class — i.e. its
//! first parameter is a GLuint with class_ == classStr AND the name starts
//! with "glNamed" or "glCreate<Class>".
bool isDsaMethodForClass(const model::Command &command, const std::string &classStr) {
    if (command.params.empty()) {
        return false;
    }
    const auto &first = command.params.front();
    if (!isUintHandle(first) || !model::hasClass(first) || *first.classStr != classStr) {
        return false;
    }
    return command.name.rfind("glNamed", 0) == 0;
}

//! Render a DSA method's param list (skipping the leading handle param) and
//! its call-arg list (passing m_id.id for the handle, rest through helpers).
struct DsaMethodStrings {
    std::string paramsStr;
    std::string callArgsStr;
};

DsaMethodStrings buildDsaStrings(const model::Command &command, const EmitterContext &ctx) {
    DsaMethodStrings s;
    s.callArgsStr = "m_id.id";
    bool firstParam = true;
    bool skippedFirst = false;
    for (const auto &param : command.params) {
        if (!skippedFirst) {
            skippedFirst = true;
            continue;
        }
        if (firstParam) {
            s.paramsStr.append("");
        } else {
            s.paramsStr.append(", ");
        }
        s.paramsStr.append(
            generateParamDecl(param, command, ctx.handleClasses, ctx.groupRename, ctx.bitmaskGroups, ctx.emittedGroups));
        s.callArgsStr.append(", ");
        s.callArgsStr.append(
            generateCallArg(param, command, ctx.handleClasses, ctx.groupRename, ctx.bitmaskGroups, ctx.emittedGroups));
        firstParam = false;
    }
    return s;
}

} // namespace

nlohmann::json buildDsaClasses(const model::Registry &registry,
                               const model::ConsolidatedRequire &consolidated,
                               const EmitterContext &ctx) {
    nlohmann::json classes = nlohmann::json::array();

    for (const auto &[classStr, handleType] : ctx.handleClasses) {
        nlohmann::json methods = nlohmann::json::array();
        for (const auto &command : registry.commands()) {
            if (consolidated.commands.count(command.name) == 0) {
                continue;
            }
            if (!isDsaMethodForClass(command, classStr)) {
                continue;
            }

            const auto strings = buildDsaStrings(command, ctx);
            const auto returnType = model::toCString(command.returnType);

            int versionInt = 0;
            if (const auto it = ctx.commandVersionMap.find(command.name);
                it != ctx.commandVersionMap.end()) {
                try {
                    versionInt = model::versionToInt(it->second);
                } catch (const std::invalid_argument &) {
                    versionInt = 0;
                }
            }

            std::string docBrief;
            nlohmann::json docParams = nlohmann::json::array();
            if (const auto it = ctx.docs.find(command.name); it != ctx.docs.end()) {
                docBrief = it->second.brief;
                for (const auto &[paramName, desc] : it->second.params) {
                    docParams.push_back(nlohmann::json{{"name", paramName}, {"desc", desc}});
                }
            }

            methods.push_back(nlohmann::json{
                {"name", dsaMethodName(command.name, classStr)},
                {"return_type", returnType},
                {"params_str", strings.paramsStr},
                {"call_args_str", strings.callArgsStr},
                {"gl_name", command.name},
                {"raw_ptr", "glaze_" + command.name},
                {"version_int", versionInt},
                {"is_void_return", returnType == "void"},
                {"has_doc_brief", !docBrief.empty()},
                {"doc_brief", docBrief},
                {"doc_params", std::move(docParams)},
            });
        }

        if (methods.empty()) {
            continue;
        }

        classes.push_back(nlohmann::json{
            {"class_name", handleType},
            {"handle_type", handleType},
            {"methods", methods},
        });
    }

    return classes;
}

} // namespace glaze::cppgen::detail
