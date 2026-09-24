#include "glaze/cppgen/detail/OverloadEmitter.h"

#include "glaze/cppgen/detail/ParamFormat.h"
#include "glaze/cppgen/detail/PatternDetector.h"
#include "glaze/model/CommandParam.h"

#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace glaze::cppgen::detail {

namespace {

std::string joinParams(const std::vector<std::string> &parts) {
    std::string out;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i != 0) {
            out.append(", ");
        }
        out.append(parts[i]);
    }
    return out;
}

//! Shared param/arg expansion that skips any params identified by pointer
//! comparison. The caller provides replacements for the skipped ones by
//! name so we can splice the custom body arguments in-place.
struct ExpandSpec {
    //! Parameters to EXCLUDE from the operator()'s C++ signature entirely.
    std::vector<const model::CommandParam *> dropFromSignature;
    //! Replacements for the C call. Keyed by parameter pointer identity.
    //! Every param not in this map is expanded via generateCallArg.
    std::vector<std::pair<const model::CommandParam *, std::string>> callSubstitutions;
    //! Extra C++ parameter declarations appended AFTER the surviving ones.
    std::vector<std::string> extraDecls;
};

bool contains(const std::vector<const model::CommandParam *> &vec,
              const model::CommandParam *p) {
    for (const auto *entry : vec) {
        if (entry == p) {
            return true;
        }
    }
    return false;
}

//! Lookup a substitution for a given param pointer, or nullptr if none.
const std::string *findSubstitution(const ExpandSpec &spec,
                                    const model::CommandParam *p) {
    for (const auto &entry : spec.callSubstitutions) {
        if (entry.first == p) {
            return &entry.second;
        }
    }
    return nullptr;
}

std::string buildParamsStr(const model::Command &command,
                           const EmitterContext &ctx, const ExpandSpec &spec) {
    std::vector<std::string> decls;
    for (const auto &param : command.params) {
        if (contains(spec.dropFromSignature, &param)) {
            continue;
        }
        decls.push_back(generateParamDecl(param, command, ctx.handleClasses,
                                          ctx.groupRename, ctx.bitmaskGroups,
                                          ctx.emittedGroups));
    }
    for (const auto &extra : spec.extraDecls) {
        decls.push_back(extra);
    }
    return joinParams(decls);
}

std::string buildCallArgsStr(const model::Command &command,
                             const EmitterContext &ctx, const ExpandSpec &spec) {
    std::vector<std::string> parts;
    for (const auto &param : command.params) {
        if (const auto *sub = findSubstitution(spec, &param); sub != nullptr) {
            parts.push_back(*sub);
            continue;
        }
        parts.push_back(generateCallArg(param, command, ctx.handleClasses,
                                        ctx.groupRename, ctx.bitmaskGroups,
                                        ctx.emittedGroups));
    }
    return joinParams(parts);
}

nlohmann::json makeOverload(std::string returnType, std::string paramsStr,
                            std::string body, bool isTemplate = false) {
    return nlohmann::json{
        {"return_type", std::move(returnType)},
        {"params_str", std::move(paramsStr)},
        {"body", std::move(body)},
        {"is_template_overload", isTemplate},
    };
}

// ── String return overload ─────────────────────────────────────────────────

std::optional<nlohmann::json>
buildStringOverload(const model::Command &command, const EmitterContext &ctx) {
    if (isInfoLogCommand(command)) {
        // Info-log commands get their dedicated zero-arg overload instead;
        // emitting both would duplicate the signature with just len param.
        return std::nullopt;
    }
    const auto *stringParam = findStringOutputParam(command);
    if (stringParam == nullptr) {
        return std::nullopt;
    }
    const auto *lengthParam = findLengthParam(command);

    ExpandSpec spec;
    spec.dropFromSignature.push_back(stringParam);
    if (lengthParam != nullptr) {
        spec.dropFromSignature.push_back(lengthParam);
    }
    spec.callSubstitutions.emplace_back(stringParam, "&result[0]");
    if (lengthParam != nullptr) {
        spec.callSubstitutions.emplace_back(lengthParam, "&written");
    }

    const auto paramsStr = buildParamsStr(command, ctx, spec);
    const auto callArgs = buildCallArgsStr(command, ctx, spec);

    std::ostringstream body;
    body << "        std::string result(static_cast<std::size_t>("
         << *stringParam->len << "), '\\0');\n";
    if (lengthParam != nullptr) {
        body << "        GLsizei written = 0;\n";
    }
    body << "        ::" << command.name << "(" << callArgs << ");\n";
    if (lengthParam != nullptr) {
        body << "        result.resize(static_cast<std::size_t>(written));\n";
    }
    body << "        return result;";

    return makeOverload("std::string", paramsStr, body.str());
}

// ── Scalar query overload ──────────────────────────────────────────────────

std::optional<nlohmann::json>
buildScalarQueryOverload(const model::Command &command,
                         const EmitterContext &ctx) {
    const auto *scalar = findScalarQueryParam(command);
    if (scalar == nullptr) {
        return std::nullopt;
    }
    const auto scalarBase = model::baseType(*scalar);
    if (!scalarBase) {
        return std::nullopt;
    }

    ExpandSpec spec;
    spec.dropFromSignature.push_back(scalar);
    spec.callSubstitutions.emplace_back(scalar, "&result");

    const auto paramsStr = buildParamsStr(command, ctx, spec);
    const auto callArgs = buildCallArgsStr(command, ctx, spec);

    std::ostringstream body;
    body << "        " << *scalarBase << " result{};\n";
    body << "        ::" << command.name << "(" << callArgs << ");\n";
    body << "        return result;";

    return makeOverload(*scalarBase, paramsStr, body.str());
}

// ── InfoLog self-query overload ────────────────────────────────────────────

std::optional<nlohmann::json>
buildInfoLogSelfQueryOverload(const model::Command &command,
                              const EmitterContext &ctx) {
    if (!isInfoLogCommand(command)) {
        return std::nullopt;
    }
    const auto &map = infoLogSelfQueryMap();
    const auto it = map.find(command.name);
    if (it == map.end() || command.params.empty()) {
        return std::nullopt;
    }
    const auto &handleParam = command.params.front();

    // Overload signature keeps only the leading handle param.
    const auto handleDecl = generateParamDecl(
        handleParam, command, ctx.handleClasses, ctx.groupRename,
        ctx.bitmaskGroups, ctx.emittedGroups);
    const auto handleCall = generateCallArg(
        handleParam, command, ctx.handleClasses, ctx.groupRename,
        ctx.bitmaskGroups, ctx.emittedGroups);

    std::ostringstream body;
    body << "        GLint length = 0;\n";
    body << "        " << it->second << "(" << handleCall
         << ", GL_INFO_LOG_LENGTH, &length);\n";
    body << "        if (length <= 0) { return std::string{}; }\n";
    body << "        std::string result(static_cast<std::size_t>(length), '\\0');\n";
    body << "        GLsizei written = 0;\n";
    body << "        ::" << command.name << "(" << handleCall
         << ", length, &written, &result[0]);\n";
    body << "        result.resize(static_cast<std::size_t>(written));\n";
    body << "        return result;";

    return makeOverload("std::string", handleDecl, body.str());
}

// ── ArrayView<T> upload overload ───────────────────────────────────────────

std::optional<nlohmann::json>
buildArrayViewOverload(const model::Command &command,
                       const EmitterContext &ctx) {
    const auto pair = findDataUploadParams(command);
    if (!pair.has_value()) {
        return std::nullopt;
    }
    const auto *dataParam = pair->data;
    const auto *sizeParam = pair->size;
    const auto sizeBase = model::baseType(*sizeParam);
    if (!sizeBase) {
        return std::nullopt;
    }

    ExpandSpec spec;
    spec.dropFromSignature.push_back(dataParam);
    spec.dropFromSignature.push_back(sizeParam);
    spec.callSubstitutions.emplace_back(
        sizeParam, "static_cast<" + *sizeBase + ">(data.size_bytes())");
    spec.callSubstitutions.emplace_back(dataParam, "data.data()");
    spec.extraDecls.emplace_back("const ArrayView<T>& data");

    const auto paramsStr = buildParamsStr(command, ctx, spec);
    const auto callArgs = buildCallArgsStr(command, ctx, spec);

    const auto returnType = command.returnType.name == "void" &&
                                     !command.returnType.isPointer
                                 ? std::string{"void"}
                                 : model::toCString(command.returnType);

    std::ostringstream body;
    if (returnType == "void") {
        body << "        ::" << command.name << "(" << callArgs << ");";
    } else {
        body << "        return ::" << command.name << "(" << callArgs << ");";
    }

    return makeOverload(returnType, paramsStr, body.str(), /*isTemplate=*/true);
}

// ── Singular object creation overload ──────────────────────────────────────

std::optional<nlohmann::json>
buildSingleObjectCreationOverload(const model::Command &command,
                                  const EmitterContext &ctx) {
    const model::CommandParam *count = nullptr;
    const model::CommandParam *output = nullptr;
    if (!findObjectCreationParams(command, count, output)) {
        return std::nullopt;
    }
    const auto handleIt = ctx.handleClasses.find(*output->classStr);
    if (handleIt == ctx.handleClasses.end()) {
        return std::nullopt;
    }
    const auto &handleName = handleIt->second;

    ExpandSpec spec;
    spec.dropFromSignature.push_back(count);
    spec.dropFromSignature.push_back(output);
    spec.callSubstitutions.emplace_back(count, "1");
    spec.callSubstitutions.emplace_back(
        output, "reinterpret_cast<GLuint*>(&obj)");

    const auto paramsStr = buildParamsStr(command, ctx, spec);
    const auto callArgs = buildCallArgsStr(command, ctx, spec);

    std::ostringstream body;
    body << "        " << handleName << " obj{};\n";
    body << "        ::" << command.name << "(" << callArgs << ");\n";
    body << "        return obj;";

    return makeOverload(handleName, paramsStr, body.str());
}

// ── Singular object deletion overload ──────────────────────────────────────

std::optional<nlohmann::json>
buildSingleObjectDeletionOverload(const model::Command &command,
                                  const EmitterContext &ctx) {
    const model::CommandParam *count = nullptr;
    const model::CommandParam *input = nullptr;
    if (!findObjectDeletionParams(command, count, input)) {
        return std::nullopt;
    }
    const auto handleIt = ctx.handleClasses.find(*input->classStr);
    if (handleIt == ctx.handleClasses.end()) {
        return std::nullopt;
    }
    const auto &handleName = handleIt->second;

    ExpandSpec spec;
    spec.dropFromSignature.push_back(count);
    spec.dropFromSignature.push_back(input);
    spec.extraDecls.push_back(handleName + " obj");
    spec.callSubstitutions.emplace_back(count, "1");
    spec.callSubstitutions.emplace_back(
        input, "reinterpret_cast<const GLuint*>(&obj)");

    const auto paramsStr = buildParamsStr(command, ctx, spec);
    const auto callArgs = buildCallArgsStr(command, ctx, spec);

    std::ostringstream body;
    body << "        ::" << command.name << "(" << callArgs << ");";

    return makeOverload("void", paramsStr, body.str());
}

} // namespace

nlohmann::json buildExtraOverloads(const model::Command &command,
                                   const EmitterContext &ctx) {
    nlohmann::json overloads = nlohmann::json::array();
    if (auto ov = buildStringOverload(command, ctx)) {
        overloads.push_back(std::move(*ov));
    }
    if (auto ov = buildScalarQueryOverload(command, ctx)) {
        overloads.push_back(std::move(*ov));
    }
    if (auto ov = buildInfoLogSelfQueryOverload(command, ctx)) {
        overloads.push_back(std::move(*ov));
    }
    if (auto ov = buildArrayViewOverload(command, ctx)) {
        overloads.push_back(std::move(*ov));
    }
    if (auto ov = buildSingleObjectCreationOverload(command, ctx)) {
        overloads.push_back(std::move(*ov));
    }
    if (auto ov = buildSingleObjectDeletionOverload(command, ctx)) {
        overloads.push_back(std::move(*ov));
    }
    return overloads;
}

} // namespace glaze::cppgen::detail
