#pragma once

#include "glaze/GenerateOptions.h"

#include <optional>
#include <string>
#include <vector>

namespace glaze::cli {

/**
 * @brief Parse CLI argv for the "generate" subcommand into GenerateOptions.
 * Extended description: mirrors the Python glaze_cli.py argument parser at a
 * high level. Flags:
 *   --registry <path>                  (default: "./gl.xml")
 *   --api <name>:<version>             (repeatable)
 *   --lang <c|cpp>                     (repeatable)
 *   --output-dir <dir>                 (default: ".")
 *   --extension-vendors <csv>          (e.g. "ARB,KHR")
 *   --extensions <csv>                 (e.g. "GL_KHR_debug")
 *
 * The --api flag uses colon form (e.g. "gl:3.3") because cxxopts cannot
 * cleanly handle two-token flags. The existing glaze Conan package already
 * uses colon form in its apis option so this matches precedent.
 *
 * @param argc number of arguments, as passed to main
 * @param argv argument vector, as passed to main
 * @return populated GenerateOptions, or nullopt if argv is invalid
 * @throws std::invalid_argument when an argument has a parseable-but-bad shape
 */
std::optional<GenerateOptions> parseGenerate(int argc, const char *const *argv);

/**
 * @brief Parse CLI argv for the "list-apis" subcommand.
 * @return the path to the registry file; defaults to "./gl.xml".
 */
std::string parseListApisRegistryPath(int argc, const char *const *argv);

} // namespace glaze::cli
