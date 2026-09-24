#include "glaze/producer/detail/CommandParser.h"

#include "glaze/producer/detail/XmlHelpers.h"

namespace glaze::producer::detail {

namespace {

//! Extract type tokens from a <proto> or <param> node, following the Python
//! rule: whitespace-split all text nodes and append each <ptype> element as
//! a single whole token. Critically, <name> elements are skipped — they hold
//! the function/parameter identifier, not the type.
std::vector<std::string> tokenizeTypeContext(const pugi::xml_node &node) {
    std::vector<std::string> parts;
    for (const auto &child : node.children()) {
        if (child.type() == pugi::node_pcdata || child.type() == pugi::node_cdata) {
            const std::string raw = child.value();
            std::size_t i = 0;
            while (i < raw.size()) {
                while (i < raw.size() &&
                       std::isspace(static_cast<unsigned char>(raw[i])) != 0) {
                    ++i;
                }
                const std::size_t start = i;
                while (i < raw.size() &&
                       std::isspace(static_cast<unsigned char>(raw[i])) == 0) {
                    ++i;
                }
                if (start < i) {
                    parts.emplace_back(raw.substr(start, i - start));
                }
            }
        } else if (child.type() == pugi::node_element) {
            const std::string tag = child.name();
            if (tag == "ptype") {
                std::string text = child.text().get();
                std::size_t a = 0;
                std::size_t b = text.size();
                while (a < b && std::isspace(static_cast<unsigned char>(text[a])) != 0) {
                    ++a;
                }
                while (b > a && std::isspace(static_cast<unsigned char>(text[b - 1])) != 0) {
                    --b;
                }
                if (a < b) {
                    parts.emplace_back(text.substr(a, b - a));
                }
            }
        }
    }
    return parts;
}

//! Build a TypeDecl + raw return-type string from a <proto> node.
//! Mirrors glaze/parser.py::RegistryParser._parse_return_type.
std::pair<glaze::model::TypeDecl, std::string> parseReturnType(const pugi::xml_node &proto) {
    const auto parts = tokenizeTypeContext(proto);

    std::string joined;
    for (const auto &part : parts) {
        if (part.empty()) {
            continue;
        }
        if (!joined.empty()) {
            joined.push_back(' ');
        }
        joined.append(part);
    }

    glaze::model::TypeDecl decl;
    decl.name = "void";
    decl.isConst = false;
    decl.isPointer = false;

    // Mirror Python: is_const = "const" in type_parts, is_pointer = "*" or
    // "**" in type_parts, name = first token that is not const/*/**/"void",
    // defaulting to "void" if none match.
    for (const auto &part : parts) {
        if (part == "const") {
            decl.isConst = true;
        } else if (part == "*" || part == "**") {
            decl.isPointer = true;
        }
    }
    for (const auto &part : parts) {
        if (!part.empty() && part != "const" && part != "*" && part != "**" && part != "void") {
            decl.name = part;
            break;
        }
    }

    return {decl, joined};
}

glaze::model::CommandParam parseParam(const pugi::xml_node &paramNode) {
    glaze::model::CommandParam param;
    param.group = optionalAttr(paramNode, "group");
    param.classStr = optionalAttr(paramNode, "class");
    param.len = optionalAttr(paramNode, "len");

    for (const auto &child : paramNode.children()) {
        if (child.type() == pugi::node_pcdata || child.type() == pugi::node_cdata) {
            // Append each whitespace-separated token from any text section.
            // We can reuse tokenizeMixedContent by wrapping the raw text, but
            // it's simpler to append inline here.
            const std::string raw = child.value();
            std::size_t i = 0;
            while (i < raw.size()) {
                while (i < raw.size() &&
                       std::isspace(static_cast<unsigned char>(raw[i])) != 0) {
                    ++i;
                }
                const std::size_t start = i;
                while (i < raw.size() &&
                       std::isspace(static_cast<unsigned char>(raw[i])) == 0) {
                    ++i;
                }
                if (start < i) {
                    param.typeParts.emplace_back(raw.substr(start, i - start));
                }
            }
        } else if (child.type() == pugi::node_element) {
            const std::string tagName = child.name();
            if (tagName == "ptype") {
                std::string text = child.text().get();
                // trim
                std::size_t a = 0;
                std::size_t b = text.size();
                while (a < b && std::isspace(static_cast<unsigned char>(text[a])) != 0) {
                    ++a;
                }
                while (b > a && std::isspace(static_cast<unsigned char>(text[b - 1])) != 0) {
                    --b;
                }
                param.typeParts.emplace_back(text.substr(a, b - a));
            } else if (tagName == "name") {
                std::string text = child.text().get();
                std::size_t a = 0;
                std::size_t b = text.size();
                while (a < b && std::isspace(static_cast<unsigned char>(text[a])) != 0) {
                    ++a;
                }
                while (b > a && std::isspace(static_cast<unsigned char>(text[b - 1])) != 0) {
                    --b;
                }
                param.name = text.substr(a, b - a);
            }
        }
    }
    return param;
}

glaze::model::Command parseCommand(const pugi::xml_node &commandNode,
                                       const std::string &namespaceName) {
    glaze::model::Command command;
    command.namespaceName = namespaceName;

    const auto proto = commandNode.child("proto");
    if (const auto nameOpt = firstChildName(proto); nameOpt) {
        command.name = *nameOpt;
    }

    auto [decl, returnStr] = parseReturnType(proto);
    command.returnType = std::move(decl);
    command.returnTypeStr = std::move(returnStr);

    for (const auto &child : commandNode.children("param")) {
        command.params.push_back(parseParam(child));
    }

    return command;
}

} // namespace

std::vector<glaze::model::Command> parseCommands(const pugi::xml_node &registry) {
    std::vector<glaze::model::Command> result;
    for (const auto &commandsNode : registry.children("commands")) {
        std::string ns = "GL";
        if (const auto attr = optionalAttr(commandsNode, "namespace"); attr) {
            ns = *attr;
        }
        for (const auto &child : commandsNode.children("command")) {
            result.push_back(parseCommand(child, ns));
        }
    }
    return result;
}

} // namespace glaze::producer::detail
