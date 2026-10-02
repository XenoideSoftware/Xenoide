#include "glaze/cppgen/detail/RaiiCollector.h"

#include "glaze/cppgen/detail/ParamFormat.h"
#include "glaze/cppgen/detail/PatternDetector.h"
#include "glaze/model/StringUtils.h"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace glaze::cppgen::detail {

namespace {

//! Convert a gl command name such as "glGenBuffers" into the canonical
//! lowerCamel functor instance name: "genBuffers". The Traits<T>::create()
//! body invokes this name and relies on the parameterless overload that the
//! functor exposes (e.g. `Buffer GenBuffersFn::operator()() const`) — so we
//! must NOT singularize it here, otherwise the call resolves to a name that
//! doesn't exist in the gl:: namespace.
std::string makeGenFunctorName(const std::string &glName) {
    std::string rest;
    if (glName.rfind("gl", 0) == 0 && glName.size() > 2) {
        rest = glName.substr(2);
    } else {
        rest = glName;
    }
    if (!rest.empty()) {
        rest.front() = static_cast<char>(std::tolower(static_cast<unsigned char>(rest.front())));
    }
    return rest;
}

std::string makeDeleteFunctorName(const std::string &glName) {
    std::string rest;
    if (glName.rfind("gl", 0) == 0 && glName.size() > 2) {
        rest = glName.substr(2);
    } else {
        rest = glName;
    }
    if (!rest.empty()) {
        rest.front() = static_cast<char>(std::tolower(static_cast<unsigned char>(rest.front())));
    }
    return rest;
}

} // namespace

namespace {

//! Build the "ShaderType type, GLenum extra" parameter list and the matching
//! "type, static_cast<GLenum>(extra)" call-arg list for whichever params on
//! the creator survive after dropping the count + output. Matches the Python
//! _collect_raii_resources code at lines 663–682.
struct ExtraParamStrings {
    std::string paramsStr;
    std::string callArgsStr;
};

//! The Traits<> specialization lives in namespace glaze, so any enum/handle
//! type name returned by paramTypeStr() (which is unqualified, assuming the
//! api namespace as scope) has to be rewritten to `api::Type` before it is
//! emitted into the Traits<>::create() signature. Raw C types (`GLenum`,
//! `const GLuint *`, `void`) are left alone because they are global typedefs.
std::string qualifyWrapperType(std::string type, const std::string &api) {
    auto startsWith = [&type](const char *prefix) {
        const std::size_t n = std::char_traits<char>::length(prefix);
        return type.size() >= n && type.compare(0, n, prefix) == 0;
    };
    if (startsWith("const ") || startsWith("GL") || startsWith("void")) {
        return type;
    }
    if (startsWith("Flags<")) {
        const auto close = type.rfind('>');
        if (close != std::string::npos && close > 6) {
            const auto inner = type.substr(6, close - 6);
            return api + "::Flags<" + api + "::" + inner + ">";
        }
    }
    return api + "::" + type;
}

ExtraParamStrings buildExtraParams(const model::Command &command,
                                   const model::CommandParam *countParam,
                                   const model::CommandParam *outputParam,
                                   const EmitterContext &ctx) {
    ExtraParamStrings s;
    bool first = true;
    for (const auto &param : command.params) {
        if (&param == countParam || &param == outputParam) {
            continue;
        }
        if (!first) {
            s.paramsStr.append(", ");
            s.callArgsStr.append(", ");
        }
        auto type = paramTypeStr(param, command, ctx.handleClasses,
                                 ctx.groupRename, ctx.bitmaskGroups,
                                 ctx.emittedGroups);
        s.paramsStr.append(qualifyWrapperType(std::move(type), ctx.api));
        s.paramsStr.push_back(' ');
        s.paramsStr.append(param.name);
        // Traits<H>::create() invokes the C++ functor (e.g. gl::createShader),
        // not the raw C pointer — the functor handles any static_cast/.value()
        // unwrapping internally. So we forward the typed param unchanged;
        // generateCallArg would otherwise double-unwrap ShaderType -> GLenum
        // and break overload resolution.
        s.callArgsStr.append(param.name);
        first = false;
    }
    return s;
}

int versionIntFor(const model::Command *command, const EmitterContext &ctx) {
    if (command == nullptr) {
        return 0;
    }
    const auto it = ctx.commandVersionMap.find(command->name);
    if (it == ctx.commandVersionMap.end()) {
        return 0;
    }
    try {
        return model::versionToInt(it->second);
    } catch (const std::invalid_argument &) {
        return 0;
    }
}

} // namespace

nlohmann::json collectRaiiResources(const model::Registry &registry,
                                    const model::ConsolidatedRequire &consolidated,
                                    const EmitterContext &ctx) {
    nlohmann::json out = nlohmann::json::array();
    const auto &handleClasses = ctx.handleClasses;

    // Index commands by GL name for quick lookup.
    for (const auto &[classStr, handleType] : handleClasses) {
        // Search for a creator: glGen<Class>s(count, out) or glCreate<Class>()
        // and a deleter: glDelete<Class>s(count, in) or glDelete<Class>(id).
        const model::Command *multiGen = nullptr;
        const model::Command *multiDel = nullptr;
        const model::Command *singleGen = nullptr;
        const model::Command *singleDel = nullptr;

        for (const auto &cmd : registry.commands()) {
            if (consolidated.commands.count(cmd.name) == 0) {
                continue;
            }
            const model::CommandParam *countParam = nullptr;
            const model::CommandParam *handleParam = nullptr;

            if (findObjectCreationParams(cmd, countParam, handleParam) &&
                handleParam->classStr == classStr) {
                multiGen = &cmd;
                continue;
            }
            if (findObjectDeletionParams(cmd, countParam, handleParam) &&
                handleParam->classStr == classStr) {
                multiDel = &cmd;
                continue;
            }
            // Singular creator: no params, returns GLuint
            if (cmd.name.rfind("glCreate", 0) == 0 && cmd.returnType.name == "GLuint" &&
                !cmd.returnType.isPointer) {
                // Match class from command name suffix, e.g. glCreateProgram -> program
                std::string suffix = cmd.name.substr(8); // drop "glCreate"
                for (auto &c : suffix) {
                    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                }
                if (suffix == classStr) {
                    // accept when params are empty OR first param has no class
                    if (cmd.params.empty() ||
                        (cmd.params.front().typeParts.size() >= 1 &&
                         !model::hasClass(cmd.params.front()))) {
                        singleGen = &cmd;
                    }
                }
            }
            // Singular deleter: single class-tagged GLuint param
            if (cmd.name.rfind("glDelete", 0) == 0 && cmd.params.size() == 1) {
                const auto &p = cmd.params.front();
                if (isUintHandle(p) && !model::isPointer(p) && model::hasClass(p) &&
                    *p.classStr == classStr) {
                    singleDel = &cmd;
                }
            }
        }

        const model::Command *chosenGen = multiGen != nullptr ? multiGen : singleGen;
        const model::Command *chosenDel = multiDel != nullptr ? multiDel : singleDel;
        if (chosenGen == nullptr || chosenDel == nullptr) {
            continue;
        }

        // Walk the creator params, skipping the count/output pair that the
        // singular overload hides. For singular creators (e.g.
        // glCreateShader) the entire param list survives.
        const model::CommandParam *countParam = nullptr;
        const model::CommandParam *outputParam = nullptr;
        if (chosenGen == multiGen) {
            findObjectCreationParams(*chosenGen, countParam, outputParam);
        }
        const auto extras = buildExtraParams(*chosenGen, countParam, outputParam, ctx);

        // Gate the Traits specialization behind the latest GLAZE_GL_VERSION
        // of the creator and deleter so we never emit Traits for symbols the
        // feature floor excludes.
        const int genVer = versionIntFor(chosenGen, ctx);
        const int delVer = versionIntFor(chosenDel, ctx);
        const int versionInt = std::max(genVer, delVer);

        nlohmann::json entry;
        entry["alias"] = handleType;
        entry["handle_type"] = handleType;
        entry["gen_gl_name"] = chosenGen->name;
        entry["gen_func_name"] = makeGenFunctorName(chosenGen->name);
        entry["delete_gl_name"] = chosenDel->name;
        entry["delete_func_name"] = makeDeleteFunctorName(chosenDel->name);
        entry["create_params_str"] = extras.paramsStr;
        entry["create_call_args_str"] = extras.callArgsStr;
        entry["version_int"] = versionInt;
        out.push_back(entry);
    }

    return out;
}

} // namespace glaze::cppgen::detail
