#include "SyntheticProjectGenerator.h"

#include "xe/cmake/io/DefaultCmakeParser.h"
#include "xe/cmake/io/ProjectLoader.h"
#include "xe/cmake/io/SyncWriter.h"
#include "xe/cmake/rules/CheckRunner.h"
#include "xe/cmake/rules/FixConflictResolver.h"
#include "xe/cmake/rules/RuleRegistry.h"
#include "xe/cmake/testing/PropertyAssertions.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake {

    using namespace xe::cmake;

    namespace {

        bool recheck_clean(const rules::RuleRegistry &registry, const std::vector<core::ConcreteSyntaxTree> &files) {
            const rules::CheckRunner runner(registry);
            return runner.run(files, {}).empty();
        }

        struct DslFixture {
            io::InMemoryFileSystem fs;
            io::DefaultCmakeParser parser;
            io::ProjectLoader loader;
            std::vector<core::ConcreteSyntaxTree> files;

            DslFixture() : parser(fs), loader(fs, parser) {
                testing::CMakeProjectFixtureBuilder().withExecutableCount(1).withLibraryCount(2).withTestCount(0).withDiverseCMakeSyntaxStyles().withSeed(42).build(fs);
                files = loader.load_project("/virtual_project");
            }
        };

        rules::RuleRegistry quote_sources_registry() {
            rules::RuleRegistry registry;
            dsl::DslRule rule;
            rule.id = "formatting.quote-source-paths";
            rule.severity = core::RuleSeverity::Warn;
            rule.match_node = dsl::DslNodeMatch::Argument;
            rule.when = "cmd.name == 'set' && cmd.argument_count > 1 && cmd.argument(0).text == 'sources' && arg.index > 0 && !arg.is_quoted";
            rule.message = "Source file must be quoted (docs/CMAKE.md)";
            rule.fix = dsl::DslFixSpec{std::string("quote_argument"), std::string(), {}};
            registry.add_dsl_rules({rule});
            return registry;
        }

        rules::RuleRegistry split_links_registry() {
            rules::RuleRegistry registry;
            dsl::DslRule rule;
            rule.id = "formatting.target-link-single-dependency";
            rule.severity = core::RuleSeverity::Warn;
            rule.match_node = dsl::DslNodeMatch::Command;
            rule.match_name = "target_link_libraries";
            rule.when = "count(cmd.arguments, a -> a.index > 1 && a.text not in ['PUBLIC', 'PRIVATE', 'INTERFACE']) > 1";
            rule.message = "target_link_libraries must have one line per dependency (docs/CMAKE.md)";
            rule.fix = dsl::DslFixSpec{std::string("split_target_link_libraries_per_line"), std::string(), {}};
            registry.add_dsl_rules({rule});
            return registry;
        }

    } // namespace

    // Scenario B: unquoted source paths get quoted, second pass is clean.
    TEST_CASE("DslEndToEnd: quote unquoted source paths and recheck cleanly") {
        DslFixture fixture;

        rules::RuleRegistry registry = quote_sources_registry();
        const rules::CheckRunner runner(registry);
        std::vector<core::Finding> findings = runner.run(fixture.files, {});
        REQUIRE(!findings.empty());
        REQUIRE(testing::requireWorkspaceEditNonOverlapping(rules::FixConflictResolver().resolve(findings).workspace));

        const rules::FixConflictResolver::Result result = rules::FixConflictResolver().resolve(findings);
        REQUIRE(!result.applied_indices.empty());

        io::SyncWriter writer(fixture.fs);
        writer.apply(result.workspace);

        // Postcondition 1: reparsing yields zero errors with valid spans.
        const auto repaired_files = fixture.loader.load_project("/virtual_project");
        for (const core::ConcreteSyntaxTree &tree : repaired_files) {
            REQUIRE_FALSE(tree.has_errors());
            REQUIRE(testing::requireCstValidSpans(tree));
        }

        // Postcondition 2: clean second pass for the repaired rule.
        const std::vector<core::Finding> recheck = runner.run(repaired_files, {});
        REQUIRE(recheck.empty());
    }

    // Scenario A: multi-dependency target_link_libraries gets split.
    TEST_CASE("DslEndToEnd: split multi-dependency target_link_libraries") {
        io::InMemoryFileSystem fs;
        fs.create_directories("/virtual_project/xe-app");
        fs.write_file(
            "/virtual_project/xe-app/CMakeLists.txt",
            "set (target \"xe-app\")\n"
            "add_executable(${target} src/main.cpp)\n"
            "target_link_libraries(${target} PRIVATE lib_alpha lib_beta PUBLIC lib_gamma)\n"
        );

        const io::DefaultCmakeParser parser(fs);
        const io::ProjectLoader loader(fs, parser);
        const auto files = loader.load_project("/virtual_project");

        rules::RuleRegistry registry = split_links_registry();
        const rules::CheckRunner runner(registry);
        const std::vector<core::Finding> findings = runner.run(files, {});
        REQUIRE(findings.size() == 1);
        REQUIRE(testing::requireFindingMatches(findings[0], "formatting.target-link-single-dependency", core::Severity::Warn));

        const rules::FixConflictResolver::Result result = rules::FixConflictResolver().resolve(findings);
        REQUIRE(result.applied_indices.size() == 1);
        REQUIRE(result.workspace.files.size() == 1);

        io::SyncWriter writer(fs);
        writer.apply(result.workspace);

        std::string content;
        REQUIRE(fs.read_file("/virtual_project/xe-app/CMakeLists.txt", content));
        REQUIRE(content.find("PRIVATE lib_alpha") != std::string::npos);
        REQUIRE(content.find("PRIVATE lib_beta") != std::string::npos);
        REQUIRE(content.find("PUBLIC lib_gamma") != std::string::npos);

        const auto reparsed = loader.load_project("/virtual_project");
        REQUIRE(recheck_clean(registry, reparsed));
    }

    // Scenario C: direct target declaration replaced with ${target}.
    TEST_CASE("DslEndToEnd: replace direct target name with ${target}") {
        io::InMemoryFileSystem fs;
        fs.create_directories("/virtual_project/xe-app");
        fs.write_file("/virtual_project/xe-app/CMakeLists.txt", "set (target \"xe-app\")\nadd_executable(my_synthetic_app src/main.cpp)\n");

        const io::DefaultCmakeParser parser(fs);
        const io::ProjectLoader loader(fs, parser);
        const auto files = loader.load_project("/virtual_project");

        rules::RuleRegistry registry;
        dsl::DslRule rule;
        rule.id = "target.declaration-uses-variable";
        rule.severity = core::RuleSeverity::Error;
        rule.match_node = dsl::DslNodeMatch::Command;
        rule.when = "cmd.name in ['add_library', 'add_executable'] && cmd.argument_count > 0 && cmd.argument(0).text != '${target}'";
        rule.message = "Target declaration must use ${target}";
        rule.fix = dsl::DslFixSpec{std::string(), "Replace target identifier with ${target}", {}};
        registry.add_dsl_rules({rule});

        const rules::CheckRunner runner(registry);
        const std::vector<core::Finding> findings = runner.run(files, {});
        REQUIRE(findings.size() == 1);

        // The custom fix is not a template; verify the finding reports but the
        // fix requires the engine to synthesize the edit (manual path here).
        REQUIRE_FALSE(findings[0].has_fix());
    }

} // namespace xe::cmake