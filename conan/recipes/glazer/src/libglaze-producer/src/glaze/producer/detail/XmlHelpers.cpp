#include "glaze/producer/detail/XmlHelpers.h"

#include <cctype>

namespace glaze::producer::detail {

namespace {

void collectTextInto(const pugi::xml_node &node, std::string &out) {
    for (const auto &child : node.children()) {
        if (child.type() == pugi::node_element) {
            collectTextInto(child, out);
        } else if (child.type() == pugi::node_pcdata || child.type() == pugi::node_cdata) {
            out.append(child.value());
        }
    }
}

//! Tokenize a std::string_view on whitespace (space/tab/newline) — matches
//! Python's str.split() default behavior.
void splitWhitespace(std::string_view value, std::vector<std::string> &out) {
    std::size_t i = 0;
    while (i < value.size()) {
        while (i < value.size() &&
               std::isspace(static_cast<unsigned char>(value[i])) != 0) {
            ++i;
        }
        const std::size_t start = i;
        while (i < value.size() &&
               std::isspace(static_cast<unsigned char>(value[i])) == 0) {
            ++i;
        }
        if (start < i) {
            out.emplace_back(value.substr(start, i - start));
        }
    }
}

//! Trim leading/trailing whitespace from a string in place.
std::string trim(std::string_view value) {
    std::size_t start = 0;
    std::size_t end = value.size();
    while (start < end && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
        ++start;
    }
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }
    return std::string{value.substr(start, end - start)};
}

} // namespace

std::string collectText(const pugi::xml_node &node) {
    std::string out;
    collectTextInto(node, out);
    return out;
}

std::vector<std::string> tokenizeMixedContent(const pugi::xml_node &node) {
    std::vector<std::string> parts;
    for (const auto &child : node.children()) {
        if (child.type() == pugi::node_pcdata || child.type() == pugi::node_cdata) {
            splitWhitespace(child.value(), parts);
        } else if (child.type() == pugi::node_element) {
            // <ptype> is the only element we care about inside proto/param;
            // its text content is appended as a single whole token to match
            // Python's `type_parts.append(child.firstChild.data.strip())`.
            const std::string inner = trim(collectText(child));
            if (!inner.empty()) {
                parts.push_back(inner);
            }
        }
    }
    return parts;
}

std::optional<std::string> firstChildName(const pugi::xml_node &node) {
    const auto nameNode = node.child("name");
    if (!nameNode) {
        return std::nullopt;
    }
    const std::string text = trim(collectText(nameNode));
    if (text.empty()) {
        return std::nullopt;
    }
    return text;
}

std::optional<std::string> optionalAttr(const pugi::xml_node &node, const char *name) {
    const auto attr = node.attribute(name);
    if (!attr) {
        return std::nullopt;
    }
    std::string value = attr.value();
    if (value.empty()) {
        return std::nullopt;
    }
    return value;
}

std::vector<std::string> splitDelimited(std::string_view value, char delim) {
    std::vector<std::string> out;
    std::size_t start = 0;
    for (std::size_t i = 0; i <= value.size(); ++i) {
        if (i == value.size() || value[i] == delim) {
            if (i > start) {
                out.emplace_back(trim(value.substr(start, i - start)));
                if (out.back().empty()) {
                    out.pop_back();
                }
            }
            start = i + 1;
        }
    }
    return out;
}

} // namespace glaze::producer::detail
