#include "glaze/cgen/CGenerator.h"

#include "glaze/cgen/detail/CHeaderContext.h"
#include "glaze/cgen/detail/CSourceContext.h"
#include "glaze/codegen/ExtensionFilter.h"

#include <inja/inja.hpp>

#include <memory>
#include <string>

namespace glaze::cgen::detail::templates {

// Defined by build-time embedded sources generated via
// cmake/EmbedTextFile.cmake.
extern const char *gl_h_inja;
extern const char *gl_c_inja;

} // namespace glaze::cgen::detail::templates

namespace glaze::cgen {

namespace {

//! Build a fresh inja environment with our standard options. Kept local so
//! callers never see inja types.
inja::Environment makeEnvironment() {
    inja::Environment env;
    env.set_trim_blocks(true);
    env.set_lstrip_blocks(true);
    return env;
}

} // namespace

CGenerator::CGenerator(const model::Registry &registry,
                       std::shared_ptr<const codegen::NameTransform> nameTransform,
                       std::shared_ptr<const codegen::TypeMapper> typeMapper)
    : registry_(registry),
      nameTransform_(nameTransform ? std::move(nameTransform)
                                   : std::make_shared<codegen::DefaultNameTransform>()),
      typeMapper_(typeMapper ? std::move(typeMapper)
                             : std::make_shared<codegen::DefaultCTypeMapper>()) {}

std::map<std::string, std::string>
CGenerator::generate(const std::string &api,
                     const std::string &version,
                     const std::set<std::string> &extensionVendors,
                     const std::set<std::string> &extensionNames,
                     const docparser::DocIndex &docs) const {
    codegen::checkApiVersion(registry_, api, version);

    const auto features = registry_.collectFeatures(api, version);
    const auto emissions = codegen::collectExtensionEmissions(
        registry_, api, version, extensionVendors, extensionNames);

    auto env = makeEnvironment();

    const auto headerCtx = detail::buildHeaderContext(
        registry_, features, emissions, api, version, *typeMapper_, docs);
    const auto sourceCtx =
        detail::buildSourceContext(registry_, features, emissions, api, version, *typeMapper_);

    std::map<std::string, std::string> result;
    result["include/glaze/" + api + ".h"] =
        env.render(detail::templates::gl_h_inja, headerCtx);
    result["src/" + api + ".c"] = env.render(detail::templates::gl_c_inja, sourceCtx);
    return result;
}

std::vector<std::string>
CGenerator::validateExtensionFilters(const std::string &api,
                                     const std::set<std::string> &extensionVendors,
                                     const std::set<std::string> &extensionNames) const {
    return codegen::validateExtensionFilters(registry_, api, extensionVendors, extensionNames);
}

} // namespace glaze::cgen
