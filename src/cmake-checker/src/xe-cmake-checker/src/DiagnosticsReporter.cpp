#include "DiagnosticsReporter.h"

namespace xe::cmake {

    std::string_view DiagnosticsReporter::severity_name(xe::cmake::core::Severity severity) {
        switch (severity) {
        case xe::cmake::core::Severity::Info:
            return "info";
        case xe::cmake::core::Severity::Error:
            return "error";
        case xe::cmake::core::Severity::Warn:
            return "warn";
        }
        return "warn";
    }

    void DiagnosticsReporter::report(const std::vector<xe::cmake::core::Finding> &findings) const {
        for (const xe::cmake::core::Finding &finding : findings) {
            out_ << finding.file_path << ":" << finding.span.start_line << ":" << finding.span.start_column << ": " << severity_name(finding.severity) << " " << finding.rule_id
                 << " " << finding.message << "\n";
            if (!finding.has_fix()) {
                out_ << "  [manual intervention required]\n";
            }
        }
    }

    int DiagnosticsReporter::exit_code(const std::vector<xe::cmake::core::Finding> &findings, bool werror) const {
        if (findings.empty()) {
            return 0;
        }
        for (const xe::cmake::core::Finding &finding : findings) {
            if (finding.severity == xe::cmake::core::Severity::Error) {
                return 1;
            }
            if (werror && finding.severity == xe::cmake::core::Severity::Warn) {
                return 1;
            }
        }
        return 0;
    }

} // namespace xe::cmake