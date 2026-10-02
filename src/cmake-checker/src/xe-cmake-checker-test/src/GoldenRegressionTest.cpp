#include "xe/cmake/io/FileSystem.h"
#include "xe/cmake/io/SyncWriter.h"
#include "xe/cmake/rules/CheckRunner.h"
#include "xe/cmake/rules/FixConflictResolver.h"
#include "xe/cmake/rules/RuleRegistry.h"
#include "xe/cmake/testing/CstSyntheticGenerator.h"
#include "xe/cmake/testing/PropertyAssertions.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake {

    // Golden regression checks: full-stack pipeline over a frozen synthetic
    // fixture, verifying stable rule ids and severity assignment.
    TEST_CASE("Golden regression: DSL target declaration rule") {
        const auto cst = testing::parse_cst("/v/CMakeLists.txt", "add_library(my_lib src/a.cpp)\n");

        rules::RuleRegistry registry;
        dsl::DslRule rule;
        rule.id = "target.declaration-uses-variable";
        rule.severity = core::RuleSeverity::Error;
        rule.match_node = dsl::DslNodeMatch::Command;
        rule.when = "cmd.name in ['add_library', 'add_executable'] && cmd.argument_count > 0 && cmd.argument(0).text != '${target}'";
        rule.message = "Target declaration must use ${target} (docs/CMAKE.md)";
        registry.add_dsl_rules({rule});

        const rules::CheckRunner runner(registry);
        const std::vector<core::Finding> findings = runner.run({cst}, {});
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].rule_id == "target.declaration-uses-variable");
        REQUIRE(findings[0].severity == core::Severity::Error);
        REQUIRE(testing::requireFindingMatches(findings[0], "target.declaration-uses-variable", core::Severity::Error));
    }

    TEST_CASE("Golden regression: fixit applies and second pass is clean") {
        const auto cst = testing::parse_cst("/v/CMakeLists.txt", "set (sources src/A.cpp src/B.cpp)\n");

        rules::RuleRegistry registry;
        dsl::DslRule rule;
        rule.id = "formatting.quote-source-paths";
        rule.severity = core::RuleSeverity::Warn;
        rule.match_node = dsl::DslNodeMatch::Argument;
        rule.when = "cmd.name == 'set' && cmd.argument_count > 1 && cmd.argument(0).text == 'sources' && arg.index > 0 && !arg.is_quoted";
        rule.message = "Source file must be quoted (docs/CMAKE.md)";
        rule.fix = dsl::DslFixSpec{std::string("quote_argument"), std::string(), {}};
        registry.add_dsl_rules({rule});

        const rules::CheckRunner runner(registry);
        const std::vector<core::Finding> findings = runner.run({cst}, {});
        REQUIRE(findings.size() == 2);

        const rules::FixConflictResolver resolver;
        const rules::FixConflictResolver::Result result = resolver.resolve(findings);
        REQUIRE(result.applied_indices.size() == 2);

        const std::string repaired = core::TextSplicer::splice(cst.source(), result.workspace.files[0].edits);
        const auto reparsed = testing::parse_cst("/v/CMakeLists.txt", repaired);
        const std::vector<core::Finding> recheck = runner.run({reparsed}, {});
        REQUIRE(recheck.empty());
    }

    TEST_CASE("Golden regression: manual findings require intervention") {
        std::vector<core::Finding> findings;
        core::Finding finding;
        finding.file_path = "/v/CMakeLists.txt";
        finding.rule_id = "structure.single-target-per-folder";
        findings.push_back(finding);

        const rules::FixConflictResolver resolver;
        const rules::FixConflictResolver::Result result = resolver.resolve(findings);
        REQUIRE(result.applied_indices.empty());
        REQUIRE(result.manual_indices.size() == 1);
    }

} // namespace xe::cmake