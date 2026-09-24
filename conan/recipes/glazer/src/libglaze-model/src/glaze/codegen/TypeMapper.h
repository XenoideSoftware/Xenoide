#pragma once

#include "glaze/model/Command.h"
#include "glaze/model/CommandParam.h"

#include <string>
#include <string_view>

namespace glaze::codegen {

/**
 * @brief Customization point for how IR types render in the target language.
 * Extended description: the companion to NameTransform — while NameTransform
 * rewrites identifiers, TypeMapper rewrites type spellings. Generators use
 * this interface every time they need to turn a CommandParam or return type
 * into target-language syntax (parameter declarations, return types, cast
 * expressions). Concrete implementations exist per target language: a
 * DefaultCTypeMapper ships here for the C generator, and
 * libglaze-cppgen provides its own DefaultCppTypeMapper because the
 * default C++ behavior requires knowledge of the cpp generator's handle and
 * location substitution layer.
 */
class TypeMapper {
public:
    TypeMapper() = default;
    TypeMapper(const TypeMapper &) = default;
    TypeMapper(TypeMapper &&) = default;
    TypeMapper &operator=(const TypeMapper &) = default;
    TypeMapper &operator=(TypeMapper &&) = default;
    virtual ~TypeMapper() = default;

    /**
     * @brief Render a command parameter as a target-language declaration.
     * @param param the parameter to render
     * @param command the command the parameter belongs to (for context)
     * @return target-language text such as "const GLuint *buffers"
     */
    virtual std::string mapParamDecl(const model::CommandParam &param,
                                     const model::Command &command) const = 0;

    /**
     * @brief Render a command return type as target-language source text.
     * @param command the command whose return type to render
     */
    virtual std::string mapReturnType(const model::Command &command) const = 0;

    /**
     * @brief Render a base GL type name (e.g. "GLuint") in the target language.
     * Default for the C mapper is identity. The C++ mapper may substitute
     * strong types (e.g. "BufferId") when the parameter has extra metadata.
     */
    virtual std::string mapBaseType(std::string_view glType) const = 0;
};

/**
 * @brief Default TypeMapper for the C generator.
 * Extended description: simply rejoins the raw typeParts tokens into a
 * C parameter declaration string. This matches the C generator's "passthrough"
 * behavior — the C output uses raw GL types verbatim.
 */
class DefaultCTypeMapper : public TypeMapper {
public:
    std::string mapParamDecl(const model::CommandParam &param,
                             const model::Command &command) const override;
    std::string mapReturnType(const model::Command &command) const override;
    std::string mapBaseType(std::string_view glType) const override;
};

} // namespace glaze::codegen
