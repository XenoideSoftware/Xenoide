#pragma once

#include <string>

#include "cmcheck/Config.h"

namespace cmcheck {

    Config loadConfigFromString(const std::string &yamlText, const std::string &baseDir);

    Config discoverConfig(const std::string &projectRoot, const std::string &overridePath);

    bool isExcluded(const Config &config, const std::string &absolutePath);

} // namespace cmcheck