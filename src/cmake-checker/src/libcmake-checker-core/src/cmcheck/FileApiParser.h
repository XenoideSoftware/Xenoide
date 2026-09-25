#pragma once

#include <string>

#include "cmcheck/Model.h"

namespace cmcheck {

    class FileApiParser {
    public:
        struct Result {
            ProjectModel model;
            std::string error;
        };

        Result parse(const std::string &replyDir) const;
    };

} // namespace cmcheck