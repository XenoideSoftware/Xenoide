#pragma once

#include <string>

namespace cmcheck {

    enum class Severity {
        Off,
        Warn,
        Error,
    };

    inline std::string severityToString(Severity severity) {
        switch (severity) {
        case Severity::Off:
            return "off";
        case Severity::Warn:
            return "warn";
        case Severity::Error:
            return "error";
        }
        return "warn";
    }

    inline Severity severityFromString(const std::string &value) {
        if (value == "error") {
            return Severity::Error;
        }
        if (value == "off") {
            return Severity::Off;
        }
        return Severity::Warn;
    }

} // namespace cmcheck