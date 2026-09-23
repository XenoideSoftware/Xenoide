
#include "StringUtil.h"

#include <codecvt>
#include <locale>
#include <map>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <cstring>

// Suppress deprecation warnings for codecvt_utf8, which was deprecated in
// C++17 but has no standard replacement until C++26 (P2871R3). All major
// compilers still ship it and removing it is not planned in the near term.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

// Convert a wide Unicode string to a UTF-8 string
std::string utf8_encode(const std::wstring &wstr) {
    if (wstr.empty())
        return {};
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

// Convert a UTF-8 string to a wide Unicode string
std::wstring utf8_decode(const std::string &str) {
    if (str.empty())
        return {};
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

std::wstring widen(const std::string &src) {
    return utf8_decode(src);
}
std::wstring widen(const char *src) {
    return utf8_decode(src);
}
std::string narrow(const std::wstring &src) {
    return utf8_encode(src);
}
std::string narrow(const wchar_t *src) {
    return utf8_encode(src);
}

bool wildcardMatch(const char *pattern, const char *text) {
    const size_t pattern_len = std::strlen(pattern);
    const char *text_ptr = text;

    for (size_t i = 0; i < pattern_len; i++) {
        const char pattern_char = pattern[i];

        switch (pattern_char) {
        case '?':
            if (*text_ptr == '\0') {
                return false;
            }
            ++text_ptr;

            break;

        case '*': {
            const char pattern_next_char = pattern[i + 1];

            if (pattern_next_char == '\0') {
                return true;
            }

            if (pattern_next_char == *(text_ptr + 1)) {
                return wildcardMatch(&pattern[i + 1], (text_ptr + 1));
            } else {
                return wildcardMatch(&pattern[i], (text_ptr + 1));
            }

            break;
        }

        default:
            if (*text_ptr != pattern_char) {
                return false;
            }
            ++text_ptr;

            break;
        }
    }

    return *text_ptr == '\0';
}