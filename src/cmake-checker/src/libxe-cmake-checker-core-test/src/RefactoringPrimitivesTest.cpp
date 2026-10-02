#include "xe/cmake/core/Lexer.h"
#include "xe/cmake/core/RefactoringPrimitives.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake::core {

    TEST_CASE("RenameSymbolRefactoring renames target declarations and links") {
        const ConcreteSyntaxTree cst = Lexer::parse(
            "/v/CMakeLists.txt",
            "set (target \"alpha_legacy\")\n"
            "add_library(${target} src/A.cpp)\n"
            "target_link_libraries(app PRIVATE alpha_legacy)\n"
        );
        const WorkspaceEdit workspace = RenameSymbolRefactoring::rename_target(cst, "alpha_legacy", "libxe-alpha");
        REQUIRE_FALSE(workspace.empty());
        // set(target...) and the target_link_libraries argument are both replaced.
        std::size_t total_edits = 0;
        for (const auto &file : workspace.files) {
            total_edits += file.edits.size();
        }
        REQUIRE(total_edits == 2);
    }

    TEST_CASE("ExtractFunctionRefactoring replaces statement range") {
        const ConcreteSyntaxTree cst = Lexer::parse("/v/CMakeLists.txt", "set (a 1)\nset (b 2)\nset (c 3)\n");
        const std::vector<const CommandNode *> statements = cst.commands();
        const std::vector<TextEdit> edits = ExtractFunctionRefactoring::extract_statements(cst, {statements[1], statements[2]}, "helper");
        REQUIRE(edits.size() == 1);
        REQUIRE(edits[0].new_text.find("function(helper)") != std::string::npos);
    }

    TEST_CASE("InlineFunctionRefactoring replaces call sites") {
        const ConcreteSyntaxTree cst = Lexer::parse("/v/CMakeLists.txt", "init_flags()\nset (a 1)\n");
        const std::vector<TextEdit> edits = InlineFunctionRefactoring::inline_function(cst, "init_flags", "set (FLAGS ON)");
        REQUIRE(edits.size() == 1);
        REQUIRE(edits[0].new_text == "set (FLAGS ON)");
    }

} // namespace xe::cmake::core