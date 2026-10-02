#pragma once

#include "glaze/Language.h"
#include "glaze/codegen/NameTransform.h"
#include "glaze/codegen/TypeMapper.h"

#include <filesystem>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace glaze {

/**
 * @brief One (api, version) tuple requested from the generator.
 */
struct ApiRequest {
    //! api name (may be virtual, e.g. "gl_compat")
    std::string api;
    //! MAJOR.MINOR version string (e.g. "3.3")
    std::string version;
};

/**
 * @brief Aggregated options for a single BindingGenerator::generate() call.
 * Extended description: a value struct, aggregate-initializable, that the
 * facade consumes to drive producer + generators + output writing in one
 * call. Extension filters apply to every (api, language) combination.
 */
struct GenerateOptions {
    //! Path to gl.xml. Required.
    std::filesystem::path registryPath;
    //! One or more (api, version) pairs to generate. Required.
    std::vector<ApiRequest> apis;
    //! One or more target languages. Required.
    std::vector<Language> languages;
    //! Directory where the relative paths returned by the generators are
    //! written. Created if missing.
    std::filesystem::path outputDir;
    //! Vendor prefixes for opt-in extension emission (e.g. {"ARB", "KHR"}).
    std::set<std::string> extensionVendors;
    //! Full extension names for opt-in emission (e.g. {"GL_KHR_debug"}).
    std::set<std::string> extensionNames;
    //! Optional custom name transform. Null -> DefaultNameTransform.
    std::shared_ptr<const codegen::NameTransform> nameTransform;
    //! Optional custom type mapper. Null -> per-language default.
    std::shared_ptr<const codegen::TypeMapper> typeMapper;
    //! Optional path to a checked-out OpenGL-Refpages tree. Empty means no
    //! documentation is attached to the generated symbols.
    std::filesystem::path refpagesDir;
};

/**
 * @brief Result of one BindingGenerator::generate() call.
 * Extended description: lists the absolute paths of every file the facade
 * wrote to disk. No mutation happens after generate() returns.
 */
struct GenerateResult {
    std::vector<std::filesystem::path> writtenFiles;
};

} // namespace glaze
