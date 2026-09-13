#pragma once

#include <set>
#include <string>
#include <string_view>

namespace glaze::codegen {

/**
 * @brief Customization point for generated symbol names.
 * Extended description: every place the Glaze code generators need to turn a
 * GL identifier (GL command, object class, enum constant, enum group) into
 * a target-language identifier goes through this interface. The default
 * implementation mirrors the hard-coded behavior from
 * glaze/generators/cpp_generator.py. Subclass and override any method to
 * retarget a single transform without touching the generator code.
 */
class NameTransform {
public:
    NameTransform() = default;
    NameTransform(const NameTransform &) = default;
    NameTransform(NameTransform &&) = default;
    NameTransform &operator=(const NameTransform &) = default;
    NameTransform &operator=(NameTransform &&) = default;
    virtual ~NameTransform() = default;

    /**
     * @brief Transform a raw GL command name into a C++ function/functor name.
     * Default mirrors glaze/generators/cpp_generator.py::_convert_function_name:
     * drops the "gl" prefix and lowercases the first letter of the remainder.
     * Example: "glClear" -> "clear", "glBindBuffer" -> "bindBuffer".
     */
    virtual std::string transformCommandName(std::string_view glName) const;

    /**
     * @brief Transform a GL object class string into a C++ handle type name.
     * Default mirrors glaze/generators/cpp_generator.py::_to_handle_name:
     * title-cases each space-separated word.
     * Example: "buffer" -> "Buffer", "program pipeline" -> "ProgramPipeline".
     */
    virtual std::string transformHandleTypeName(std::string_view classStr) const;

    /**
     * @brief Transform a GL_* enum constant name into a C++ enum class entry
     *        identifier, given the target C++ enum class name.
     * Default mirrors glaze/generators/cpp_generator.py::_EnumIdentifierConverter:
     * strips the leading "GL_" prefix, title-cases the remaining underscore
     * separated words, and prepends "e".
     * Example: group="ClearBufferMask", constant="GL_COLOR_BUFFER_BIT" ->
     *          "eColorBufferBit".
     */
    virtual std::string transformEnumEntry(std::string_view groupName,
                                           std::string_view constantName) const;

    /**
     * @brief Transform an enum group name into its C++ enum class name.
     * Default mirrors glaze/generators/cpp_generator.py::_build_group_rename:
     * strips a known vendor suffix ("ARB", "KHR", "EXT", "NV", ...) when
     * doing so would not collide with another name in allGroups.
     * Example: "BufferTargetARB" -> "BufferTarget" (when no collision).
     */
    virtual std::string transformEnumGroupName(std::string_view group,
                                               const std::set<std::string> &allGroups) const;
};

/**
 * @brief Default NameTransform implementation matching Python behavior.
 * Extended description: this is the implementation the facade instantiates
 * when the caller does not supply a custom NameTransform. It holds no state
 * and is safe to share across generators.
 */
class DefaultNameTransform : public NameTransform {
public:
    std::string transformCommandName(std::string_view glName) const override;
    std::string transformHandleTypeName(std::string_view classStr) const override;
    std::string transformEnumEntry(std::string_view groupName,
                                   std::string_view constantName) const override;
    std::string transformEnumGroupName(std::string_view group,
                                       const std::set<std::string> &allGroups) const override;
};

} // namespace glaze::codegen
