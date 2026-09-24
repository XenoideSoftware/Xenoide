#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace glaze::model {

/**
 * @brief Detect whether a string follows the "TitleLower" capitalization shape.
 * Mirrors glaze/utils/string_utils.py::is_capitalized.
 * Extended description: the Python test walks a small state machine — initial
 * upper-case letter(s), optional lower-case tail, no trailing upper-case. An
 * empty string returns false. Used by splitCapitalized to decide where one
 * CamelCase segment ends and the next begins.
 */
bool isCapitalized(std::string_view value) noexcept;

/**
 * @brief Split a CamelCase string into its component words.
 * Mirrors glaze/utils/string_utils.py::split_capitalized.
 * @param value the string to split (e.g. "GLenum" → {"GL", "enum"})
 * @return one entry per segment, preserving original casing
 */
std::vector<std::string> splitCapitalized(std::string_view value);

/**
 * @brief Convert a space-delimited lower-case phrase into PascalCase.
 * Mirrors glaze/utils/string_utils.py::camel_case.
 * @param value the phrase (e.g. "program pipeline" → "ProgramPipeline")
 */
std::string camelCase(std::string_view value);

/**
 * @brief Encode a "MAJOR.MINOR" GL version string as MAJOR * 10 + MINOR.
 * Mirrors glaze/utils/string_utils.py::version_to_int.
 * @throws std::invalid_argument when the input isn't two integer parts
 */
int versionToInt(std::string_view version);

} // namespace glaze::model
