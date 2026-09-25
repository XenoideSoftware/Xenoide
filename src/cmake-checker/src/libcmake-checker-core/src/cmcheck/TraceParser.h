#pragma once

#include <string>
#include <vector>

#include "cmcheck/Model.h"

namespace cmcheck {

    class TraceParser {
    public:
        std::vector<TraceCommand> parse(const std::string &traceJson) const;
    };

} // namespace cmcheck