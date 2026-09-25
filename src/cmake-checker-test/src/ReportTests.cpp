#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include <string>
#include <vector>

#include <cmcheck/Finding.h>
#include <cmcheck/Report.h>

using namespace cmcheck;

namespace {

    Finding makeFinding(const std::string &file, int line, int column, Severity severity, const std::string &ruleId) {
        Finding finding;
        finding.file = file;
        finding.line = line;
        finding.column = column;
        finding.severity = severity;
        finding.rule_id = ruleId;
        finding.message = "message";
        return finding;
    }

} // namespace

TEST_CASE("Report prints locatable findings", "[report]") {
    std::vector<Finding> findings;
    findings.push_back(makeFinding("/repo/src/engine/CMakeLists.txt", 4, 1, Severity::Error, "R1.one_target_per_folder"));

    std::ostringstream out;
    Report().print(findings, out);

    CHECK(out.str() == "/repo/src/engine/CMakeLists.txt:4:1: error R1.one_target_per_folder message\n");
}

TEST_CASE("Report exit code is zero without error findings", "[report]") {
    std::vector<Finding> findings;
    findings.push_back(makeFinding("/f", 1, 1, Severity::Warn, "L2.whitespace"));

    const Report report;
    CHECK(report.exitCode(findings, false) == 0);
    CHECK(report.exitCode(findings, true) == 1);
}

TEST_CASE("Report exit code is one on error findings", "[report]") {
    std::vector<Finding> findings;
    findings.push_back(makeFinding("/f", 1, 1, Severity::Error, "R2a.library_name"));

    const Report report;
    CHECK(report.exitCode(findings, false) == 1);
    CHECK(report.exitCode(findings, true) == 1);
}

TEST_CASE("Report exit code is zero for empty findings", "[report]") {
    CHECK(Report().exitCode({}, false) == 0);
    CHECK(Report().exitCode({}, true) == 0);
}