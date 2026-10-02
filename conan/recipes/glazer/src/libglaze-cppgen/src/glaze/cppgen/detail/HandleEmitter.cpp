#include "glaze/cppgen/detail/HandleEmitter.h"

#include "glaze/cppgen/detail/CppKeywords.h"
#include "glaze/cppgen/detail/ParamFormat.h"
#include "glaze/cppgen/detail/PatternDetector.h"
#include "glaze/model/StringUtils.h"
#include "glaze/model/TypeDecl.h"

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

namespace glaze::cppgen::detail {

namespace {

//! Turn a raw gl command name into a lowerCamel method name. Mirrors
//! cpp_generator.py::_handle_method_name — we simply strip "gl" and
//! lowercase the first remaining letter. The class portion is NOT stripped
//! here because legacy handle methods keep names like bindBuffer(), linkProgram().
std::string handleMethodName(const std::string &glName) {
    if (glName.size() < 3 || glName[0] != 'g' || glName[1] != 'l') {
        return glName;
    }
    std::string out;
    out.reserve(glName.size() - 2);
    out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(glName[2]))));
    out.append(glName.substr(3));
    return sanitizeMethodName(std::move(out));
}

//! True when a command is a legacy handle method for the given class: first
//! param has class_ == classStr AND the command name does NOT start with glNamed*.
bool isHandleMethodForClass(const model::Command &command, const std::string &classStr) {
    if (command.params.empty()) {
        return false;
    }
    const auto &first = command.params.front();
    if (!isUintHandle(first) || !model::hasClass(first) || *first.classStr != classStr) {
        return false;
    }
    return command.name.rfind("glNamed", 0) != 0;
}

//! Join a list of "TypeName name" param declarations with ", ".
std::string joinDecls(const std::vector<std::string> &decls) {
    std::string out;
    for (std::size_t i = 0; i < decls.size(); ++i) {
        if (i != 0) {
            out.append(", ");
        }
        out.append(decls[i]);
    }
    return out;
}

//! Build a decl list for every param whose pointer is NOT in the skip set.
std::vector<std::string>
paramDecls(const model::Command &command, const EmitterContext &ctx,
           const std::vector<const model::CommandParam *> &skip) {
    std::vector<std::string> decls;
    // Always skip the leading handle param — the handle wrapper owns it.
    for (std::size_t i = 1; i < command.params.size(); ++i) {
        const auto &param = command.params[i];
        bool skipped = false;
        for (const auto *p : skip) {
            if (p == &param) {
                skipped = true;
                break;
            }
        }
        if (skipped) {
            continue;
        }
        decls.push_back(generateParamDecl(param, command, ctx.handleClasses,
                                          ctx.groupRename, ctx.bitmaskGroups,
                                          ctx.emittedGroups));
    }
    return decls;
}

//! Build a call-arg list for the free functor delegation, starting with
//! "m_id" and appending each param's camelCase name. Skipped params are
//! omitted — the functor's overload resolution figures out which overload
//! the handle method targets.
std::string callArgs(const model::Command &command,
                     const std::vector<const model::CommandParam *> &skip) {
    std::string out = "m_id";
    for (std::size_t i = 1; i < command.params.size(); ++i) {
        const auto &param = command.params[i];
        bool skipped = false;
        for (const auto *p : skip) {
            if (p == &param) {
                skipped = true;
                break;
            }
        }
        if (skipped) {
            continue;
        }
        out.append(", ");
        out.append(param.name);
    }
    return out;
}

std::string buildDelegationBody(const std::string &api,
                                const std::string &functorName,
                                const std::string &callArgsStr,
                                bool isVoid) {
    std::ostringstream body;
    if (isVoid) {
        body << "        ::" << api << "::" << functorName << "("
             << callArgsStr << ");";
    } else {
        body << "        return ::" << api << "::" << functorName << "("
             << callArgsStr << ");";
    }
    return body.str();
}

//! Canonical return type of a handle method — mirrors the functor's return
//! type rules (string return / location / raw C type).
std::string canonicalReturnType(const model::Command &command) {
    if (isStringReturnCommand(command)) {
        return "std::string";
    }
    if (isUniformLocationReturn(command)) {
        return "UniformLocation";
    }
    if (isAttribLocationReturn(command)) {
        return "AttribLocation";
    }
    return model::toCString(command.returnType);
}

nlohmann::json makeMethod(const std::string &name,
                          const std::string &returnType,
                          const std::string &paramsStr,
                          const std::string &body, int versionInt,
                          const std::string &docBrief = "",
                          nlohmann::json docParams = nlohmann::json::array()) {
    return nlohmann::json{
        {"name", name},
        {"return_type", returnType},
        {"params_str", paramsStr},
        {"body", body},
        {"version_int", versionInt},
        {"has_doc_brief", !docBrief.empty()},
        {"doc_brief", docBrief},
        {"doc_params", std::move(docParams)},
    };
}

int commandVersion(const model::Command &command, const EmitterContext &ctx) {
    if (const auto it = ctx.commandVersionMap.find(command.name);
        it != ctx.commandVersionMap.end()) {
        try {
            return model::versionToInt(it->second);
        } catch (const std::invalid_argument &) {
            return 0;
        }
    }
    return 0;
}

//! Emit every applicable method entry for one command.  The canonical
//! overload is always present; the specialized shapes are added when the
//! corresponding pattern detector matches.
void appendMethods(nlohmann::json &methods, const model::Command &command,
                   const EmitterContext &ctx, const std::string &api) {
    const auto name = handleMethodName(command.name);
    const auto functorName = ctx.nameTransform.transformCommandName(command.name);
    const auto versionInt = commandVersion(command, ctx);

    std::string docBrief;
    nlohmann::json docParams = nlohmann::json::array();
    if (const auto it = ctx.docs.find(command.name); it != ctx.docs.end()) {
        docBrief = it->second.brief;
        for (const auto &[paramName, desc] : it->second.params) {
            docParams.push_back(nlohmann::json{{"name", paramName}, {"desc", desc}});
        }
    }

    // 1. Canonical method — every param after the handle.
    {
        const auto decls = paramDecls(command, ctx, {});
        const auto argsStr = callArgs(command, {});
        const auto rt = canonicalReturnType(command);
        methods.push_back(makeMethod(
            name, rt, joinDecls(decls),
            buildDelegationBody(api, functorName, argsStr, rt == "void"),
            versionInt, docBrief, docParams));
    }

    // 2. Scalar query overload (drops the trailing scalar out-pointer).
    if (const auto *scalar = findScalarQueryParam(command); scalar != nullptr) {
        const auto base = model::baseType(*scalar);
        if (base) {
            const auto decls = paramDecls(command, ctx, {scalar});
            const auto argsStr = callArgs(command, {scalar});
            methods.push_back(makeMethod(
                name, *base, joinDecls(decls),
                buildDelegationBody(api, functorName, argsStr,
                                    /*isVoid=*/false),
                versionInt, docBrief, docParams));
        }
    }

    // 3. std::string output overload (drops GLchar* buffer + length sink).
    if (const auto *str = findStringOutputParam(command); str != nullptr) {
        std::vector<const model::CommandParam *> skip{str};
        if (const auto *len = findLengthParam(command); len != nullptr) {
            skip.push_back(len);
        }
        const auto decls = paramDecls(command, ctx, skip);
        const auto argsStr = callArgs(command, skip);
        methods.push_back(makeMethod(
            name, "std::string", joinDecls(decls),
            buildDelegationBody(api, functorName, argsStr, /*isVoid=*/false),
            versionInt, docBrief, docParams));
    }

    // 4. InfoLog self-query overload (zero extra args, returns std::string).
    if (isInfoLogCommand(command)) {
        // The zero-arg overload's delegation is just "::gl::func(m_id)".
        methods.push_back(makeMethod(
            name, "std::string", /*paramsStr=*/"",
            buildDelegationBody(api, functorName, "m_id", /*isVoid=*/false),
            versionInt, docBrief, docParams));
    }
}

} // namespace

nlohmann::json buildHandleClasses(const model::Registry &registry,
                                  const model::ConsolidatedRequire &consolidated,
                                  const EmitterContext &ctx) {
    nlohmann::json classes = nlohmann::json::array();

    const auto &api = ctx.api;

    for (const auto &[classStr, handleType] : ctx.handleClasses) {
        nlohmann::json methods = nlohmann::json::array();
        for (const auto &command : registry.commands()) {
            if (consolidated.commands.count(command.name) == 0) {
                continue;
            }
            if (!isHandleMethodForClass(command, classStr)) {
                continue;
            }
            appendMethods(methods, command, ctx, api);
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
