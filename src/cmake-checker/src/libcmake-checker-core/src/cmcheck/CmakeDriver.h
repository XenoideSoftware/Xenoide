#pragma once

#include <string>

namespace cmcheck {

    class CmakeDriver {
    public:
        static std::string locateCmake();

        static std::string locateBuildDir(const std::string &projectRoot, const std::string &explicitBuildDir);

        static bool hasTrace(const std::string &buildDir);

        static bool hasFileApiReply(const std::string &buildDir);
    };

} // namespace cmcheck