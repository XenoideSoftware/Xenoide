#pragma once

#include <map>
#include <string>
#include <vector>

#include "cmcheck/Severity.h"

namespace cmcheck {

    struct Config {
        int line_length = 180;
        int indent = 4;
        std::map<std::string, Severity> rule_severities;
        std::vector<std::string> exclude_globs;
        std::string base_dir;
    };

} // namespace cmcheck