
#include <xenoide/ui/StringUtil.h>

#include <cassert>
#include <codecvt>
#include <cstring>
#include <locale>

std::wstring widen(const std::string &src) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring result = converter.from_bytes(src);

    return result;
}

std::wstring widen(const char *src) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring result = converter.from_bytes(src);

    return result;
}

std::string narrow(const std::wstring &src) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::string result = converter.to_bytes(src);

    return result;
}

std::string narrow(const wchar_t *src) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::string result = converter.to_bytes(src);

    return result;
}

bool wildcard_match(const char *pattern, const char *text) {
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
                return wildcard_match(&pattern[i + 1], (text_ptr + 1));
            } else {
                return wildcard_match(&pattern[i], (text_ptr + 1));
            }

            return false;
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

template <typename CharComparator> std::size_t find_impl(const char *str, const char *substr, CharComparator equals_fn) {
    assert(str);
    assert(substr);

    const char *const str_original = str;
    const char *const substr_original = substr;

    do {
        if (equals_fn(*str, *substr)) {
            do {
                str++;
                substr++;
            } while (equals_fn(*str, *substr) && *str != '\0' && *substr != '\0');

            if (*substr == '\0') {
                return (str - str_original) - (substr - substr_original);
            }

            substr = substr_original;
        }
    } while (*(str++) != '\0');

    return std::string::npos;
}

std::size_t find(const char *str, const std::size_t offset, const char *substr, const FIND_FLAGS flags) {
    if (flags == FF_DEFAULT) {
        const std::size_t partial_offset = find_impl(&str[offset], substr, [](const char ch1, const char ch2) { return std::tolower(ch1) == std::tolower(ch2); });

        if (partial_offset == std::string::npos) {
            return std::string::npos;
        }

        return offset + partial_offset;
    }

    if (flags == FF_MATCH_CASE) {
        const std::size_t partial_offset = find_impl(&str[offset], substr, [](const char ch1, const char ch2) { return ch1 == ch2; });

        return offset + partial_offset;
    }

    return std::string::npos;
}
