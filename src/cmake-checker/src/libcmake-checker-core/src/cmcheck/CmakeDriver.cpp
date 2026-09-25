#include "cmcheck/CmakeDriver.h"

#include <nlohmann/json.hpp>

#include <cstdlib>
#include <string>
#include <vector>

#include "cmcheck/FileUtil.h"

namespace cmcheck {

    namespace {

        std::vector<std::string> presetDerivedCandidates(const std::string &projectRoot) {
            std::vector<std::string> candidates;
            const std::string userPresets = joinPath(projectRoot, "CMakeUserPresets.json");
            if (!fileExists(userPresets)) {
                return candidates;
            }
            const nlohmann::json document = nlohmann::json::parse(readTextFile(userPresets), nullptr, false);
            if (document.is_discarded() || !document.is_object() || !document.contains("include") || !document["include"].is_array()) {
                return candidates;
            }
            for (const auto &entry : document["include"]) {
                if (!entry.is_string()) {
                    continue;
                }
                const std::string includePath = entry.get<std::string>();
                const std::string presetDir = dirName(joinPath(projectRoot, includePath));
                candidates.push_back(presetDir);
            }
            return candidates;
        }

    } // namespace

    std::string CmakeDriver::locateCmake() {
        const char *path = ::getenv("PATH");
        if (path == nullptr) {
            return "cmake";
        }
        std::string env(path);
        std::size_t start = 0;
        while (start <= env.size()) {
            const std::size_t sep = env.find(':', start);
            const std::string directory = env.substr(start, sep == std::string::npos ? std::string::npos : sep - start);
            const std::string candidate = joinPath(directory, "cmake");
            if (fileExists(candidate)) {
                return candidate;
            }
            if (sep == std::string::npos) {
                break;
            }
            start = sep + 1;
        }
        return "cmake";
    }

    std::string CmakeDriver::locateBuildDir(const std::string &projectRoot, const std::string &explicitBuildDir) {
        if (!explicitBuildDir.empty()) {
            const std::string candidate = absolutePath(explicitBuildDir);
            if (isDirectory(candidate) && hasTrace(candidate) && hasFileApiReply(candidate)) {
                return candidate;
            }
            return std::string();
        }

        std::vector<std::string> candidates;
        candidates.push_back(joinPath(projectRoot, "build-cmake-check/Release"));
        candidates.push_back(joinPath(projectRoot, "build-cmake-check/Debug"));
        candidates.push_back(joinPath(projectRoot, "build/Release"));
        candidates.push_back(joinPath(projectRoot, "build/Debug"));
        for (const std::string &candidate : presetDerivedCandidates(projectRoot)) {
            candidates.push_back(candidate);
        }

        for (const std::string &candidate : candidates) {
            if (isDirectory(candidate) && hasTrace(candidate) && hasFileApiReply(candidate)) {
                return candidate;
            }
        }
        return std::string();
    }

    bool CmakeDriver::hasTrace(const std::string &buildDir) {
        return fileExists(joinPath(buildDir, "trace.json"));
    }

    bool CmakeDriver::hasFileApiReply(const std::string &buildDir) {
        return isDirectory(joinPath(buildDir, ".cmake/api/v1/reply"));
    }

} // namespace cmcheck