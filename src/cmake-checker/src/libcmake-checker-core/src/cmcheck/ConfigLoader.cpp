#include "cmcheck/ConfigLoader.h"

#include <c4/std/string.hpp>
#include <ryml.hpp>
#include <ryml_std.hpp>

#include <cstdlib>
#include <exception>
#include <string>

#include "cmcheck/FileUtil.h"
#include "cmcheck/Glob.h"

namespace cmcheck {

namespace {

std::string nodeValue(ryml::ConstNodeRef node) {
    if (node.invalid() || node.val().empty()) {
        return std::string();
    }
    return std::string(node.val().str, node.val().len);
}

std::string nodeKey(ryml::ConstNodeRef node) {
    if (node.invalid() || !node.has_key()) {
        return std::string();
    }
    return std::string(node.key().str, node.key().len);
}

int parseInt(const std::string &value, int fallback) {
    char *end = nullptr;
    const long parsed = std::strtol(value.c_str(), &end, 10);
    if (end == value.c_str() || *end != '\0') {
        return fallback;
    }
    return static_cast<int>(parsed);
}

} // namespace

Config loadConfigFromString(const std::string &yamlText, const std::string &baseDir) {
    Config config;
    config.base_dir = baseDir;

    ryml::Tree tree;
    try {
        tree = ryml::parse_in_arena(ryml::to_csubstr(yamlText));
    } catch (const std::exception &) {
        return config;
    }
    const ryml::ConstNodeRef root = tree.crootref();
    if (root.invalid()) {
        return config;
    }

    if (root.has_child("line_length")) {
        const std::string value = nodeValue(root["line_length"]);
        if (!value.empty()) {
            config.line_length = parseInt(value, config.line_length);
        }
    }

    if (root.has_child("indent")) {
        const std::string value = nodeValue(root["indent"]);
        if (!value.empty()) {
            config.indent = parseInt(value, config.indent);
        }
    }

    if (root.has_child("rules") && root["rules"].is_map()) {
        for (const ryml::ConstNodeRef rule : root["rules"].children()) {
            const std::string id = nodeKey(rule);
            if (id.empty()) {
                continue;
            }
            config.rule_severities[id] = severityFromString(nodeValue(rule));
        }
    }

    if (root.has_child("exclude") && root["exclude"].is_seq()) {
        for (const ryml::ConstNodeRef entry : root["exclude"].children()) {
            const std::string pattern = nodeValue(entry);
            if (!pattern.empty()) {
                config.exclude_globs.push_back(pattern);
            }
        }
    }

    return config;
}

    Config discoverConfig(const std::string &projectRoot, const std::string &overridePath) {
        if (!overridePath.empty()) {
            const std::string absolute = absolutePath(overridePath);
            if (!fileExists(absolute)) {
                return Config();
            }
            return loadConfigFromString(readTextFile(absolute), dirName(absolute));
        }

        std::string directory = normalizePath(projectRoot);
        while (true) {
            const std::string candidate = joinPath(directory, ".cmake-check.yaml");
            if (fileExists(candidate)) {
                return loadConfigFromString(readTextFile(candidate), directory);
            }
            const std::string parent = dirName(directory);
            if (parent == directory) {
                break;
            }
            directory = parent;
        }

        Config config;
        config.base_dir = normalizePath(projectRoot);
        return config;
    }

    bool isExcluded(const Config &config, const std::string &absolutePath) {
        if (config.exclude_globs.empty()) {
            return false;
        }
        const std::string absolute = normalizePath(absolutePath);
        const std::string relative = relativePath(config.base_dir, absolute);
        for (const std::string &pattern : config.exclude_globs) {
            if (globMatch(pattern, absolute) || globMatch(pattern, relative)) {
                return true;
            }
        }
        return false;
    }

} // namespace cmcheck