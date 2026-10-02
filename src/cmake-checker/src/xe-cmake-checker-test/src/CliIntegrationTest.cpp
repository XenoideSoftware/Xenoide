#include "xe/cmake/io/DefaultCmakeParser.h"
#include "xe/cmake/io/FileSystem.h"
#include "xe/cmake/io/ProjectLoader.h"
#include "xe/cmake/io/SyncWriter.h"
#include "xe/cmake/rules/CheckRunner.h"
#include "xe/cmake/rules/FixConflictResolver.h"
#include "xe/cmake/rules/RuleRegistry.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake {

    // CLI integration: full programmatic pipeline over an in-memory project,
    // mirroring what the xe-cmake-checker binary drives from main().
    TEST_CASE("CLI integration: full pipeline over an in-memory project") {
        io::InMemoryFileSystem fs;
        fs.create_directories("/v/libxe-a");
        fs.write_file("/v/libxe-a/CMakeLists.txt", "set (target \"libxe-a\")\nset (sources src/A.cpp)\nadd_library(${target} ${sources})\n");

        const io::DefaultCmakeParser parser(fs);
        const io::ProjectLoader loader(fs, parser);
        const std::vector<core::ConcreteSyntaxTree> files = loader.load_project("/v");
        REQUIRE(files.size() == 1);

        rules::RuleRegistry registry;
        dsl::DslRule rule;
        rule.id = "target.variable-definition";
        rule.severity = core::RuleSeverity::Error;
        rule.match_node = dsl::DslNodeMatch::File;
        rule.when = "!file.has_command('set') || count(file.commands, c -> c.name == 'set' && c.argument_count >= 2 && c.argument(0).text == 'target') == 0";
        rule.message = "Listfile is missing set(target ...) (docs/CMAKE.md)";
        registry.add_dsl_rules({rule});

        const rules::CheckRunner runner(registry);
        const std::vector<core::Finding> findings = runner.run(files, {});
        REQUIRE(findings.empty());
    }

    TEST_CASE("CLI integration: fix flow writes repaired listfile") {
        io::InMemoryFileSystem fs;
        fs.create_directories("/v/libxe-a");
        fs.write_file("/v/libxe-a/CMakeLists.txt", "set (target \"libxe-a\")\nset (sources src/A.cpp src/B.cpp)\nadd_library(${target} ${sources})\n");

        const io::DefaultCmakeParser parser(fs);
        const io::ProjectLoader loader(fs, parser);
        const std::vector<core::ConcreteSyntaxTree> files = loader.load_project("/v");

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
        const std::vector<core::Finding> findings = runner.run(files, {});
        REQUIRE(findings.size() == 2);

        const rules::FixConflictResolver resolver;
        const rules::FixConflictResolver::Result result = resolver.resolve(findings);
        const io::SyncWriter writer(fs);
        const std::vector<std::string> written = writer.apply(result.workspace);
        REQUIRE(written.size() == 1);

        std::string content;
        REQUIRE(fs.read_file("/v/libxe-a/CMakeLists.txt", content));
        REQUIRE(content.find("\"src/A.cpp\"") != std::string::npos);
        REQUIRE(content.find("\"src/B.cpp\"") != std::string::npos);
    }

} // namespace xe::cmake