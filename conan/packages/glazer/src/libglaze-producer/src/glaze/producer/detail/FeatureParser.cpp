#include "glaze/producer/detail/FeatureParser.h"

#include "glaze/producer/detail/XmlHelpers.h"

#include <string>

namespace glaze::producer::detail {

namespace {

glaze::model::Remove parseRemove(const pugi::xml_node &removeNode) {
    glaze::model::Remove result;
    result.profile = removeNode.attribute("profile").as_string();
    result.comment = optionalAttr(removeNode, "comment");

    for (const auto &child : removeNode.children()) {
        if (child.type() != pugi::node_element) {
            continue;
        }
        const std::string tag = child.name();
        auto childComment = optionalAttr(child, "comment");
        const std::string refName = child.attribute("name").as_string();
        if (tag == "type") {
            result.types.push_back({refName, childComment});
        } else if (tag == "enum") {
            result.enums.push_back({refName, childComment});
        } else if (tag == "command") {
            result.commands.push_back({refName, childComment});
        }
    }
    return result;
}

glaze::model::Feature parseFeature(const pugi::xml_node &featureNode) {
    glaze::model::Feature result;
    result.api = featureNode.attribute("api").as_string();
    result.name = featureNode.attribute("name").as_string();
    result.number = featureNode.attribute("number").as_string();

    for (const auto &child : featureNode.children()) {
        if (child.type() != pugi::node_element) {
            continue;
        }
        const std::string tag = child.name();
        if (tag == "require") {
            result.requireList.push_back(parseRequire(child));
        } else if (tag == "remove") {
            result.removeList.push_back(parseRemove(child));
        }
    }
    return result;
}

} // namespace

glaze::model::Require parseRequire(const pugi::xml_node &requireNode) {
    glaze::model::Require result;
    for (const auto &child : requireNode.children()) {
        if (child.type() != pugi::node_element) {
            continue;
        }
        const std::string tag = child.name();
        auto comment = optionalAttr(child, "comment");
        const std::string refName = child.attribute("name").as_string();
        if (tag == "type") {
            result.types.push_back({refName, comment});
        } else if (tag == "enum") {
            result.enums.push_back({refName, comment});
        } else if (tag == "command") {
            result.commands.push_back({refName, comment});
        }
    }
    return result;
}

std::vector<glaze::model::Feature> parseFeatures(const pugi::xml_node &registry) {
    std::vector<glaze::model::Feature> result;
    for (const auto &child : registry.children("feature")) {
        result.push_back(parseFeature(child));
    }
    return result;
}

} // namespace glaze::producer::detail
