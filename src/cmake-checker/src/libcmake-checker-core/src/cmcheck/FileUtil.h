#pragma once

#include <string>
#include <vector>

namespace cmcheck {

    std::string readTextFile(const std::string &path);

    bool fileExists(const std::string &path);

    bool isDirectory(const std::string &path);

    bool listFiles(const std::string &directory, std::vector<std::string> &names);

    std::string normalizePath(const std::string &path);

    std::string joinPath(const std::string &base, const std::string &child);

    std::string absolutePath(const std::string &path);

    std::string dirName(const std::string &path);

    std::string baseName(const std::string &path);

    std::string stripLibPrefix(const std::string &name);

    bool endsWith(const std::string &value, const std::string &suffix);

    bool startsWith(const std::string &value, const std::string &prefix);

    bool contains(const std::string &value, const std::string &needle);

    std::vector<std::string> splitLines(const std::string &content);

    std::string trim(const std::string &value);

    bool isSubPath(const std::string &path, const std::string &base);

    std::string relativePath(const std::string &base, const std::string &path);

} // namespace cmcheck