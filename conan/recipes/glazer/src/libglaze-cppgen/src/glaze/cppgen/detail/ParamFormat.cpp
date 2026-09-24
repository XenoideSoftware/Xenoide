#include "glaze/cppgen/detail/ParamFormat.h"

#include "glaze/cppgen/detail/PatternDetector.h"

#include <sstream>
#include <utility>

namespace glaze::cppgen::detail {

namespace {

//! Reconstruct the raw type portion of a param (const / type / *) without the
//! name token, so we can substitute a handle or enum spelling into it.
std::string rawTypeString(const model::CommandParam &param) {
    std::string out;
    for (const auto &part : param.typeParts) {
        if (part.empty()) {
            continue;
        }
        if (!out.empty() && part != "*" && part != "**") {
            out.push_back(' ');
        }
        out.append(part);
    }
    return out;
}

} // namespace

std::string paramTypeStr(const model::CommandParam &param,
                         const model::Command &command,
                         const std::map<std::string, std::string> &handleClasses,
                         const std::map<std::string, std::string> &groupRename,
                         const std::set<std::string> &bitmaskGroups,
                         const std::set<std::string> &emittedGroups) {
    // 1. Location-type overrides (name-based heuristics).
    if (isUniformLocationParam(command, param)) {
        return "UniformLocation";
    }
    if (isAttribLocationParam(command, param)) {
        return "AttribLocation";
    }

    // 2. Handle-type substitution for GLuint params with a class_ attribute.
    if (model::hasClass(param) && isUintHandle(param)) {
        const auto handleIt = handleClasses.find(*param.classStr);
        if (handleIt != handleClasses.end()) {
            if (model::isPointer(param)) {
                // Pointer to a handle-typed array is unusual at the high-level
                // C++ API; we pass it through as raw GLuint* and the call arg
                // helper reinterpret_casts when bridging back into C.
                std::string out = model::isConst(param) ? "const " : "";
                out.append("GLuint *");
                return out;
            }
            return handleIt->second;
        }
    }

    // 3. Enum-group substitution for params with a matching group attribute.
    //    Pointer/array params (e.g. `const GLenum *attachments`) keep their raw
    //    C spelling — wrapping an array pointer in a scalar enum type would be
    //    a type error at the call site. Mirrors the handle-pointer branch.
    //    We also skip substitution when the group has no entries in the
    //    consolidated enum set for this api profile (e.g. legacy MapTarget /
    //    PixelMap in core 4.6) — those enum classes never get emitted, so
    //    naming them here would produce an undeclared-identifier error.
    if (model::hasGroup(param) && !model::isPointer(param) &&
        emittedGroups.count(*param.group) != 0) {
        const auto clean = groupRename.count(*param.group) != 0
                               ? groupRename.at(*param.group)
                               : *param.group;
        if (bitmaskGroups.count(*param.group) != 0) {
            return "Flags<" + clean + ">";
        }
        return clean;
    }

    // 4. Fall through to the raw type spelling.
    return rawTypeString(param);
}

std::string generateParamDecl(const model::CommandParam &param,
                              const model::Command &command,
                              const std::map<std::string, std::string> &handleClasses,
                              const std::map<std::string, std::string> &groupRename,
                              const std::set<std::string> &bitmaskGroups,
                              const std::set<std::string> &emittedGroups) {
    auto type = paramTypeStr(param, command, handleClasses, groupRename, bitmaskGroups,
                             emittedGroups);
    type.push_back(' ');
    type.append(param.name);
    return type;
}

std::string generateCallArg(const model::CommandParam &param,
                            const model::Command &command,
                            const std::map<std::string, std::string> &handleClasses,
                            const std::map<std::string, std::string> &groupRename,
                            const std::set<std::string> &bitmaskGroups,
                            const std::set<std::string> &emittedGroups) {
    (void)groupRename; // reserved for future overloads that substitute by rename
    // Location overrides extract the raw value and optionally cast to GLuint.
    if (isUniformLocationParam(command, param)) {
        return param.name + ".loc";
    }
    if (isAttribLocationParam(command, param)) {
        return "static_cast<GLuint>(" + param.name + ".loc)";
    }

    // Handle substitution: scalars pass .id, pointers reinterpret_cast through.
    if (model::hasClass(param) && isUintHandle(param)) {
        const auto it = handleClasses.find(*param.classStr);
        if (it != handleClasses.end()) {
            if (!model::isPointer(param)) {
                return param.name + ".id";
            }
            // Pointer path: stay as GLuint*/const GLuint* per paramTypeStr.
            return param.name;
        }
    }

    // Enum-group substitution: static_cast or .value() for bitmasks. Pointer
    // params kept their raw spelling in paramTypeStr(), so forward as-is. We
    // also forward the raw spelling when the group wasn't emitted — paramTypeStr
    // already degraded the type to GLenum, so no wrapping / unwrapping needed.
    if (model::hasGroup(param) && !model::isPointer(param) &&
        emittedGroups.count(*param.group) != 0) {
        if (bitmaskGroups.count(*param.group) != 0) {
            return param.name + ".value()";
        }
        return "static_cast<GLenum>(" + param.name + ")";
    }

    return param.name;
}

std::string returnTypeStr(const model::Command &command,
                          const std::map<std::string, std::string> &handleClasses) {
    if (isStringReturnCommand(command)) {
        return "std::string";
    }
    if (isUniformLocationReturn(command)) {
        return "UniformLocation";
    }
    if (isAttribLocationReturn(command)) {
        return "AttribLocation";
    }
    if (command.returnType.name == "GLuint" && !command.returnType.isPointer) {
        // Singular create commands (glCreateProgram, glCreateShader) return a
        // handle; map to the first discovered handle class of matching name.
        // The orchestrator supplies a better mapping when the command's class
        // context is available. Here we fall through to raw GLuint if no
        // match exists.
        (void)handleClasses;
    }
    return model::toCString(command.returnType);
}

} // namespace glaze::cppgen::detail
