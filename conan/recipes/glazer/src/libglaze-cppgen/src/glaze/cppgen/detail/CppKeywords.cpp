#include "glaze/cppgen/detail/CppKeywords.h"

#include <utility>

namespace glaze::cppgen::detail {

const std::unordered_set<std::string> &cppKeywords() {
    static const std::unordered_set<std::string> kKeywords{
        "delete",    "new",      "class",    "template", "operator",
        "return",    "switch",   "case",     "default",  "break",
        "continue",  "if",       "else",     "for",      "while",
        "do",        "void",     "int",      "float",    "double",
        "bool",      "char",     "namespace","using",    "static",
        "const",     "virtual",  "public",   "private",  "protected",
        "struct",    "enum",     "union",    "typedef",  "extern",
        "inline",    "volatile", "register", "auto",     "throw",
        "try",       "catch",    "this",     "sizeof",   "true",
        "false",
    };
    return kKeywords;
}

std::string sanitizeMethodName(std::string name) {
    if (cppKeywords().count(name) != 0) {
        name.push_back('_');
    }
    return name;
}

std::string sanitizeMethodName(std::string_view name) {
    return sanitizeMethodName(std::string{name});
}

} // namespace glaze::cppgen::detail
