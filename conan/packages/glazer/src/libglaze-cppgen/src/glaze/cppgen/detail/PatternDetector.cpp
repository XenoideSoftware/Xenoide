#include "glaze/cppgen/detail/PatternDetector.h"

#include <array>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace glaze::cppgen::detail {

namespace {

constexpr std::array<std::string_view, 3> kUniformLocationReturnCommands{{
    "glGetUniformLocation",
    "glGetFragDataLocation",
    "glGetFragDataIndex",
}};

constexpr std::array<std::string_view, 1> kAttribLocationReturnCommands{{
    "glGetAttribLocation",
}};

bool containsName(const std::array<std::string_view, 3> &arr, std::string_view name) {
    for (const auto &entry : arr) {
        if (entry == name) {
            return true;
        }
    }
    return false;
}

bool containsName(const std::array<std::string_view, 1> &arr, std::string_view name) {
    return arr[0] == name;
}

bool startsWith(const std::string &s, std::string_view prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

bool contains(const std::string &s, std::string_view needle) {
    return s.find(needle) != std::string::npos;
}

} // namespace

bool isUintHandle(const model::CommandParam &param) noexcept {
    const auto base = model::baseType(param);
    return base && *base == "GLuint";
}

bool isStringReturnCommand(const model::Command &command) noexcept {
    const auto &rt = command.returnType;
    if (!rt.isConst || !rt.isPointer) {
        return false;
    }
    return rt.name == "GLubyte" || rt.name == "GLchar";
}

bool findObjectCreationParams(const model::Command &command,
                              const model::CommandParam *&countOut,
                              const model::CommandParam *&outputOut) noexcept {
    countOut = nullptr;
    outputOut = nullptr;
    if (!startsWith(command.name, "glGen") && !startsWith(command.name, "glCreate")) {
        return false;
    }
    for (const auto &param : command.params) {
        const auto base = model::baseType(param);
        if (base && *base == "GLsizei" && !model::isPointer(param)) {
            countOut = &param;
        }
        if (base && *base == "GLuint" && model::isPointer(param) && !model::isConst(param) &&
            model::hasClass(param)) {
            outputOut = &param;
        }
    }
    return countOut != nullptr && outputOut != nullptr;
}

bool findObjectDeletionParams(const model::Command &command,
                              const model::CommandParam *&countOut,
                              const model::CommandParam *&inputOut) noexcept {
    countOut = nullptr;
    inputOut = nullptr;
    if (!startsWith(command.name, "glDelete")) {
        return false;
    }
    for (const auto &param : command.params) {
        const auto base = model::baseType(param);
        if (base && *base == "GLsizei" && !model::isPointer(param)) {
            countOut = &param;
        }
        if (base && *base == "GLuint" && model::isPointer(param) && model::isConst(param) &&
            model::hasClass(param)) {
            inputOut = &param;
        }
    }
    return countOut != nullptr && inputOut != nullptr;
}

bool isUniformLocationParam(const model::Command &command,
                            const model::CommandParam &param) noexcept {
    const auto base = model::baseType(param);
    if (!base || *base != "GLint" || param.name != "location") {
        return false;
    }
    return startsWith(command.name, "glUniform") || startsWith(command.name, "glProgramUniform");
}

bool isAttribLocationParam(const model::Command &command,
                           const model::CommandParam &param) noexcept {
    const auto base = model::baseType(param);
    if (!base || *base != "GLuint" || param.name != "index") {
        return false;
    }
    return contains(command.name, "VertexAttrib") || command.name == "glBindAttribLocation";
}

bool isUniformLocationReturn(const model::Command &command) noexcept {
    return containsName(kUniformLocationReturnCommands, command.name);
}

bool isAttribLocationReturn(const model::Command &command) noexcept {
    return containsName(kAttribLocationReturnCommands, command.name);
}

const std::unordered_set<std::string> &scalarQueryTypes() {
    static const std::unordered_set<std::string> kTypes{
        "GLint", "GLuint",  "GLfloat",  "GLdouble",
        "GLboolean", "GLint64", "GLuint64",
    };
    return kTypes;
}

const std::unordered_map<std::string, std::string> &infoLogSelfQueryMap() {
    static const std::unordered_map<std::string, std::string> kMap{
        {"glGetShaderInfoLog", "::glGetShaderiv"},
        {"glGetProgramInfoLog", "::glGetProgramiv"},
    };
    return kMap;
}

const model::CommandParam *findScalarQueryParam(const model::Command &command) noexcept {
    if (!startsWith(command.name, "glGet")) {
        return nullptr;
    }
    if (command.params.size() < 2) {
        return nullptr;
    }
    const auto &first = command.params.front();
    {
        const auto firstBase = model::baseType(first);
        if (!model::hasClass(first) || !firstBase || *firstBase != "GLuint" ||
            model::isPointer(first)) {
            return nullptr;
        }
    }
    const auto &last = command.params.back();
    const auto lastBase = model::baseType(last);
    if (!lastBase || scalarQueryTypes().count(*lastBase) == 0 ||
        !model::isPointer(last) || model::isConst(last)) {
        return nullptr;
    }
    // Every interior param must be const or non-pointer — the trailing out
    // must be the only non-const output.
    for (std::size_t i = 1; i + 1 < command.params.size(); ++i) {
        const auto &p = command.params[i];
        if (model::isPointer(p) && !model::isConst(p)) {
            return nullptr;
        }
    }
    return &last;
}

namespace {

bool paramHasName(const model::Command &command, std::string_view name) {
    for (const auto &p : command.params) {
        if (p.name == name) {
            return true;
        }
    }
    return false;
}

const model::CommandParam *findByName(const model::Command &command,
                                      std::string_view name) {
    for (const auto &p : command.params) {
        if (p.name == name) {
            return &p;
        }
    }
    return nullptr;
}

} // namespace

const model::CommandParam *findStringOutputParam(const model::Command &command) noexcept {
    for (const auto &param : command.params) {
        const auto base = model::baseType(param);
        if (!base || *base != "GLchar") {
            continue;
        }
        if (!model::isPointer(param) || model::isConst(param)) {
            continue;
        }
        if (!param.len || param.len->empty()) {
            continue;
        }
        if (!paramHasName(command, *param.len)) {
            continue;
        }
        return &param;
    }
    return nullptr;
}

const model::CommandParam *findLengthParam(const model::Command &command) noexcept {
    for (const auto &param : command.params) {
        const auto base = model::baseType(param);
        if (!base || *base != "GLsizei") {
            continue;
        }
        if (!model::isPointer(param) || model::isConst(param)) {
            continue;
        }
        if (param.len && *param.len == "1") {
            return &param;
        }
    }
    return nullptr;
}

std::optional<DataUploadParams> findDataUploadParams(const model::Command &command) noexcept {
    for (const auto &param : command.params) {
        if (!model::isVoid(param) || !model::isPointer(param) || !model::isConst(param)) {
            continue;
        }
        if (!param.len || param.len->empty()) {
            continue;
        }
        const auto *sizeParam = findByName(command, *param.len);
        if (sizeParam == nullptr) {
            continue;
        }
        const auto sizeBase = model::baseType(*sizeParam);
        if (!sizeBase) {
            continue;
        }
        if ((*sizeBase != "GLsizei" && *sizeBase != "GLsizeiptr") ||
            model::isPointer(*sizeParam)) {
            continue;
        }
        return DataUploadParams{&param, sizeParam};
    }
    return std::nullopt;
}

bool isInfoLogCommand(const model::Command &command) noexcept {
    return infoLogSelfQueryMap().count(command.name) != 0;
}

} // namespace glaze::cppgen::detail
