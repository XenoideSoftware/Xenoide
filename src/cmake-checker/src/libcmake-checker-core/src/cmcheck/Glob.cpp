#include "cmcheck/Glob.h"

#include <string>

namespace cmcheck {

    bool globMatch(const std::string &pattern, const std::string &text) {
        std::size_t p = 0;
        std::size_t t = 0;
        std::size_t star = std::string::npos;
        std::size_t mark = 0;

        while (t < text.size()) {
            if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == text[t])) {
                ++p;
                ++t;
            } else if (p < pattern.size() && pattern[p] == '*') {
                star = p;
                mark = t;
                ++p;
            } else if (star != std::string::npos) {
                p = star + 1;
                t = ++mark;
            } else {
                return false;
            }
        }
        while (p < pattern.size() && pattern[p] == '*') {
            ++p;
        }
        return p == pattern.size();
    }

} // namespace cmcheck