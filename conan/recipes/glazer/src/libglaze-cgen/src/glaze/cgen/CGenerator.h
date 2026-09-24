#pragma once

#include "glaze/codegen/NameTransform.h"
#include "glaze/codegen/TypeMapper.h"
#include "glaze/docparser/DocIndex.h"
#include "glaze/model/Registry.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace glaze::cgen {

/**
 * @brief C binding generator. Mirrors glaze/generators/c_generator.py.
 * Extended description: consumes an in-memory Registry and emits two files
 * per invocation — include/glaze/{api}.h with type definitions, enum macros,
 * function pointer typedefs and extern declarations, and src/{api}.c with
 * the function pointer variable definitions and the glazeLoadFunctions()
 * implementation. Takes optional NameTransform and TypeMapper policies so
 * callers can retarget identifier and type rendering without touching the
 * generator internals.
 */
class CGenerator {
public:
    /**
     * @brief Construct a generator bound to a registry.
     * @param registry the Registry to draw from — must outlive the generator
     * @param nameTransform optional policy for name rewrites; null -> default
     * @param typeMapper optional policy for type rendering; null -> DefaultCTypeMapper
     */
    explicit CGenerator(const model::Registry &registry,
                        std::shared_ptr<const codegen::NameTransform> nameTransform = {},
                        std::shared_ptr<const codegen::TypeMapper> typeMapper = {});

    /**
     * @brief Generate the C binding files for a single api/version pair.
     * @param api registry api name (may be virtual, e.g. "gl_compat")
     * @param version MAJOR.MINOR string
     * @param extensionVendors optional vendor prefixes for opt-in extension emission
     * @param extensionNames optional full extension names for opt-in emission
     * @return {relative path -> file content}
     * @throws std::invalid_argument when api/version is unknown
     */
    std::map<std::string, std::string> generate(
        const std::string &api,
        const std::string &version,
        const std::set<std::string> &extensionVendors = {},
        const std::set<std::string> &extensionNames = {},
        const docparser::DocIndex &docs = {}) const;

    /**
     * @brief Validate that the requested extension filters exist for the api.
     * @return a list of human-readable error messages; empty if all valid
     */
    std::vector<std::string>
    validateExtensionFilters(const std::string &api,
                             const std::set<std::string> &extensionVendors,
                             const std::set<std::string> &extensionNames) const;

private:
    const model::Registry &registry_;
    std::shared_ptr<const codegen::NameTransform> nameTransform_;
    std::shared_ptr<const codegen::TypeMapper> typeMapper_;
};

} // namespace glaze::cgen
