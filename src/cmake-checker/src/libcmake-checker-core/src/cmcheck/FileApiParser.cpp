#include "cmcheck/FileApiParser.h"

#include <nlohmann/json.hpp>

#include <string>

#include "cmcheck/FileUtil.h"

namespace cmcheck {

    namespace {

        bool resolveReplyPaths(const std::string &replyDir, const nlohmann::json &index, std::string &codemodelFile, std::string &cmakeFilesFile) {
            if (!index.is_object() || !index.contains("objects") || !index["objects"].is_array()) {
                return false;
            }
            for (const auto &object : index["objects"]) {
                if (!object.is_object()) {
                    continue;
                }
                const std::string kind = object.value("kind", std::string());
                const std::string jsonFile = object.value("jsonFile", std::string());
                if (jsonFile.empty()) {
                    continue;
                }
                if (kind == "codemodel" && codemodelFile.empty()) {
                    codemodelFile = joinPath(replyDir, jsonFile);
                } else if (kind == "cmakeFiles" && cmakeFilesFile.empty()) {
                    cmakeFilesFile = joinPath(replyDir, jsonFile);
                }
            }
            return !codemodelFile.empty() && !cmakeFilesFile.empty();
        }

    } // namespace

    FileApiParser::Result FileApiParser::parse(const std::string &replyDir) const {
        Result result;

        std::string codemodelFile;
        std::string cmakeFilesFile;
        bool found = false;
        if (isDirectory(replyDir)) {
            std::vector<std::string> names;
            if (listFiles(replyDir, names)) {
                for (const std::string &name : names) {
                    if (!startsWith(name, "index-") || !endsWith(name, ".json")) {
                        continue;
                    }
                    const nlohmann::json index = nlohmann::json::parse(readTextFile(joinPath(replyDir, name)), nullptr, false);
                    if (index.is_discarded()) {
                        continue;
                    }
                    if (resolveReplyPaths(replyDir, index, codemodelFile, cmakeFilesFile)) {
                        found = true;
                        break;
                    }
                }
            }
        }

        if (!found) {
            result.error = "no usable File API reply found in " + replyDir;
            return result;
        }

        const nlohmann::json codemodel = nlohmann::json::parse(readTextFile(codemodelFile), nullptr, false);
        if (codemodel.is_discarded()) {
            result.error = "failed to parse codemodel reply: " + codemodelFile;
            return result;
        }

        const nlohmann::json cmakeFiles = nlohmann::json::parse(readTextFile(cmakeFilesFile), nullptr, false);
        if (cmakeFiles.is_discarded()) {
            result.error = "failed to parse cmakeFiles reply: " + cmakeFilesFile;
            return result;
        }

        const std::string sourceRoot = codemodel.value("paths", nlohmann::json()).value("source", std::string());
        const std::string buildRoot = codemodel.value("paths", nlohmann::json()).value("build", std::string());
        if (sourceRoot.empty()) {
            result.error = "codemodel reply has no source path";
            return result;
        }

        result.model.root = normalizePath(sourceRoot);
        result.model.build_dir = buildRoot.empty() ? std::string() : normalizePath(buildRoot);

        const auto &configurations = codemodel.value("configurations", nlohmann::json());
        if (configurations.empty() || !configurations.is_array()) {
            result.error = "codemodel reply has no configurations";
            return result;
        }
        const nlohmann::json &configuration = configurations[0];

        const auto &directories = configuration.value("directories", nlohmann::json());
        const auto &targets = configuration.value("targets", nlohmann::json());

        std::vector<std::string> directoryPaths;
        if (directories.is_array()) {
            for (const auto &directory : directories) {
                const std::string relativeSource = directory.value("source", std::string());
                directoryPaths.push_back(normalizePath(joinPath(sourceRoot, relativeSource)));
                FolderInfo folder;
                folder.path = directoryPaths.back();
                result.model.folders.push_back(folder);
            }
        }

        if (targets.is_array()) {
            for (const auto &target : targets) {
                const int directoryIndex = target.value("directoryIndex", -1);
                const std::string jsonFile = target.value("jsonFile", std::string());
                TargetInfo info;
                info.name = target.value("name", std::string());
                if (directoryIndex >= 0 && directoryIndex < static_cast<int>(directoryPaths.size())) {
                    info.source_dir = directoryPaths[static_cast<std::size_t>(directoryIndex)];
                }
                if (!jsonFile.empty()) {
                    const nlohmann::json detail = nlohmann::json::parse(readTextFile(joinPath(replyDir, jsonFile)), nullptr, false);
                    if (!detail.is_discarded()) {
                        info.type = detail.value("type", std::string());
                    }
                }
                if (info.name.empty()) {
                    continue;
                }
                const bool isAlias = startsWith(info.type, "ALIAS");
                info.is_alias = isAlias;
                result.model.targets.push_back(info);
            }
        }

        if (cmakeFiles.contains("paths") && cmakeFiles["paths"].contains("source")) {
            const std::string source = cmakeFiles["paths"]["source"].get<std::string>();
            if (!source.empty() && result.model.root.empty()) {
                result.model.root = normalizePath(source);
            }
        }

        const auto &inputs = cmakeFiles.value("inputs", nlohmann::json());
        if (inputs.is_array()) {
            for (const auto &input : inputs) {
                const std::string path = input.value("path", std::string());
                if (path.empty()) {
                    continue;
                }
                const bool isGenerated = input.value("isGenerated", false);
                const bool isExternal = input.value("isExternal", false);
                if (isGenerated || isExternal) {
                    continue;
                }
                std::string absolute;
                if (path.front() == '/') {
                    absolute = normalizePath(path);
                } else {
                    absolute = normalizePath(joinPath(result.model.root, path));
                }
                if (!endsWith(absolute, "CMakeLists.txt") && !endsWith(absolute, ".cmake")) {
                    continue;
                }
                if (fileExists(absolute)) {
                    result.model.listfiles.push_back(absolute);
                }
            }
        }

        return result;
    }

} // namespace cmcheck