#include "QueryPrimitives.h"

namespace xe::cmake::core {

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    bool StringPrimitives::regex_match(std::string_view text, std::string_view pattern) {
        try {
            const std::regex expression(pattern.data(), pattern.size());
            return std::regex_match(text.begin(), text.end(), expression);
        } catch (const std::regex_error &) {
            return false;
        }
    }

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    bool StringPrimitives::regex_search(std::string_view text, std::string_view pattern) {
        try {
            const std::regex expression(pattern.data(), pattern.size());
            return std::regex_search(text.begin(), text.end(), expression);
        } catch (const std::regex_error &) {
            return false;
        }
    }

    bool StringPrimitives::str_contains(std::string_view text, std::string_view substr) {
        return text.find(substr) != std::string_view::npos;
    }

    bool StringPrimitives::str_starts_with(std::string_view text, std::string_view prefix) {
        return text.size() >= prefix.size() && text.substr(0, prefix.size()) == prefix;
    }

    bool StringPrimitives::str_ends_with(std::string_view text, std::string_view suffix) {
        return text.size() >= suffix.size() && text.substr(text.size() - suffix.size()) == suffix;
    }

    std::vector<std::string> StringPrimitives::str_split(std::string_view text, std::string_view delimiter) {
        std::vector<std::string> parts;
        std::size_t start = 0;
        while (true) {
            const std::size_t pos = text.find(delimiter, start);
            if (pos == std::string_view::npos) {
                parts.emplace_back(text.substr(start));
                break;
            }
            parts.emplace_back(text.substr(start, pos - start));
            start = pos + delimiter.size();
        }
        return parts;
    }

    std::string StringPrimitives::to_lower(std::string_view text) {
        std::string result(text);
        for (char &c : result) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return result;
    }

    std::string StringPrimitives::to_upper(std::string_view text) {
        std::string result(text);
        for (char &c : result) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
        return result;
    }

} // namespace xe::cmake::core