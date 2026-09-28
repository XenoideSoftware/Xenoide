#include "xe/cmake/rules/FixConflictResolver.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake::rules {

    TEST_CASE("FixConflictResolver applies non-overlapping fixes") {
        std::vector<xe::cmake::core::Finding> findings;

        xe::cmake::core::Finding first;
        first.file_path = "/v/a.txt";
        xe::cmake::core::Fix first_fix("fix a");
        first_fix.add_edit(xe::cmake::core::TextEdit::replace(xe::cmake::core::SourceSpan{0, 4, 1, 1, 1, 1}, "AAAA"));
        first.fix = first_fix;
        findings.push_back(first);

        xe::cmake::core::Finding second;
        second.file_path = "/v/a.txt";
        xe::cmake::core::Fix second_fix("fix b");
        second_fix.add_edit(xe::cmake::core::TextEdit::replace(xe::cmake::core::SourceSpan{8, 12, 1, 1, 1, 1}, "BBBB"));
        second.fix = second_fix;
        findings.push_back(second);

        const FixConflictResolver resolver;
        const FixConflictResolver::Result result = resolver.resolve(findings);
        REQUIRE(result.applied_indices.size() == 2);
        REQUIRE(result.manual_indices.empty());
        REQUIRE(result.workspace.files.size() == 1);
    }

    TEST_CASE("FixConflictResolver routes overlapping fixes to manual") {
        std::vector<xe::cmake::core::Finding> findings;

        xe::cmake::core::Finding first;
        first.file_path = "/v/a.txt";
        xe::cmake::core::Fix first_fix("fix a");
        first_fix.add_edit(xe::cmake::core::TextEdit::replace(xe::cmake::core::SourceSpan{0, 10, 1, 1, 1, 1}, "AAAA"));
        first.fix = first_fix;
        findings.push_back(first);

        xe::cmake::core::Finding second;
        second.file_path = "/v/a.txt";
        xe::cmake::core::Fix second_fix("fix b");
        second_fix.add_edit(xe::cmake::core::TextEdit::replace(xe::cmake::core::SourceSpan{5, 12, 1, 1, 1, 1}, "BBBB"));
        second.fix = second_fix;
        findings.push_back(second);

        const FixConflictResolver resolver;
        const FixConflictResolver::Result result = resolver.resolve(findings);
        REQUIRE(result.applied_indices.size() == 1);
        REQUIRE(result.manual_indices.size() == 1);
    }

    TEST_CASE("FixConflictResolver reports findings without fixes as manual") {
        std::vector<xe::cmake::core::Finding> findings;
        xe::cmake::core::Finding finding;
        finding.file_path = "/v/a.txt";
        finding.rule_id = "diagnostic.only";
        findings.push_back(finding);

        const FixConflictResolver resolver;
        const FixConflictResolver::Result result = resolver.resolve(findings);
        REQUIRE(result.applied_indices.empty());
        REQUIRE(result.manual_indices.size() == 1);
        REQUIRE(result.workspace.empty());
    }

    TEST_CASE("FixConflictResolver sorts edits in reverse-offset order") {
        std::vector<xe::cmake::core::Finding> findings;
        xe::cmake::core::Finding finding;
        finding.file_path = "/v/a.txt";
        xe::cmake::core::Fix fix("fix");
        fix.add_edit(xe::cmake::core::TextEdit::replace(xe::cmake::core::SourceSpan{10, 12, 1, 1, 1, 1}, "B"));
        fix.add_edit(xe::cmake::core::TextEdit::replace(xe::cmake::core::SourceSpan{0, 2, 1, 1, 1, 1}, "A"));
        finding.fix = fix;
        findings.push_back(finding);

        const FixConflictResolver resolver;
        const FixConflictResolver::Result result = resolver.resolve(findings);
        REQUIRE(result.applied_indices.size() == 1);
        REQUIRE(result.workspace.files[0].edits[0].span.start_offset == 10);
        REQUIRE(result.workspace.files[0].edits[1].span.start_offset == 0);
    }

    TEST_CASE("FixConflictResolver sorts same-start edits by end offset") {
        std::vector<xe::cmake::core::Finding> findings;
        xe::cmake::core::Finding finding;
        finding.file_path = "/v/a.txt";
        xe::cmake::core::Fix fix("fix");
        fix.add_edit(xe::cmake::core::TextEdit::replace(xe::cmake::core::SourceSpan{5, 8, 1, 1, 1, 1}, "X"));
        fix.add_edit(xe::cmake::core::TextEdit::replace(xe::cmake::core::SourceSpan{5, 12, 1, 1, 1, 1}, "Y"));
        finding.fix = fix;
        findings.push_back(finding);

        const FixConflictResolver resolver;
        const FixConflictResolver::Result result = resolver.resolve(findings);
        REQUIRE(result.applied_indices.size() == 1);
        REQUIRE(result.workspace.files[0].edits.size() == 2);
        // Same start offset: larger end offset is applied first.
        REQUIRE(result.workspace.files[0].edits[0].span.end_offset == 12);
        REQUIRE(result.workspace.files[0].edits[1].span.end_offset == 8);
    }

    TEST_CASE("FixConflictResolver multi-file grouping") {
        std::vector<xe::cmake::core::Finding> findings;
        xe::cmake::core::Finding a;
        a.file_path = "/v/a.txt";
        xe::cmake::core::Fix fix_a("a");
        fix_a.add_edit(xe::cmake::core::TextEdit::replace(xe::cmake::core::SourceSpan{0, 2, 1, 1, 1, 1}, "A"));
        a.fix = fix_a;
        findings.push_back(a);

        xe::cmake::core::Finding b;
        b.file_path = "/v/b.txt";
        xe::cmake::core::Fix fix_b("b");
        fix_b.add_edit(xe::cmake::core::TextEdit::replace(xe::cmake::core::SourceSpan{0, 2, 1, 1, 1, 1}, "B"));
        b.fix = fix_b;
        findings.push_back(b);

        const FixConflictResolver resolver;
        const FixConflictResolver::Result result = resolver.resolve(findings);
        REQUIRE(result.applied_indices.size() == 2);
        REQUIRE(result.workspace.files.size() == 2);
    }

    TEST_CASE("FixConflictResolver empty fix list yields empty workspace") {
        const FixConflictResolver resolver;
        const FixConflictResolver::Result result = resolver.resolve({});
        REQUIRE(result.applied_indices.empty());
        REQUIRE(result.manual_indices.empty());
        REQUIRE(result.workspace.empty());
    }

} // namespace xe::cmake::rules