#pragma once

#include <optional>
#include <string>
#include <vector>

namespace glaze::model {

/**
 * @brief Reference to a type by name inside a <require> / <remove> block.
 * Mirrors glaze/model.py::TypeRef.
 */
struct TypeRef {
    //! target type name
    std::string name;
    //! optional comment
    std::optional<std::string> comment;
};

/**
 * @brief Reference to an enum by name inside a <require> / <remove> block.
 * Mirrors glaze/model.py::EnumRef.
 */
struct EnumRef {
    //! target enum constant name
    std::string name;
    //! optional comment
    std::optional<std::string> comment;
};

/**
 * @brief Reference to a command by name inside a <require> / <remove> block.
 * Mirrors glaze/model.py::CommandRef.
 */
struct CommandRef {
    //! target command name
    std::string name;
    //! optional comment
    std::optional<std::string> comment;
};

/**
 * @brief One <require> block inside a feature or extension.
 * Mirrors glaze/model.py::Require.
 */
struct Require {
    //! required types
    std::vector<TypeRef> types;
    //! required enums
    std::vector<EnumRef> enums;
    //! required commands
    std::vector<CommandRef> commands;
};

/**
 * @brief One <remove> block inside a feature.
 * Mirrors glaze/model.py::Remove.
 * Extended description: <remove> elements only appear inside <feature>, not
 * inside <extension>, and always carry a profile attribute. Consolidation
 * skips removals whose profile is listed in ApiProfile::skipRemoveProfiles.
 */
struct Remove {
    //! profile name (e.g. "core", "compatibility")
    std::string profile;
    //! optional comment attribute
    std::optional<std::string> comment;
    //! removed types
    std::vector<TypeRef> types;
    //! removed enums
    std::vector<EnumRef> enums;
    //! removed commands
    std::vector<CommandRef> commands;
};

/**
 * @brief One <feature> element. Mirrors glaze/model.py::Feature.
 * Extended description: a <feature> represents a versioned OpenGL API level
 * (e.g. GL_VERSION_3_3). It contains one or more <require> and <remove>
 * blocks naming the symbols that enter or leave the API at that version.
 */
struct Feature {
    //! API this feature targets (e.g. "gl", "gles2")
    std::string api;
    //! feature name (e.g. "GL_VERSION_3_3")
    std::string name;
    //! version string (e.g. "3.3")
    std::string number;
    //! ordered list of <require> blocks
    std::vector<Require> requireList;
    //! ordered list of <remove> blocks
    std::vector<Remove> removeList;
};

} // namespace glaze::model
