#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include <cmcheck/Lexer.h>
#include <cmcheck/Rules.h>

using namespace cmcheck;

namespace {

    RuleContext makeContext(const std::string &content) {
        RuleContext context;
        context.model.root = "/proj";
        context.config = Config();
        context.file_contents["/proj/CMakeLists.txt"] = content;
        context.file_tokens["/proj/CMakeLists.txt"] = Lexer().tokenize(content);
        return context;
    }

    TargetInfo makeTarget(const std::string &name, const std::string &type, const std::string &folder) {
        TargetInfo target;
        target.name = name;
        target.type = type;
        target.source_dir = "/proj/" + folder;
        target.definition_file = "/proj/" + folder + "/CMakeLists.txt";
        target.definition_line = 1;
        target.is_alias = type.find("ALIAS") == 0;
        return target;
    }

    std::vector<Finding> findingsFor(const RuleContext &context, const std::string &ruleId) {
        const std::vector<Finding> all = RuleEngine().run(context);
        std::vector<Finding> filtered;
        for (const Finding &finding : all) {
            if (finding.rule_id == ruleId) {
                filtered.push_back(finding);
            }
        }
        return filtered;
    }

} // namespace

TEST_CASE("R1 flags folders with more than one real target", "[rules]") {
    RuleContext context = makeContext("");
    context.model.targets.push_back(makeTarget("xe-core", "STATIC_LIBRARY", "libxe-core"));
    context.model.targets.push_back(makeTarget("xe-core-extra", "STATIC_LIBRARY", "libxe-core"));
    context.model.targets.push_back(makeTarget("xe::core", "ALIAS_LIBRARY", "libxe-core"));

    const auto findings = findingsFor(context, "R1.one_target_per_folder");
    REQUIRE(findings.size() == 1);
    CHECK(findings[0].line == 1);
}

TEST_CASE("R1 passes when each folder defines a single target", "[rules]") {
    RuleContext context = makeContext("");
    context.model.targets.push_back(makeTarget("xe-core", "STATIC_LIBRARY", "libxe-core"));
    context.model.targets.push_back(makeTarget("xe-math", "STATIC_LIBRARY", "libxe-math"));
    context.model.targets.push_back(makeTarget("xe::core", "ALIAS_LIBRARY", "libxe-core"));

    CHECK(findingsFor(context, "R1.one_target_per_folder").empty());
}

TEST_CASE("R2a flags mismatched library target names", "[rules]") {
    RuleContext context = makeContext("");
    context.model.targets.push_back(makeTarget("xe-math", "STATIC_LIBRARY", "libxe-core"));

    const auto findings = findingsFor(context, "R2a.library_name");
    REQUIRE(findings.size() == 1);
    CHECK(findings[0].message.find("xe-math") != std::string::npos);
}

TEST_CASE("R2a accepts target matching folder with lib stripped", "[rules]") {
    RuleContext context = makeContext("");
    context.model.targets.push_back(makeTarget("xe-core", "STATIC_LIBRARY", "libxe-core"));

    CHECK(findingsFor(context, "R2a.library_name").empty());
}

TEST_CASE("R2b flags mismatched executable target names", "[rules]") {
    RuleContext context = makeContext("");
    context.model.targets.push_back(makeTarget("xe-ktxc", "EXECUTABLE", "xe-ktxc2"));

    const auto findings = findingsFor(context, "R2b.executable_name");
    REQUIRE(findings.size() == 1);
}

TEST_CASE("R2b accepts executable matching folder exactly", "[rules]") {
    RuleContext context = makeContext("");
    context.model.targets.push_back(makeTarget("xe-ktxc", "EXECUTABLE", "xe-ktxc"));

    CHECK(findingsFor(context, "R2b.executable_name").empty());
}

TEST_CASE("R2c enforces -test suffix for test targets", "[rules]") {
    RuleContext context = makeContext("");
    TargetInfo target = makeTarget("xe-core-test", "EXECUTABLE", "libxe-core-test");
    target.is_test = true;
    context.model.targets.push_back(target);

    CHECK(findingsFor(context, "R2c.test_name").empty());

    TargetInfo bad = makeTarget("xe-core", "EXECUTABLE", "libxe-core-test");
    bad.is_test = true;
    context.model.targets.push_back(bad);

    const auto findings = findingsFor(context, "R2c.test_name");
    REQUIRE(findings.size() == 1);
}

TEST_CASE("R3 flags targets outside the project root", "[rules]") {
    RuleContext context = makeContext("");
    TargetInfo target = makeTarget("xe-core", "STATIC_LIBRARY", "libxe-core");
    target.source_dir = "/elsewhere/libxe-core";
    context.model.targets.push_back(target);

    const auto findings = findingsFor(context, "R3.target_location");
    REQUIRE(findings.size() == 1);
}

TEST_CASE("R4 flags dependencies without scope keywords", "[rules]") {
    RuleContext context = makeContext("");
    TraceCommand command;
    command.file = "/proj/CMakeLists.txt";
    command.line = 3;
    command.cmd = "target_link_libraries";
    command.args = {"xe-core", "xe::math", "fmt::fmt"};
    context.model.commands.push_back(command);

    const auto findings = findingsFor(context, "R4.link_keywords");
    REQUIRE(findings.size() == 2);
}

TEST_CASE("R4 flags keyword groups out of order", "[rules]") {
    RuleContext context = makeContext("");
    TraceCommand command;
    command.file = "/proj/CMakeLists.txt";
    command.line = 5;
    command.cmd = "target_link_libraries";
    command.args = {"xe-core", "PUBLIC", "xe::math", "PRIVATE", "xe::interface", "PUBLIC", "fmt::fmt"};
    context.model.commands.push_back(command);

    const auto findings = findingsFor(context, "R4.link_keywords");
    REQUIRE(findings.size() == 1);
}

TEST_CASE("R4 accepts valid keyword ordering", "[rules]") {
    RuleContext context = makeContext("");
    TraceCommand command;
    command.file = "/proj/CMakeLists.txt";
    command.line = 5;
    command.cmd = "target_link_libraries";
    command.args = {"xe-core", "PUBLIC", "xe::math", "PRIVATE", "xe::interface", "INTERFACE", "fmt::fmt"};
    context.model.commands.push_back(command);

    CHECK(findingsFor(context, "R4.link_keywords").empty());
}

TEST_CASE("R4 flags multiple dependencies on a single line", "[rules]") {
    RuleContext context = makeContext("target_link_libraries(${target} PRIVATE xe::core xe::interface)\n");

    const auto findings = findingsFor(context, "R4.link_keywords");
    REQUIRE_FALSE(findings.empty());
    CHECK(findings[0].message.find("one dependency per line") != std::string::npos);
}

TEST_CASE("R4 passes one dependency per line", "[rules]") {
    RuleContext context = makeContext("target_link_libraries(${target}\n    PRIVATE xe::core\n    PRIVATE xe::interface)\n");

    CHECK(findingsFor(context, "R4.link_keywords").empty());
}

TEST_CASE("R5 flags library targets without include directories", "[rules]") {
    RuleContext context = makeContext("");
    context.model.targets.push_back(makeTarget("xe-core", "STATIC_LIBRARY", "libxe-core"));

    const auto findings = findingsFor(context, "R5.include_directories");
    REQUIRE(findings.size() == 1);
}

TEST_CASE("R5 accepts library targets declaring include directories", "[rules]") {
    RuleContext context = makeContext("");
    context.model.targets.push_back(makeTarget("xe-core", "STATIC_LIBRARY", "libxe-core"));

    TraceCommand command;
    command.file = "/proj/libxe-core/CMakeLists.txt";
    command.line = 4;
    command.cmd = "target_include_directories";
    command.args = {"xe-core", "PUBLIC", "src"};
    context.model.commands.push_back(command);

    CHECK(findingsFor(context, "R5.include_directories").empty());
}

TEST_CASE("R6 flags unknown commands and the target_link_libraries typo", "[rules]") {
    RuleContext context = makeContext("target_link_librarie(${target} PUBLIC foo)\n");
    const auto findings = findingsFor(context, "R6.known_commands");
    REQUIRE(findings.size() == 1);
    CHECK(findings[0].message.find("target_link_libraries") != std::string::npos);
}

TEST_CASE("R6 accepts user-defined functions", "[rules]") {
    RuleContext context = makeContext("function(my_helper arg)\nendfunction()\nmy_helper(1)\n");
    CHECK(findingsFor(context, "R6.known_commands").empty());
}

TEST_CASE("R7 flags test targets missing the Catch2 pattern", "[rules]") {
    RuleContext context = makeContext("");
    TargetInfo target = makeTarget("xe-core-test", "EXECUTABLE", "libxe-core-test");
    target.is_test = true;
    context.model.targets.push_back(target);

    const auto findings = findingsFor(context, "R7.catch2_test_pattern");
    REQUIRE(findings.size() == 3);
}

TEST_CASE("R7 accepts a fully conforming test target", "[rules]") {
    RuleContext context = makeContext("");
    TargetInfo target = makeTarget("xe-core-test", "EXECUTABLE", "libxe-core-test");
    target.is_test = true;
    context.model.targets.push_back(target);

    TraceCommand link;
    link.file = "/proj/libxe-core-test/CMakeLists.txt";
    link.line = 5;
    link.cmd = "target_link_libraries";
    link.args = {"xe-core-test", "PRIVATE", "Catch2::Catch2WithMain"};
    context.model.commands.push_back(link);

    TraceCommand include;
    include.file = "/proj/libxe-core-test/CMakeLists.txt";
    include.line = 7;
    include.cmd = "include";
    include.args = {"Catch"};
    context.model.commands.push_back(include);

    TraceCommand discover;
    discover.file = "/proj/libxe-core-test/CMakeLists.txt";
    discover.line = 8;
    discover.cmd = "catch_discover_tests";
    discover.args = {"xe-core-test"};
    context.model.commands.push_back(discover);

    CHECK(findingsFor(context, "R7.catch2_test_pattern").empty());
}

TEST_CASE("L1 flags a space before the opening parenthesis", "[rules]") {
    RuleContext context = makeContext("set (target \"x\")\n");
    const auto findings = findingsFor(context, "L1.space_before_paren");
    REQUIRE(findings.size() == 1);
    CHECK(findings[0].column == 5);
}

TEST_CASE("L1 accepts adjacent command names", "[rules]") {
    RuleContext context = makeContext("set(target \"x\")\n");
    CHECK(findingsFor(context, "L1.space_before_paren").empty());
}

TEST_CASE("L2 flags tabs, trailing whitespace and bad indentation", "[rules]") {
    RuleContext context = makeContext("set(x 1)\t\n  set(y 2)\n");
    const auto findings = findingsFor(context, "L2.whitespace");
    REQUIRE(findings.size() == 3);
}

TEST_CASE("L2 accepts clean whitespace", "[rules]") {
    RuleContext context = makeContext("set(x 1)\n    set(y 2)\n");
    CHECK(findingsFor(context, "L2.whitespace").empty());
}

TEST_CASE("L3 flags lines over the configured length", "[rules]") {
    std::string longLine(190, 'x');
    longLine += '\n';
    RuleContext context = makeContext(longLine);
    const auto findings = findingsFor(context, "L3.line_length");
    REQUIRE(findings.size() == 1);
    CHECK(findings[0].column == 181);
}

TEST_CASE("L3 accepts lines within the configured length", "[rules]") {
    std::string shortLine(100, 'x');
    RuleContext context = makeContext(shortLine + "\n");
    CHECK(findingsFor(context, "L3.line_length").empty());
}

TEST_CASE("L4 flags mixed quoting in source lists", "[rules]") {
    RuleContext context = makeContext("set(sources \"src/a.cpp\" src/b.cpp)\n");
    const auto findings = findingsFor(context, "L4.quoting");
    REQUIRE(findings.size() == 1);
}

TEST_CASE("L4 accepts consistent quoting and non-source lists", "[rules]") {
    RuleContext context = makeContext("set(sources \"src/a.cpp\" \"src/b.cpp\")\nset(sources src/a.cpp src/b.cpp)\n");
    CHECK(findingsFor(context, "L4.quoting").empty());
}

TEST_CASE("L5 flags more than one consecutive blank line", "[rules]") {
    RuleContext context = makeContext("set(x 1)\n\n\nset(y 2)\n");
    const auto findings = findingsFor(context, "L5.blank_lines");
    REQUIRE(findings.size() == 1);
}

TEST_CASE("L5 accepts a single blank line", "[rules]") {
    RuleContext context = makeContext("set(x 1)\n\nset(y 2)\n");
    CHECK(findingsFor(context, "L5.blank_lines").empty());
}

TEST_CASE("L6 flags commented-out code fragments", "[rules]") {
    RuleContext context = makeContext("# \"src/Foo.cpp\"\n");
    const auto findings = findingsFor(context, "L6.no_commented_out_code");
    REQUIRE(findings.size() == 1);
}

TEST_CASE("L6 accepts prose comments", "[rules]") {
    RuleContext context = makeContext("# Enable autodiscovering\n");
    CHECK(findingsFor(context, "L6.no_commented_out_code").empty());
}

TEST_CASE("markTestTargets classifies Catch2 test targets", "[rules]") {
    ProjectModel model;
    TargetInfo target = makeTarget("xe-core-test", "EXECUTABLE", "libxe-core-test");
    model.targets.push_back(target);

    TraceCommand discover;
    discover.cmd = "catch_discover_tests";
    discover.args = {"xe-core-test"};
    model.commands.push_back(discover);

    markTestTargets(model);
    CHECK(model.targets[0].is_test);
}

TEST_CASE("markDefinitionLocations records target definitions", "[rules]") {
    ProjectModel model;
    TargetInfo target = makeTarget("xe-core", "STATIC_LIBRARY", "libxe-core");
    model.targets.push_back(target);

    TraceCommand definition;
    definition.cmd = "add_library";
    definition.args = {"xe-core", "src/xe-core.cpp"};
    definition.file = "/proj/libxe-core/CMakeLists.txt";
    definition.line = 3;
    model.commands.push_back(definition);

    markDefinitionLocations(model);
    CHECK(model.targets[0].definition_file == "/proj/libxe-core/CMakeLists.txt");
    CHECK(model.targets[0].definition_line == 3);
}

TEST_CASE("markDefinitionLocations maps variable-expanded definitions to their folder", "[rules]") {
    ProjectModel model;
    TargetInfo target = makeTarget("xe-core", "STATIC_LIBRARY", "libxe-core");
    model.targets.push_back(target);

    TraceCommand definition;
    definition.cmd = "add_library";
    definition.args = {"${target}", "${sources}"};
    definition.file = "/proj/libxe-core/CMakeLists.txt";
    definition.line = 5;
    model.commands.push_back(definition);

    markDefinitionLocations(model);
    CHECK(model.targets[0].definition_file == "/proj/libxe-core/CMakeLists.txt");
    CHECK(model.targets[0].definition_line == 5);
}

TEST_CASE("markDefinitionLocations skips targets defined outside the project", "[rules]") {
    ProjectModel model;
    TargetInfo target = makeTarget("Continuous", "UTILITY", ".");
    model.targets.push_back(target);

    markDefinitionLocations(model);
    CHECK(model.targets[0].definition_file.empty());
}

TEST_CASE("R7 accepts Catch2 linked under a PRIVATE scope group", "[rules]") {
    RuleContext context = makeContext("");
    TargetInfo target = makeTarget("xe-core-test", "EXECUTABLE", "libxe-core-test");
    target.is_test = true;
    context.model.targets.push_back(target);

    TraceCommand link;
    link.file = "/proj/libxe-core-test/CMakeLists.txt";
    link.line = 5;
    link.cmd = "target_link_libraries";
    link.args = {"${target}", "PRIVATE", "xe::core", "xe::interface", "Catch2::Catch2WithMain"};
    context.model.commands.push_back(link);

    TraceCommand include;
    include.file = "/proj/libxe-core-test/CMakeLists.txt";
    include.line = 7;
    include.cmd = "include";
    include.args = {"Catch"};
    context.model.commands.push_back(include);

    TraceCommand discover;
    discover.file = "/proj/libxe-core-test/CMakeLists.txt";
    discover.line = 8;
    discover.cmd = "catch_discover_tests";
    discover.args = {"${target}"};
    context.model.commands.push_back(discover);

    CHECK(findingsFor(context, "R7.catch2_test_pattern").empty());
}

TEST_CASE("R5 recognizes target_include_directories with variable target references", "[rules]") {
    RuleContext context = makeContext("");
    context.model.targets.push_back(makeTarget("xe-core", "STATIC_LIBRARY", "libxe-core"));

    TraceCommand command;
    command.file = "/proj/libxe-core/CMakeLists.txt";
    command.line = 4;
    command.cmd = "target_include_directories";
    command.args = {"${target}", "PUBLIC", "src"};
    context.model.commands.push_back(command);

    CHECK(findingsFor(context, "R5.include_directories").empty());
}

TEST_CASE("R1 ignores synthetic dashboard targets", "[rules]") {
    ProjectModel model;
    model.root = "/proj";
    TargetInfo interfaceTarget = makeTarget("xe-interface", "INTERFACE_LIBRARY", ".");
    model.targets.push_back(interfaceTarget);
    TargetInfo dashboard = makeTarget("Continuous", "UTILITY", ".");
    model.targets.push_back(dashboard);

    TraceCommand definition;
    definition.cmd = "add_library";
    definition.args = {"${target}", "INTERFACE"};
    definition.file = "/proj/cmake/XEBaseTarget.cmake";
    definition.line = 1;
    model.commands.push_back(definition);

    markDefinitionLocations(model);

    RuleContext context = makeContext("");
    context.model = model;
    CHECK(findingsFor(context, "R1.one_target_per_folder").empty());
}