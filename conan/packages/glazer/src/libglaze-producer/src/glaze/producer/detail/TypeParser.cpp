#include "glaze/producer/detail/TypeParser.h"

#include "glaze/producer/detail/XmlHelpers.h"

namespace glaze::producer::detail {

namespace {

//! Mirrors _type_name_from_child: looks for a nested <name> element and
//! returns its trimmed text content if present.
std::optional<std::string> typeNameFromNestedElement(const pugi::xml_node &node) {
    return firstChildName(node);
}

std::optional<glaze::model::Type> parseSingleType(const pugi::xml_node &typeNode) {
    auto name = typeNameFromNestedElement(typeNode);
    if (!name) {
        name = optionalAttr(typeNode, "name");
    }
    if (!name) {
        return std::nullopt;
    }

    glaze::model::Type result;
    result.name = *name;
    result.cDefinition = collectText(typeNode);
    result.requiresName = optionalAttr(typeNode, "requires");
    result.comment = optionalAttr(typeNode, "comment");
    return result;
}

} // namespace

std::vector<glaze::model::Type> parseTypes(const pugi::xml_node &registry) {
    std::vector<glaze::model::Type> result;
    const auto typesNode = registry.child("types");
    if (!typesNode) {
        return result;
    }
    for (const auto &child : typesNode.children("type")) {
        if (auto parsed = parseSingleType(child); parsed) {
            result.push_back(std::move(*parsed));
        }
    }
    return result;
}

} // namespace glaze::producer::detail
