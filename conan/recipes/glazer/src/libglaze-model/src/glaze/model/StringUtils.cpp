#include "glaze/model/StringUtils.h"

#include <cctype>
#include <stdexcept>

namespace glaze::model {

bool isCapitalized(std::string_view value) noexcept {
    if (value.empty()) {
        return false;
    }

    // state 0: haven't seen any characters yet — expect an upper-case letter
    // state 1: in the upper-case prefix — upper extends it, lower starts the tail
    // state 2: in the lower-case tail — any upper-case letter disqualifies
    int state = 0;
    for (char ch : value) {
        const auto c = static_cast<unsigned char>(ch);
        if (state == 0) {
            if (std::isupper(c) != 0) {
                state = 1;
            } else {
                return false;
            }
        } else if (state == 1) {
            if (std::isupper(c) != 0) {
                continue;
            }
            if (std::islower(c) != 0) {
                state = 2;
            } else {
                return false;
            }
        } else if (state == 2 && std::islower(c) == 0) {
            return false;
        }
    }

    return true;
}

std::vector<std::string> splitCapitalized(std::string_view value) {
    std::vector<std::string> entries;

    std::string current;
    std::size_t currentStart = 0;
    for (std::size_t i = 0; i < value.size(); ++i) {
        const char ch = value[i];

        if (isCapitalized(current)) {
            std::string candidate = current;
            candidate.push_back(ch);
            if (!isCapitalized(candidate)) {
                entries.push_back(current);
                current.clear();
                currentStart = i;
            }
        }

        current.push_back(ch);
    }

    entries.emplace_back(value.substr(currentStart));
    return entries;
}

std::string camelCase(std::string_view value) {
    std::string out;
    std::string word;
    auto flush = [&](std::string_view w) {
        if (w.empty()) {
            return;
        }
        out.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(w.front()))));
        for (std::size_t i = 1; i < w.size(); ++i) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(w[i]))));
        }
    };

    for (char ch : value) {
        if (ch == ' ') {
            flush(word);
            word.clear();
        } else {
            word.push_back(ch);
        }
    }
    flush(word);
    return out;
}

int versionToInt(std::string_view version) {
    const auto dot = version.find('.');
    if (dot == std::string_view::npos) {
        throw std::invalid_argument("version must be 'MAJOR.MINOR'");
    }
    const auto majorPart = version.substr(0, dot);
    const auto minorPart = version.substr(dot + 1);
    if (majorPart.empty() || minorPart.empty() || minorPart.find('.') != std::string_view::npos) {
        throw std::invalid_argument("version must be 'MAJOR.MINOR'");
    }

    auto parseInt = [](std::string_view s) -> int {
        int out = 0;
        for (char ch : s) {
            if (std::isdigit(static_cast<unsigned char>(ch)) == 0) {
                throw std::invalid_argument("version must be 'MAJOR.MINOR'");
            }
            out = out * 10 + (ch - '0');
        }
        return out;
    };

    return parseInt(majorPart) * 10 + parseInt(minorPart);
}

} // namespace glaze::model
