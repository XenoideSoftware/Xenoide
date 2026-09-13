
#pragma once

#include <map>
#include <string>

namespace xenoide {
    enum class LanguageDialect {
        GLSL_110,
        GLSL_120,
        GLSL_130,
        GLSL_140,
        GLSL_150,
        GLSL_330,
        GLSL_400,
        GLSL_410,
        GLSL_420,
        GLSL_430,
        GLSL_440,
        GLSL_450,
    };

    struct LanguageKeywords {
        std::string keywords;
        std::string reservedKeywords;
    };

    extern const std::map<LanguageDialect, LanguageKeywords> languageMap;
}
