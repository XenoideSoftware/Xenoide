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

namespace glaze::cppgen {

/**
 * @brief C++ binding generator. Mirrors glaze/generators/cpp_generator.py.
 * Extended description: consumes an in-memory Registry and emits three files
 * per invocation — include/glaze/{api}.hpp (strong handles, enum classes,
 * functors, DSA classes, RAII Traits), include/glaze/{api}_handle.hpp (legacy
 * handle wrapper classes), and include/glaze/raii.hpp (a verbatim copy of
 * the hand-written RAII smart-pointer header shipped with glaze). Takes
 * optional NameTransform and TypeMapper policies so callers can retarget
 * identifier and type rendering without touching the generator internals.
 */
class CppGenerator {
public:
    explicit CppGenerator(const model::Registry &registry,
                          std::shared_ptr<const codegen::NameTransform> nameTransform = {},
                          std::shared_ptr<const codegen::TypeMapper> typeMapper = {});

    std::map<std::string, std::string>
    generate(const std::string &api,
             const std::string &version,
             const std::set<std::string> &extensionVendors = {},
             const std::set<std::string> &extensionNames = {},
             const docparser::DocIndex &docs = {}) const;

    std::vector<std::string>
    validateExtensionFilters(const std::string &api,
                             const std::set<std::string> &extensionVendors,
                             const std::set<std::string> &extensionNames) const;

private:
    const model::Registry &registry_;
    std::shared_ptr<const codegen::NameTransform> nameTransform_;
    std::shared_ptr<const codegen::TypeMapper> typeMapper_;
};

} // namespace glaze::cppgen
