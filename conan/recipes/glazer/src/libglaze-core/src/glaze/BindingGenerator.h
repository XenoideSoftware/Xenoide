#pragma once

#include "glaze/GenerateOptions.h"

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace glaze {

/**
 * @brief High-level facade that ties producer + generators + file-writing
 *        into a single call. This is the entry point consumers import.
 * Extended description: loads the registry via RegistryLoader, instantiates
 * the requested generators for each (api, language) pair, runs them, and
 * writes the returned relative paths under options.outputDir. Returns a
 * GenerateResult listing every file written. All exceptions from the
 * producer and generators (glaze::producer::ParseError,
 * std::invalid_argument, std::filesystem::filesystem_error) propagate up
 * to the caller.
 */
class BindingGenerator {
public:
    BindingGenerator();
    ~BindingGenerator();
    BindingGenerator(const BindingGenerator &) = delete;
    BindingGenerator(BindingGenerator &&) = delete;
    BindingGenerator &operator=(const BindingGenerator &) = delete;
    BindingGenerator &operator=(BindingGenerator &&) = delete;

    /**
     * @brief Generate all requested language bindings in one pass.
     * @param options the configuration for this generation run
     * @throws glaze::producer::ParseError on registry I/O or structural failure
     * @throws std::invalid_argument on unknown api/version or bad extension filters
     * @throws std::filesystem::filesystem_error on output I/O failure
     */
    GenerateResult generate(const GenerateOptions &options) const;

    /**
     * @brief List APIs and their sorted version strings from a registry file.
     * @param registryPath filesystem path to the registry file
     */
    std::map<std::string, std::vector<std::string>>
    listApis(const std::filesystem::path &registryPath) const;
};

} // namespace glaze
