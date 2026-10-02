#pragma once

#include "glaze/model/Command.h"
#include "glaze/model/CommandParam.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace glaze::cppgen::detail {

/**
 * @brief True if a parameter's base type is GLuint — a potential object handle.
 * Mirrors cpp_generator.py::_is_uint_handle.
 */
bool isUintHandle(const model::CommandParam &param) noexcept;

/**
 * @brief True if a command returns a C string (const GLubyte* / const GLchar*).
 * Mirrors cpp_generator.py::_is_string_return_command.
 */
bool isStringReturnCommand(const model::Command &command) noexcept;

/**
 * @brief Detect a multi-object creation pattern: (GLsizei n, GLuint *out).
 * Mirrors cpp_generator.py::_find_object_creation_params.
 * @return true and populates countOut/outputOut when the pattern matches
 */
bool findObjectCreationParams(const model::Command &command,
                              const model::CommandParam *&countOut,
                              const model::CommandParam *&outputOut) noexcept;

/**
 * @brief Detect a multi-object deletion pattern: (GLsizei n, const GLuint *in).
 * Mirrors cpp_generator.py::_find_object_deletion_params.
 */
bool findObjectDeletionParams(const model::Command &command,
                              const model::CommandParam *&countOut,
                              const model::CommandParam *&inputOut) noexcept;

/**
 * @brief True if a GLint "location" parameter is a uniform location.
 * Mirrors cpp_generator.py::_is_uniform_location_param.
 */
bool isUniformLocationParam(const model::Command &command,
                            const model::CommandParam &param) noexcept;

/**
 * @brief True if a GLuint "index" parameter is a vertex-attrib location.
 * Mirrors cpp_generator.py::_is_attrib_location_param.
 */
bool isAttribLocationParam(const model::Command &command,
                           const model::CommandParam &param) noexcept;

/**
 * @brief True if the command's name appears in the known set of commands that
 *        return a uniform location (e.g. glGetUniformLocation).
 */
bool isUniformLocationReturn(const model::Command &command) noexcept;

/**
 * @brief True if the command's name appears in the known set of commands that
 *        return an attribute location (e.g. glGetAttribLocation).
 */
bool isAttribLocationReturn(const model::Command &command) noexcept;

/**
 * @brief Set of scalar pointer types accepted as the trailing output of a
 *        per-object "getter" query. Mirrors cpp_generator.py::_SCALAR_QUERY_TYPES.
 */
const std::unordered_set<std::string> &scalarQueryTypes();

/**
 * @brief Command → Get*iv functor name map used to synthesize zero-argument
 *        info-log queries. Mirrors cpp_generator.py::_INFOLOG_SELF_QUERY_MAP.
 */
const std::unordered_map<std::string, std::string> &infoLogSelfQueryMap();

/**
 * @brief Detect the trailing scalar output pointer on a per-object getter query.
 * Mirrors cpp_generator.py::_find_scalar_query_param (cpp_generator.py:181).
 *
 * Matches commands of the form `glGet<Obj>...v(<handle>, ..., T *out)` where
 * the first parameter is a named GL object handle and the last is the only
 * non-const output pointer whose base type is in scalarQueryTypes().
 *
 * @return pointer to the trailing out-parameter, or nullptr when no match.
 */
const model::CommandParam *findScalarQueryParam(const model::Command &command) noexcept;

/**
 * @brief Find the GLchar* non-const output param whose len= references another
 *        param by name. Mirrors cpp_generator.py::_find_string_output_param.
 */
const model::CommandParam *findStringOutputParam(const model::Command &command) noexcept;

/**
 * @brief Find the GLsizei* non-const output length sink (len="1") param.
 * Mirrors cpp_generator.py::_find_length_param.
 */
const model::CommandParam *findLengthParam(const model::Command &command) noexcept;

/**
 * @brief Result of a successful data-upload pattern match.
 */
struct DataUploadParams {
    //! The const void* data pointer whose len="<size_param>" drives the upload.
    const model::CommandParam *data;
    //! The matching GLsizei/GLsizeiptr size parameter referenced by data.len.
    const model::CommandParam *size;
};

/**
 * @brief Detect an ArrayView-style (size, data) upload pair.
 * Mirrors cpp_generator.py::_find_data_upload_params.
 */
std::optional<DataUploadParams> findDataUploadParams(const model::Command &command) noexcept;

/**
 * @brief True if the command is one of the two known info-log queries
 *        (glGetShaderInfoLog / glGetProgramInfoLog).
 */
bool isInfoLogCommand(const model::Command &command) noexcept;

} // namespace glaze::cppgen::detail
