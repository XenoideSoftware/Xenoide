#ifndef XE_CMAKE_DIAGNOSTICS_REPORTER_H
#define XE_CMAKE_DIAGNOSTICS_REPORTER_H

#include "xe/cmake/core/MutationEngine.h"

#include <iostream>
#include <string_view>
#include <vector>

namespace xe::cmake {

    // Formats findings as human-readable diagnostics.
    class DiagnosticsReporter {
    public:
        explicit DiagnosticsReporter(std::ostream &out) : out_(out) {
        }

        void report(const std::vector<xe::cmake::core::Finding> &findings) const;

        // Returns the process exit code: 0 when clean, 1 when findings remain
        // (or warnings promoted by werror), 2 on tool error.
        int exit_code(const std::vector<xe::cmake::core::Finding> &findings, bool werror) const;

        static std::string_view severity_name(xe::cmake::core::Severity severity);

    private:
        std::ostream &out_;
    };

} // namespace xe::cmake

#endif // XE_CMAKE_DIAGNOSTICS_REPORTER_H