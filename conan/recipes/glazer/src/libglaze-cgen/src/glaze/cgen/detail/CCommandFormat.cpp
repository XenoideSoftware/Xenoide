#include "glaze/cgen/detail/CCommandFormat.h"

#include "glaze/model/TypeDecl.h"

#include <cctype>

namespace glaze::cgen::detail {

std::string commandPtrTypeName(const std::string &commandName) {
    std::string out = "PFN";
    out.reserve(commandName.size() + 7);
    for (char ch : commandName) {
        out.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
    }
    out.append("PROC");
    return out;
}

namespace {

std::string joinParams(const model::Command &command, const codegen::TypeMapper &typeMapper) {
    std::string out;
    bool first = true;
    for (const auto &param : command.params) {
        if (!first) {
            out.append(", ");
        }
        out.append(typeMapper.mapParamDecl(param, command));
        first = false;
    }
    return out;
}

} // namespace

std::string generateCommandPtrTypedef(const model::Command &command,
                                      const codegen::TypeMapper &typeMapper) {
    const auto returnTypeStr = model::toCString(command.returnType);
    const auto params = joinParams(command, typeMapper);
    const auto ptrType = commandPtrTypeName(command.name);
    return "typedef " + returnTypeStr + " (GLCALLCONV *" + ptrType + ")(" + params + ");";
}

std::string generateCommandPtrExternRaw(const model::Command &command) {
    const auto ptrType = commandPtrTypeName(command.name);
    return "extern GLAZE_API " + ptrType + " glaze_" + command.name + ";";
}

std::string generateDebugWrapperDecl(const model::Command &command,
                                     const codegen::TypeMapper &typeMapper) {
    const auto returnTypeStr = model::toCString(command.returnType);
    auto params = joinParams(command, typeMapper);
    if (params.empty()) {
        params = "void";
    }
    return "GLAZE_API " + returnTypeStr + " GLCALLCONV glaze_debug_" + command.name + "(" + params + ");";
}

} // namespace glaze::cgen::detail
