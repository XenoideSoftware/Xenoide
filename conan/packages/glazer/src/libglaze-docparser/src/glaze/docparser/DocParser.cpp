#include "glaze/docparser/DocParser.h"

#include <pugixml.hpp>

#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <vector>

namespace glaze::docparser {

namespace {

//! Mirror of `_API_TO_REFDIR` in `doc_parser.py`. Unknown APIs fall back to
//! "gl4" which is the most complete refpage set.
std::string refpagesSubdir(std::string_view api) {
    if (api == "gles1") {
        return "es1.1";
    }
    if (api == "gles2") {
        return "es3.0";
    }
    // gl, gl_compat, glsc2, or anything else -> gl4
    return "gl4";
}

//! Extract every text descendant concatenated — matches Python
//! `_text_content` in `doc_parser.py`.
void collectText(const pugi::xml_node &node, std::string &out) {
    for (auto child = node.first_child(); child; child = child.next_sibling()) {
        switch (child.type()) {
        case pugi::node_pcdata:
        case pugi::node_cdata:
            out.append(child.value());
            break;
        case pugi::node_element:
            collectText(child, out);
            break;
        default:
            break;
        }
    }
}

std::string textOf(const pugi::xml_node &node) {
    std::string buf;
    collectText(node, buf);
    return buf;
}

//! Collapse internal whitespace, strip leading/trailing. Mirrors Python
//! `_strip_whitespace` which does `" ".join(text.split())`.
std::string stripWhitespace(std::string_view text) {
    std::string out;
    out.reserve(text.size());
    bool first = true;
    for (std::size_t i = 0; i < text.size();) {
        // skip whitespace runs
        while (i < text.size() && (text[i] == ' ' || text[i] == '\t' ||
                                   text[i] == '\n' || text[i] == '\r' ||
                                   text[i] == '\f' || text[i] == '\v')) {
            ++i;
        }
        std::size_t start = i;
        while (i < text.size() && !(text[i] == ' ' || text[i] == '\t' ||
                                    text[i] == '\n' || text[i] == '\r' ||
                                    text[i] == '\f' || text[i] == '\v')) {
            ++i;
        }
        if (start < i) {
            if (!first) {
                out.push_back(' ');
            }
            out.append(text, start, i - start);
            first = false;
        }
    }
    return out;
}

//! Recursively find descendant element nodes matching the given name.
//! pugixml doesn't handle XML namespaces specially — it preserves the raw
//! element names. The OpenGL-Refpages XMLs use a default namespace with no
//! prefix, so `child.name()` equals the local name directly.
void findDeepImpl(const pugi::xml_node &parent, const char *name,
                  std::vector<pugi::xml_node> &out) {
    for (auto child = parent.first_child(); child; child = child.next_sibling()) {
        if (child.type() != pugi::node_element) {
            continue;
        }
        if (std::string_view{child.name()} == name) {
            out.push_back(child);
        }
        findDeepImpl(child, name, out);
    }
}

std::vector<pugi::xml_node> findDeep(const pugi::xml_node &parent,
                                     const char *name) {
    std::vector<pugi::xml_node> results;
    findDeepImpl(parent, name, results);
    return results;
}

pugi::xml_node findFirst(const pugi::xml_node &parent, const char *name) {
    for (auto child = parent.first_child(); child; child = child.next_sibling()) {
        if (child.type() == pugi::node_element &&
            std::string_view{child.name()} == name) {
            return child;
        }
        auto deep = findFirst(child, name);
        if (deep) {
            return deep;
        }
    }
    return {};
}

//! Look up the `xml:id` or `id` attribute on a section. DocBook 5 uses
//! `xml:id` but the Python reference also accepts plain `id`.
std::string_view sectionId(const pugi::xml_node &sect) {
    if (auto attr = sect.attribute("xml:id")) {
        return attr.value();
    }
    if (auto attr = sect.attribute("id")) {
        return attr.value();
    }
    return {};
}

struct ParsedRefpage {
    std::vector<std::string> names;
    FunctionDoc doc;
};

//! Parse one refpage XML file. Returns std::nullopt-equivalent empty
//! ParsedRefpage (no names) when the file cannot be parsed or is missing
//! a brief. Matches the Python reference's "silently skip" behaviour.
ParsedRefpage parseRefpage(const std::filesystem::path &path) {
    ParsedRefpage result;
    pugi::xml_document doc;
    // Keep DOCTYPE intact: OpenGL-Refpages files use DOCTYPE ENTITY
    // references (e.g. mathent) which pugixml happily ignores by default.
    const auto load = doc.load_file(path.string().c_str(),
                                    pugi::parse_default | pugi::parse_ws_pcdata);
    if (!load) {
        return result;
    }
    auto root = doc.document_element();
    if (!root) {
        return result;
    }

    // 1. Brief — first <refpurpose> anywhere in the tree.
    const auto purpose = findFirst(root, "refpurpose");
    if (!purpose) {
        return result;
    }
    result.doc.brief = stripWhitespace(textOf(purpose));
    if (result.doc.brief.empty()) {
        return result;
    }

    // 2. Parameters — first <refsect1 xml:id="parameters">.
    for (const auto &sect : findDeep(root, "refsect1")) {
        if (sectionId(sect) != "parameters") {
            continue;
        }
        for (const auto &varentry : findDeep(sect, "varlistentry")) {
            const auto terms = findDeep(varentry, "term");
            const auto listitems = findDeep(varentry, "listitem");
            if (terms.empty() || listitems.empty()) {
                continue;
            }
            const auto paramNodes = findDeep(terms.front(), "parameter");
            if (paramNodes.empty()) {
                continue;
            }
            const auto paramName = stripWhitespace(textOf(paramNodes.front()));
            if (paramName.empty()) {
                continue;
            }
            const auto paras = findDeep(listitems.front(), "para");
            if (paras.empty()) {
                continue;
            }
            auto desc = stripWhitespace(textOf(paras.front()));
            if (!desc.empty()) {
                result.doc.params.emplace(paramName, std::move(desc));
            }
        }
        break; // only first <refsect1 id=parameters> like Python
    }

    // 3. Names — every <refname> declared in the refpage. All share the
    //    same brief/params payload.
    for (const auto &refname : findDeep(root, "refname")) {
        auto name = stripWhitespace(textOf(refname));
        if (!name.empty()) {
            result.names.push_back(std::move(name));
        }
    }
    return result;
}

} // namespace

DocIndex DocParser::parseRefpages(const std::filesystem::path &refpagesDir,
                                  std::string_view api) const {
    DocIndex index;
    std::error_code ec;
    const auto searchDir = refpagesDir / refpagesSubdir(api);
    if (!std::filesystem::is_directory(searchDir, ec)) {
        return index;
    }
    for (const auto &entry :
         std::filesystem::directory_iterator{searchDir, ec}) {
        if (ec) {
            break;
        }
        if (!entry.is_regular_file(ec)) {
            continue;
        }
        const auto &path = entry.path();
        if (path.extension() != ".xml") {
            continue;
        }
        const auto filename = path.filename().string();
        if (filename.rfind("gl", 0) != 0) {
            continue; // match Python `filename.startswith("gl")`
        }
        auto parsed = parseRefpage(path);
        if (parsed.names.empty()) {
            continue;
        }
        for (const auto &name : parsed.names) {
            // Each name gets its own copy — matches the Python behaviour
            // where all functions documented by the same refpage share
            // the same (deep-copied) brief and params.
            index.insert_or_assign(name, parsed.doc);
        }
    }
    return index;
}

} // namespace glaze::docparser
