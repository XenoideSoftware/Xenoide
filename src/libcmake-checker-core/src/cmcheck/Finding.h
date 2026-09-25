#pragma once

#include <string>

#include "cmcheck/Severity.h"

namespace cmcheck {

    struct Finding {
        std::string file;
        int line = 0;
        int column = 0;
        Severity severity = Severity::Error;
        std::string rule_id;
        std::string message;
    };

} // namespace cmcheck