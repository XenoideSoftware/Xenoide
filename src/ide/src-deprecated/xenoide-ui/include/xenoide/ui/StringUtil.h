
#pragma once 

#include <string>

extern std::wstring widen(const std::string &src);

extern std::wstring widen(const char *src);

extern std::string narrow(const std::wstring &src);

extern std::string narrow(const wchar_t *src);

extern bool wildcard_match(const char *pattern, const char *text);

enum FIND_FLAGS {
    FF_DEFAULT = 0x00,
    FF_MATCH_CASE = 0x01,
    FF_MATCH_WHOLE_WORD = 0x02
};

extern std::size_t find(const char *text, const std::size_t offset, const char *param, const FIND_FLAGS flags);
