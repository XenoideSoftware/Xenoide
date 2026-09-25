#include "cmcheck/FileUtil.h"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <sstream>

namespace cmcheck {

    namespace {

        std::string canonicalize(const std::string &path) {
            const bool absolute = !path.empty() && path.front() == '/';
            std::vector<std::string> parts;
            std::string current;
            for (const char c : path) {
                if (c == '/') {
                    if (!current.empty()) {
                        parts.push_back(current);
                        current.clear();
                    }
                } else {
                    current.push_back(c);
                }
            }
            if (!current.empty()) {
                parts.push_back(current);
            }

            std::vector<std::string> result;
            for (const std::string &part : parts) {
                if (part == ".") {
                    continue;
                }
                if (part == "..") {
                    if (!result.empty()) {
                        result.pop_back();
                    }
                    continue;
                }
                result.push_back(part);
            }

            std::string out;
            if (absolute) {
                out.push_back('/');
            }
            for (std::size_t i = 0; i < result.size(); ++i) {
                if (i > 0) {
                    out.push_back('/');
                }
                out += result[i];
            }
            if (out.empty()) {
                return absolute ? "/" : ".";
            }
            return out;
        }

    } // namespace

    std::string readTextFile(const std::string &path) {
        std::ifstream stream(path, std::ios::binary);
        if (!stream) {
            return std::string();
        }
        std::ostringstream buffer;
        buffer << stream.rdbuf();
        return buffer.str();
    }

    bool fileExists(const std::string &path) {
        struct stat info {};
        return ::stat(path.c_str(), &info) == 0 && S_ISREG(info.st_mode);
    }

    bool isDirectory(const std::string &path) {
        struct stat info {};
        return ::stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode);
    }

    bool listFiles(const std::string &directory, std::vector<std::string> &names) {
        DIR *dir = ::opendir(directory.c_str());
        if (dir == nullptr) {
            return false;
        }
        names.clear();
        while (struct dirent *entry = ::readdir(dir)) {
            const std::string name = entry->d_name;
            if (name == "." || name == "..") {
                continue;
            }
            names.push_back(name);
        }
        ::closedir(dir);
        return true;
    }

    std::string normalizePath(const std::string &path) {
        return canonicalize(path);
    }

    std::string joinPath(const std::string &base, const std::string &child) {
        if (base.empty()) {
            return child;
        }
        if (!child.empty() && child.front() == '/') {
            return canonicalize(child);
        }
        if (base.back() == '/') {
            return canonicalize(base + child);
        }
        return canonicalize(base + "/" + child);
    }

    std::string absolutePath(const std::string &path) {
        if (!path.empty() && path.front() == '/') {
            return canonicalize(path);
        }
        const std::string cwd = [&] {
            char buffer[4096];
            const char *value = getcwd(buffer, sizeof(buffer));
            return value != nullptr ? std::string(value) : std::string();
        }();
        return canonicalize(joinPath(cwd, path));
    }

    std::string dirName(const std::string &path) {
        const std::string normalized = canonicalize(path);
        const std::size_t pos = normalized.find_last_of('/');
        if (pos == std::string::npos) {
            return ".";
        }
        if (pos == 0) {
            return "/";
        }
        return normalized.substr(0, pos);
    }

    std::string baseName(const std::string &path) {
        const std::string normalized = canonicalize(path);
        const std::size_t pos = normalized.find_last_of('/');
        if (pos == std::string::npos) {
            return normalized;
        }
        return normalized.substr(pos + 1);
    }

    std::string stripLibPrefix(const std::string &name) {
        if (startsWith(name, "lib")) {
            return name.substr(3);
        }
        return name;
    }

    bool endsWith(const std::string &value, const std::string &suffix) {
        if (suffix.size() > value.size()) {
            return false;
        }
        return value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    bool startsWith(const std::string &value, const std::string &prefix) {
        if (prefix.size() > value.size()) {
            return false;
        }
        return value.compare(0, prefix.size(), prefix) == 0;
    }

    bool contains(const std::string &value, const std::string &needle) {
        return value.find(needle) != std::string::npos;
    }

    std::vector<std::string> splitLines(const std::string &content) {
        std::vector<std::string> lines;
        std::string current;
        for (const char c : content) {
            if (c == '\n') {
                lines.push_back(current);
                current.clear();
            } else {
                current.push_back(c);
            }
        }
        lines.push_back(current);
        return lines;
    }

    std::string trim(const std::string &value) {
        const std::size_t begin = value.find_first_not_of(" \t\r\n");
        if (begin == std::string::npos) {
            return std::string();
        }
        const std::size_t end = value.find_last_not_of(" \t\r\n");
        return value.substr(begin, end - begin + 1);
    }

    bool isSubPath(const std::string &path, const std::string &base) {
        if (path == base) {
            return true;
        }
        if (!startsWith(path, base)) {
            return false;
        }
        if (path.size() > base.size() && path[base.size()] == '/') {
            return true;
        }
        return false;
    }

    std::string relativePath(const std::string &base, const std::string &path) {
        const std::string normalizedBase = normalizePath(base);
        const std::string normalizedPath = normalizePath(path);
        if (isSubPath(normalizedPath, normalizedBase)) {
            if (normalizedPath == normalizedBase) {
                return ".";
            }
            return normalizedPath.substr(normalizedBase.size() + 1);
        }
        return normalizedPath;
    }

} // namespace cmcheck