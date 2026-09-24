#include "glaze/producer/detail/EnumParser.h"

#include "glaze/producer/detail/XmlHelpers.h"

namespace glaze::producer::detail {

namespace {

glaze::model::Enum parseEnum(const pugi::xml_node &enumNode) {
    glaze::model::Enum result;
    result.name = enumNode.attribute("name").as_string();
    result.value = enumNode.attribute("value").as_string();

    if (const auto groupAttr = optionalAttr(enumNode, "group"); groupAttr) {
        result.groups = splitDelimited(*groupAttr, ',');
    }
    result.alias = optionalAttr(enumNode, "alias");
    result.comment = optionalAttr(enumNode, "comment");
    return result;
}

glaze::model::EnumGroup parseEnumGroup(const pugi::xml_node &enumsNode) {
    glaze::model::EnumGroup group;
    group.namespaceName = enumsNode.attribute("namespace").as_string();
    group.group = optionalAttr(enumsNode, "group");
    group.enumGroupType = optionalAttr(enumsNode, "type");
    group.vendor = optionalAttr(enumsNode, "vendor");
    group.comment = optionalAttr(enumsNode, "comment");

    for (const auto &child : enumsNode.children("enum")) {
        auto e = parseEnum(child);
        const std::string name = e.name;
        group.enums.emplace_back(name, std::move(e));
    }
    return group;
}

} // namespace

std::vector<glaze::model::EnumGroup> parseEnumGroups(const pugi::xml_node &registry) {
    std::vector<glaze::model::EnumGroup> result;
    for (const auto &child : registry.children("enums")) {
        result.push_back(parseEnumGroup(child));
    }
    return result;
}

} // namespace glaze::producer::detail
