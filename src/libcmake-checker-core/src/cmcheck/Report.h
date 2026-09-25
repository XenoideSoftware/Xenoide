#pragma once

#include <ostream>
#include <vector>

#include "cmcheck/Finding.h"

namespace cmcheck {

    class Report {
    public:
        void print(const std::vector<Finding> &findings, std::ostream &out) const;
        int exitCode(const std::vector<Finding> &findings, bool werror) const;
    };

} // namespace cmcheck