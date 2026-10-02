#include "glaze/producer/RegistryLoader.h"

#include "glaze/producer/ParseError.h"
#include "glaze/producer/detail/CommandParser.h"
#include "glaze/producer/detail/EnumParser.h"
#include "glaze/producer/detail/ExtensionParser.h"
#include "glaze/producer/detail/FeatureParser.h"
#include "glaze/producer/detail/TypeParser.h"

#include <pugixml.hpp>

#include <string>

namespace glaze::producer {

namespace {

glaze::model::Registry buildRegistryFromDocument(const pugi::xml_document &doc) {
    const auto registryNode = doc.child("registry");
    if (!registryNode) {
        throw ParseError("Expected <registry> root node");
    }

    auto types = detail::parseTypes(registryNode);
    auto enumGroups = detail::parseEnumGroups(registryNode);
    auto commands = detail::parseCommands(registryNode);
    auto features = detail::parseFeatures(registryNode);
    auto extensions = detail::parseExtensions(registryNode);

    return glaze::model::Registry{std::move(types), std::move(enumGroups),
                                      std::move(commands), std::move(features),
                                      std::move(extensions)};
}

} // namespace

glaze::model::Registry RegistryLoader::loadFromFile(std::string_view path) const {
    pugi::xml_document doc;
    const std::string pathStr{path};
    const auto result = doc.load_file(pathStr.c_str());
    if (!result) {
        throw ParseError(std::string{"Failed to parse "} + pathStr + ": " + result.description());
    }
    return buildRegistryFromDocument(doc);
}

glaze::model::Registry RegistryLoader::loadFromString(std::string_view xml) const {
    pugi::xml_document doc;
    const auto result = doc.load_buffer(xml.data(), xml.size());
    if (!result) {
        throw ParseError(std::string{"Failed to parse XML: "} + result.description());
    }
    return buildRegistryFromDocument(doc);
}

} // namespace glaze::producer
