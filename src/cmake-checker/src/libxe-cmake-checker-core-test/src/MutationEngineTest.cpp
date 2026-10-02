#include "xe/cmake/core/MutationEngine.h"
#include "xe/cmake/testing/PropertyAssertions.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake::core {

    TEST_CASE("TextSplicer applies a single replacement") {
        const std::string source = "set (target \"old\")";
        std::vector<TextEdit> edits;
        // Replace just the word "old" (offsets 13..16), keeping the quotes.
        edits.push_back(TextEdit::replace(SourceSpan{13, 16, 1, 1, 1, 1}, "new"));
        const std::string result = TextSplicer::splice(source, edits);
        REQUIRE(result == "set (target \"new\")");
    }

    TEST_CASE("TextSplicer applies multiple non-overlapping edits in reverse order") {
        const std::string source = "abcd";
        std::vector<TextEdit> edits;
        edits.push_back(TextEdit::replace(SourceSpan{0, 1, 1, 1, 1, 1}, "A"));
        edits.push_back(TextEdit::replace(SourceSpan{3, 4, 1, 1, 1, 1}, "D"));
        const std::string result = TextSplicer::splice(source, edits);
        REQUIRE(result == "AbcD");
    }

    TEST_CASE("TextSplicer inserts before and after") {
        const std::string source = "ab";
        std::vector<TextEdit> edits;
        edits.push_back(TextEdit::insert_before(0, "<"));
        edits.push_back(TextEdit::insert_after(2, ">"));
        const std::string result = TextSplicer::splice(source, edits);
        REQUIRE(result == "<ab>");
    }

    TEST_CASE("TextSplicer removes a span") {
        const std::string source = "remove_me";
        std::vector<TextEdit> edits;
        edits.push_back(TextEdit::remove(SourceSpan{0, 9, 1, 1, 1, 1}));
        const std::string result = TextSplicer::splice(source, edits);
        REQUIRE(result.empty());
    }

    TEST_CASE("WorkspaceEdit groups edits per file") {
        WorkspaceEdit workspace;
        workspace.add_edit("/a/CMakeLists.txt", TextEdit::replace(SourceSpan{0, 1, 1, 1, 1, 1}, "x"));
        workspace.add_edit("/b/CMakeLists.txt", TextEdit::replace(SourceSpan{0, 1, 1, 1, 1, 1}, "y"));
        workspace.add_edit("/a/CMakeLists.txt", TextEdit::replace(SourceSpan{2, 3, 1, 1, 1, 1}, "z"));
        REQUIRE(workspace.files.size() == 2);
        REQUIRE(workspace.files[0].edits.size() == 2);
        REQUIRE(workspace.files[1].edits.size() == 1);
    }

    TEST_CASE("WorkspaceEdit non-overlap property holds for disjoint spans") {
        WorkspaceEdit workspace;
        workspace.add_edit("/a.txt", TextEdit::replace(SourceSpan{0, 2, 1, 1, 1, 1}, "x"));
        workspace.add_edit("/a.txt", TextEdit::replace(SourceSpan{4, 6, 1, 1, 1, 1}, "y"));
        REQUIRE(testing::requireWorkspaceEditNonOverlapping(workspace));
    }

    TEST_CASE("WorkspaceEdit overlapping spans violate the property") {
        WorkspaceEdit workspace;
        workspace.add_edit("/a.txt", TextEdit::replace(SourceSpan{0, 5, 1, 1, 1, 1}, "x"));
        workspace.add_edit("/a.txt", TextEdit::replace(SourceSpan{3, 7, 1, 1, 1, 1}, "y"));
        REQUIRE_FALSE(testing::requireWorkspaceEditNonOverlapping(workspace));
    }

    TEST_CASE("WorkspaceEdit splicing is valid for disjoint edits") {
        WorkspaceEdit workspace;
        workspace.add_edit("/a.txt", TextEdit::replace(SourceSpan{0, 1, 1, 1, 1, 1}, "X"));
        workspace.add_edit("/a.txt", TextEdit::replace(SourceSpan{4, 5, 1, 1, 1, 1}, "Y"));
        REQUIRE(testing::requireWorkspaceEditSplicingValid(workspace, "abcdXefgh"));
    }

    TEST_CASE("TextSplicer returns source unchanged for empty edits") {
        const std::string source = "unchanged";
        const std::string result = TextSplicer::splice(source, {});
        REQUIRE(result == source);
    }

    TEST_CASE("TextSplicer orders edits with identical start offsets") {
        // Two edits sharing a start offset exercise the end-offset tie-break; the
        // exact resulting interleave is an implementation detail but both edits
        // must be applied.
        const std::string source = "abcdef";
        std::vector<TextEdit> edits;
        edits.push_back(TextEdit::replace(SourceSpan{1, 3, 1, 1, 1, 1}, "X"));
        edits.push_back(TextEdit::replace(SourceSpan{1, 4, 1, 1, 1, 1}, "Z"));
        const std::string result = TextSplicer::splice(source, edits);
        REQUIRE(result.size() == source.size() - 5 + 2);
    }

    TEST_CASE("TextSplicer keeps untouched bytes identical") {
        const std::string source = "set (a \"keep\")\n# comment\nset (b \"keep\")\n";
        std::vector<TextEdit> edits;
        edits.push_back(TextEdit::replace(SourceSpan{8, 12, 1, 1, 1, 1}, "new1"));
        edits.push_back(TextEdit::replace(SourceSpan{33, 37, 1, 1, 1, 1}, "new2"));
        const std::string result = TextSplicer::splice(source, edits);
        REQUIRE(result == "set (a \"new1\")\n# comment\nset (b \"new2\")\n");
    }

    TEST_CASE("Fix default and description constructor") {
        Fix default_fix;
        REQUIRE(default_fix.edits.empty());
        Fix named_fix("named");
        REQUIRE(named_fix.description == "named");
        named_fix.add_edit(TextEdit::insert_before(0, "x"));
        REQUIRE(named_fix.edits.size() == 1);
    }

    TEST_CASE("Finding severity values are distinct") {
        const Severity info = Severity::Info;
        const Severity warn = Severity::Warn;
        const Severity error = Severity::Error;
        REQUIRE_FALSE(info == warn);
        REQUIRE_FALSE(warn == error);
        REQUIRE_FALSE(info == error);
    }

    TEST_CASE("WorkspaceEdit is empty when no edits") {
        WorkspaceEdit workspace;
        REQUIRE(workspace.empty());
        workspace.add_edit("/a.txt", TextEdit::replace(SourceSpan{0, 1, 1, 1, 1, 1}, "x"));
        REQUIRE_FALSE(workspace.empty());
    }

    TEST_CASE("Finding fix optional preserves description") {
        Finding finding;
        Fix fix("apply me");
        fix.add_edit(TextEdit::remove(SourceSpan{0, 3, 1, 1, 1, 1}));
        finding.fix = fix;
        REQUIRE(finding.has_fix());
        REQUIRE(finding.fix->description == "apply me");
        REQUIRE(finding.fix->edits.size() == 1);
    }

    TEST_CASE("TextSplicer comparator orders edits sharing a start offset") {
        // Two edits with the same start offset: the one with the larger end
        // offset is applied first (reverse end-offset tie-break). Overlapping
        // edits are the caller's responsibility; the splice is deterministic.
        const std::string source = "abcdefgh";
        std::vector<TextEdit> edits;
        edits.push_back(TextEdit::replace(SourceSpan{2, 4, 1, 1, 1, 1}, "A"));
        edits.push_back(TextEdit::replace(SourceSpan{2, 6, 1, 1, 1, 1}, "B"));
        const std::string result = TextSplicer::splice(source, edits);
        REQUIRE(result == "abAh");
    }

    TEST_CASE("TextSplicer remove and replace share ordering") {
        const std::string source = "0123456789";
        std::vector<TextEdit> edits;
        edits.push_back(TextEdit::remove(SourceSpan{0, 4, 1, 1, 1, 1}));
        edits.push_back(TextEdit::replace(SourceSpan{7, 9, 1, 1, 1, 1}, "xy"));
        const std::string result = TextSplicer::splice(source, edits);
        REQUIRE(result == "456xy9");
    }

    TEST_CASE("PropertyAssertions finding matches and fix applicability") {
        Finding matching;
        matching.rule_id = "test.rule";
        matching.severity = Severity::Error;
        REQUIRE(testing::requireFindingMatches(matching, "test.rule", Severity::Error));
        REQUIRE_FALSE(testing::requireFindingMatches(matching, "other.rule", Severity::Error));
        REQUIRE_FALSE(testing::requireFindingMatches(matching, "test.rule", Severity::Warn));

        Fix valid_fix;
        valid_fix.add_edit(TextEdit::replace(SourceSpan{0, 3, 1, 1, 1, 1}, "x"));
        REQUIRE(testing::requireFixApplicable(valid_fix, "abcdef"));

        Fix overrun_fix;
        overrun_fix.add_edit(TextEdit::replace(SourceSpan{10, 14, 1, 1, 1, 1}, "x"));
        REQUIRE_FALSE(testing::requireFixApplicable(overrun_fix, "abcdef"));

        Fix inverted_fix;
        inverted_fix.add_edit(TextEdit::replace(SourceSpan{4, 1, 1, 1, 1, 1}, "x"));
        REQUIRE_FALSE(testing::requireFixApplicable(inverted_fix, "abcdef"));
    }

    TEST_CASE("PropertyAssertions graph acyclicity and workspace properties") {
        xe::cmake::analysis::DirectedDependencyGraph cyclic;
        cyclic.add_edge("a", "b");
        cyclic.add_edge("b", "a");
        REQUIRE_FALSE(testing::requireGraphAcyclic(cyclic));

        xe::cmake::analysis::DirectedDependencyGraph dag;
        dag.add_edge("a", "b");
        dag.add_edge("b", "c");
        REQUIRE(testing::requireGraphAcyclic(dag));

        WorkspaceEdit workspace;
        workspace.add_edit("/a.txt", TextEdit::replace(SourceSpan{0, 2, 1, 1, 1, 1}, "x"));
        workspace.add_edit("/a.txt", TextEdit::replace(SourceSpan{4, 6, 1, 1, 1, 1}, "y"));
        REQUIRE(testing::requireWorkspaceEditNonOverlapping(workspace));
        REQUIRE(testing::requireWorkspaceEditSplicingValid(workspace, "abcdefghij"));

        WorkspaceEdit overlapping;
        overlapping.add_edit("/a.txt", TextEdit::replace(SourceSpan{0, 6, 1, 1, 1, 1}, "x"));
        overlapping.add_edit("/a.txt", TextEdit::replace(SourceSpan{3, 9, 1, 1, 1, 1}, "y"));
        REQUIRE_FALSE(testing::requireWorkspaceEditNonOverlapping(overlapping));
    }

} // namespace xe::cmake::core