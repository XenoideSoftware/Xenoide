#ifndef XE_CMAKE_CORE_QUERY_PRIMITIVES_H
#define XE_CMAKE_CORE_QUERY_PRIMITIVES_H

#include <regex>
#include <string>
#include <string_view>
#include <vector>

namespace xe::cmake::core {

    // Low-level string and regex primitives shared by the DSL evaluator and the
    // ChaiScript bindings. All functions are pure and take read-only views.
    class StringPrimitives {
    public:
        static bool regex_match(std::string_view text, std::string_view pattern);
        static bool regex_search(std::string_view text, std::string_view pattern);
        static bool str_contains(std::string_view text, std::string_view substr);
        static bool str_starts_with(std::string_view text, std::string_view prefix);
        static bool str_ends_with(std::string_view text, std::string_view suffix);
        static std::vector<std::string> str_split(std::string_view text, std::string_view delimiter);
        static std::string to_lower(std::string_view text);
        static std::string to_upper(std::string_view text);
    };

} // namespace xe::cmake::core

#endif // XE_CMAKE_CORE_QUERY_PRIMITIVES_H