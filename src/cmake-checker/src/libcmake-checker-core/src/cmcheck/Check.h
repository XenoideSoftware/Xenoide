#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "cmcheck/Finding.h"

namespace cmcheck {

    struct CheckOptions {
        std::string project;
        std::string build_dir;
        std::string config_file;
        bool werror = false;
    };

    struct CheckResult {
        std::vector<Finding> findings;
        bool tool_error = false;
        std::string error_message;
    };

    CheckResult runCheck(const CheckOptions &options, std::ostream &out);

} // namespace cmcheck