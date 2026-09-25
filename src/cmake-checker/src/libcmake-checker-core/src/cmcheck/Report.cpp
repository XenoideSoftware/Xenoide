#include "cmcheck/Report.h"

#include <fmt/core.h>

#include <string>

#include "cmcheck/FileUtil.h"

namespace cmcheck {

    void Report::print(const std::vector<Finding> &findings, std::ostream &out) const {
        for (const Finding &finding : findings) {
            out << fmt::format("{}:{}:{}: {} {} {}\n", finding.file, finding.line, finding.column, severityToString(finding.severity), finding.rule_id, finding.message);
        }
    }

    int Report::exitCode(const std::vector<Finding> &findings, bool werror) const {
        for (const Finding &finding : findings) {
            if (finding.severity == Severity::Error) {
                return 1;
            }
            if (werror && finding.severity == Severity::Warn) {
                return 1;
            }
        }
        return 0;
    }

} // namespace cmcheck