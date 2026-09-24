
#pragma once

#include "cgltf.h"
#include "xe/graphics/gl/RendererGL.h"
#include <string>

inline std::string to_string(cgltf_result result) {
    switch (result) {
    case cgltf_result_data_too_short:
        return "cgltf_result_data_too_short";
    case cgltf_result_unknown_format:
        return "cgltf_result_unknown_format";
    case cgltf_result_invalid_json:
        return "cgltf_result_invalid_json";
    case cgltf_result_invalid_gltf:
        return "cgltf_result_invalid_gltf";
    case cgltf_result_invalid_options:
        return "cgltf_result_invalid_options";
    case cgltf_result_file_not_found:
        return "cgltf_result_file_not_found";
    case cgltf_result_io_error:
        return "cgltf_result_io_error";
    case cgltf_result_out_of_memory:
        return "cgltf_result_out_of_memory";
    case cgltf_result_legacy_gltf:
        return "cgltf_result_legacy_gltf";
    case cgltf_result_success:
        return "cgltf_result_success";
    default:
        return "unknown enum" + std::to_string(result);
    }
}

inline std::string to_string(cgltf_primitive_type type) {
    switch (type) {
    case cgltf_primitive_type_points:
        return "cgltf_primitive_type_points";
    case cgltf_primitive_type_lines:
        return "cgltf_primitive_type_lines";
    case cgltf_primitive_type_line_loop:
        return "cgltf_primitive_type_line_loop";
    case cgltf_primitive_type_line_strip:
        return "cgltf_primitive_type_line_strip";
    case cgltf_primitive_type_triangles:
        return "cgltf_primitive_type_triangles";
    case cgltf_primitive_type_triangle_strip:
        return "cgltf_primitive_type_triangle_strip";
    case cgltf_primitive_type_triangle_fan:
        return "cgltf_primitive_type_triangle_fan";
    default:
        return "<unknown primitive type>";
    }
}

inline std::ostream &operator<<(std::ostream &os, cgltf_primitive_type type) {
    return os << to_string(type);
}

inline std::string to_string(cgltf_attribute_type type) {
    switch (type) {
    case cgltf_attribute_type_invalid:
        return "cgltf_attribute_type_invalid";
    case cgltf_attribute_type_position:
        return "cgltf_attribute_type_position";
    case cgltf_attribute_type_normal:
        return "cgltf_attribute_type_normal";
    case cgltf_attribute_type_tangent:
        return "cgltf_attribute_type_tangent";
    case cgltf_attribute_type_texcoord:
        return "cgltf_attribute_type_texcoord";
    case cgltf_attribute_type_color:
        return "cgltf_attribute_type_color";
    case cgltf_attribute_type_joints:
        return "cgltf_attribute_type_joints";
    case cgltf_attribute_type_weights:
        return "cgltf_attribute_type_weights";
    default:
        return "<unknown primitive type>";
    }
}

inline std::ostream &operator<<(std::ostream &os, cgltf_attribute_type type) {
    return os << to_string(type);
}

inline std::string to_string(cgltf_type type) {
    switch (type) {
    case cgltf_type_invalid:
        return "cgltf_type_invalid";
    case cgltf_type_scalar:
        return "cgltf_type_scalar";
    case cgltf_type_vec2:
        return "cgltf_type_vec2";
    case cgltf_type_vec3:
        return "cgltf_type_vec3";
    case cgltf_type_vec4:
        return "cgltf_type_vec4";
    case cgltf_type_mat2:
        return "cgltf_type_mat2";
    case cgltf_type_mat3:
        return "cgltf_type_mat3";
    case cgltf_type_mat4:
        return "cgltf_type_mat4";
    default:
        return "<unknown type>";
    }
}

inline std::ostream &operator<<(std::ostream &os, cgltf_type type) {
    return os << to_string(type);
}

inline std::string to_string(cgltf_component_type type) {
    switch (type) {
    case cgltf_component_type_invalid:
        return "cgltf_component_type_invalid";
    case cgltf_component_type_r_8:
        return "cgltf_component_type_r_8";
    case cgltf_component_type_r_8u:
        return "cgltf_component_type_r_8u";
    case cgltf_component_type_r_16:
        return "cgltf_component_type_r_16";
    case cgltf_component_type_r_16u:
        return "cgltf_component_type_r_16u";
    case cgltf_component_type_r_32u:
        return "cgltf_component_type_r_32u";
    case cgltf_component_type_r_32f:
        return "cgltf_component_type_r_32f";
    default:
        return "<unknown component type>";
    }
}

inline std::ostream &operator<<(std::ostream &os, cgltf_component_type type) {
    return os << to_string(type);
}

inline std::string to_string(cgltf_interpolation_type type) {
    switch (type) {
    case cgltf_interpolation_type_linear:
        return "cgltf_interpolation_type_linear";
    case cgltf_interpolation_type_step:
        return "cgltf_interpolation_type_step";
    case cgltf_interpolation_type_cubic_spline:
        return "cgltf_interpolation_type_cubic_spline";
    default:
        return "<unknown internpolation_type>";
    }
}

inline std::ostream &operator<<(std::ostream &os, cgltf_interpolation_type type) {
    return os << to_string(type);
}

inline std::string to_string(cgltf_animation_path_type type) {
    switch (type) {
    case cgltf_animation_path_type_invalid:
        return "cgltf_animation_path_type_invalid";
    case cgltf_animation_path_type_translation:
        return "cgltf_animation_path_type_translation";
    case cgltf_animation_path_type_rotation:
        return "cgltf_animation_path_type_rotation";
    case cgltf_animation_path_type_scale:
        return "cgltf_animation_path_type_scale";
    case cgltf_animation_path_type_weights:
        return "cgltf_animation_path_type_weights";
    default:
        return "<unknown internpolation_type>";
    }
}

inline std::ostream &operator<<(std::ostream &os, cgltf_animation_path_type type) {
    return os << to_string(type);
}

inline std::string evaluate_name(const char *name) {
    if (name) {
        return name;
    }

    return "";
}

template <typename T> std::string evaluate_ptr(const std::string &msg, const T *ptr) {
    return msg + (ptr ? "true" : "false");
}

inline std::string evaluate_bool(const cgltf_bool value) {
    return (value ? "true" : "false");
}

inline std::string sanitizeString(const char *value, const std::string &defaultValue = "<noname>") {
    return value ? value : defaultValue;
}

inline bool toBool(const cgltf_bool value) {
    return value ? true : false;
}

template <typename T> bool toBool(const T *value) {
    return value ? true : false;
}

template <typename IntegerLike> void *addPointerOffset(void *ptr, IntegerLike offset) {
    static_assert(std::is_integral_v<IntegerLike>);

    return static_cast<std::byte *>(ptr) + offset;
}

inline std::optional<std::string> to_string(const char *str) {
    if (str) {
        return str;
    }

    return {};
}

/**
 * Maps a cgltf_primitive_type enum to a GL primitive enum.
 * Should be future-proof, as we are assuming that the cgltf_primitive_type enum values
 * could change in the future
 */
[[nodiscard]]
constexpr GLenum mapToPrimitive(const cgltf_primitive_type type) {
    switch (type) {
    case cgltf_primitive_type_points:
        return GL_POINTS;
    case cgltf_primitive_type_lines:
        return GL_LINES;
    case cgltf_primitive_type_line_loop:
        return GL_LINE_LOOP;
    case cgltf_primitive_type_line_strip:
        return GL_LINE_STRIP;
    case cgltf_primitive_type_triangles:
        return GL_TRIANGLES;
    case cgltf_primitive_type_triangle_strip:
        return GL_TRIANGLE_STRIP;
    case cgltf_primitive_type_triangle_fan:
        return GL_TRIANGLE_FAN;
    default:
        return GL_INVALID_ENUM;
    }
}

[[nodiscard]]
constexpr std::optional<xe::gl::AttributeType> mapToAttributeDataType(const cgltf_component_type type) {
    switch (type) {
        // case cgltf_component_type_invalid: return "cgltf_component_type_invalid";
        // case cgltf_component_type_r_8: return "cgltf_component_type_r_8";
        // case cgltf_component_type_r_8u: return "cgltf_component_type_r_8u";
        // case cgltf_component_type_r_16: return "cgltf_component_type_r_16";
        // case cgltf_component_type_r_16u: return "cgltf_component_type_r_16u";

    case cgltf_component_type_r_8u:
        return xe::gl::AttributeType::UnsignedByte;

    case cgltf_component_type_r_16u:
        return xe::gl::AttributeType::UnsignedShort;

    case cgltf_component_type_r_32u:
        return xe::gl::AttributeType::UnsignedInt;

    case cgltf_component_type_r_32f:
        return xe::gl::AttributeType::Float;

    default:
        return std::nullopt;
    }
}

[[nodiscard]]
constexpr std::optional<GLenum> mapToGLDataType(const cgltf_component_type type) {
    switch (type) {
    case cgltf_component_type_r_8:
        return GL_BYTE;
    case cgltf_component_type_r_8u:
        return GL_UNSIGNED_BYTE;
    case cgltf_component_type_r_16:
        return GL_SHORT;
    case cgltf_component_type_r_16u:
        return GL_UNSIGNED_SHORT;
    case cgltf_component_type_r_32u:
        return GL_UNSIGNED_INT;
    case cgltf_component_type_r_32f:
        return GL_FLOAT;
    default:
        return std::nullopt;
    }
}

[[nodiscard]]
constexpr std::optional<xe::gl::AttributeDim> mapToAttribDim(const cgltf_type type) {
    switch (type) {
    case cgltf_type_scalar:
        return xe::gl::AttributeDim::_1;
    case cgltf_type_vec2:
        return xe::gl::AttributeDim::_2;
    case cgltf_type_vec3:
        return xe::gl::AttributeDim::_3;
    case cgltf_type_vec4:
        return xe::gl::AttributeDim::_4;
    default:
        return std::nullopt;
    }
}
