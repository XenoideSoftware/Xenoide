#include "xe/cmake/dsl/DslPredicateEvaluator.h"
#include "xe/cmake/dsl/FixTemplateEngine.h"
#include "xe/cmake/testing/CstSyntheticGenerator.h"

#include <catch2/catch_test_macros.hpp>

namespace xe::cmake::dsl {
    namespace {

        DslContext context_for(const xe::cmake::core::ConcreteSyntaxTree &cst, std::size_t command_index, int argument_index = -1) {
            DslContext context;
            context.file = &cst;
            const auto commands = cst.commands();
            if (command_index < commands.size()) {
                context.command = commands[command_index];
            }
            if (argument_index >= 0 && context.command != nullptr) {
                if (static_cast<std::size_t>(argument_index) < context.command->argument_count()) {
                    context.argument = &context.command->argument(static_cast<std::size_t>(argument_index));
                }
            }
            return context;
        }

    } // namespace

    TEST_CASE("DslPredicateEvaluator evaluates command name membership") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(lib src/a.cpp)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("cmd.name in ['add_library', 'add_executable']", context));
        REQUIRE_FALSE(evaluator.evaluate("cmd.name in ['add_custom_command']", context));
    }

    TEST_CASE("DslPredicateEvaluator evaluates argument properties") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (sources src/Alpha.cpp src/Beta.cpp)");
        const DslPredicateEvaluator evaluator;
        const DslContext context = context_for(cst, 0, 1);
        REQUIRE(context.argument != nullptr);
        REQUIRE(evaluator.evaluate("cmd.name == 'set' && arg.index > 0 && !arg.is_quoted", context));
        REQUIRE(evaluator.evaluate("arg.text == 'src/Alpha.cpp'", context));
    }

    TEST_CASE("DslPredicateEvaluator evaluates count with lambda") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "target_link_libraries(${target} PRIVATE lib_a lib_b PUBLIC lib_c)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("count(cmd.arguments, a -> a.index > 1 && a.text not in ['PUBLIC', 'PRIVATE', 'INTERFACE']) > 1", context));
    }

    TEST_CASE("DslPredicateEvaluator evaluates logical operators") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(lib)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("cmd.name == 'add_library' && cmd.argument_count == 1", context));
        REQUIRE(evaluator.evaluate("cmd.name == 'add_executable' || cmd.argument_count == 1", context));
        REQUIRE(evaluator.evaluate("!cmd.is_unknown", context));
    }

    TEST_CASE("DslPredicateEvaluator evaluates string primitives") {
        const auto cst = xe::cmake::testing::parse_cst("/v/libxe-core/CMakeLists.txt", "set (a 1)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("regex_search(file.path, 'libxe-.*')", context));
        REQUIRE(evaluator.evaluate("regex_match('libxe-core', 'libxe-.*')", context));
        REQUIRE(evaluator.evaluate("contains(file.path, 'libxe-core')", context));
        REQUIRE(evaluator.evaluate("starts_with(file.path, '/v/')", context));
        REQUIRE(evaluator.evaluate("ends_with(file.path, 'CMakeLists.txt')", context));
    }

    TEST_CASE("DslPredicateEvaluator evaluates file has_command") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (target \"lib\")\nadd_library(${target} src/a.cpp)\n");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("file.has_command('add_library')", context));
        REQUIRE_FALSE(evaluator.evaluate("file.has_command('add_executable')", context));
        REQUIRE(evaluator.evaluate("file.count_commands('set') == 1", context));
    }

    TEST_CASE("DslPredicateEvaluator interpolates message expressions") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(lib)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        const std::string message = evaluator.interpolate("Target ${cmd.name} in ${file.path}", context);
        REQUIRE(message == "Target add_library in /v/CMakeLists.txt");
    }

    TEST_CASE("FixTemplateEngine quote_argument wraps a raw argument") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (sources src/A.cpp)");
        const DslContext context = context_for(cst, 0, 1);
        const auto fix = FixTemplateEngine::apply_template("quote_argument", context);
        REQUIRE(fix.has_value());
        REQUIRE(fix->edits.size() == 1);
        REQUIRE(fix->edits[0].new_text == "\"src/A.cpp\"");
    }

    TEST_CASE("FixTemplateEngine split_target_link_libraries_per_line") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "target_link_libraries(${target} PRIVATE lib_a lib_b)");
        const DslContext context = context_for(cst, 0);
        const auto fix = FixTemplateEngine::apply_template("split_target_link_libraries_per_line", context);
        REQUIRE(fix.has_value());
        REQUIRE(fix->edits.size() == 1);
        REQUIRE(fix->edits[0].new_text.find("PRIVATE lib_a") != std::string::npos);
        REQUIRE(fix->edits[0].new_text.find("PRIVATE lib_b") != std::string::npos);
    }

    TEST_CASE("FixTemplateEngine returns nullopt for unknown template") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(lib)");
        const DslContext context = context_for(cst, 0);
        REQUIRE_FALSE(FixTemplateEngine::apply_template("does_not_exist", context).has_value());
    }

    TEST_CASE("FixTemplateEngine unquote_argument removes quotes") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (sources \"src/A.cpp\")");
        const DslContext context = context_for(cst, 0, 1);
        const auto fix = FixTemplateEngine::apply_template("unquote_argument", context);
        REQUIRE(fix.has_value());
        REQUIRE(fix->edits.size() == 1);
        REQUIRE(fix->edits[0].new_text == "src/A.cpp");
    }

    TEST_CASE("FixTemplateEngine unquote_argument no-ops on raw") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (sources src/A.cpp)");
        const DslContext context = context_for(cst, 0, 1);
        REQUIRE_FALSE(FixTemplateEngine::apply_template("unquote_argument", context).has_value());
    }

    TEST_CASE("FixTemplateEngine quote_argument no-ops on already-quoted") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (sources \"src/A.cpp\")");
        const DslContext context = context_for(cst, 0, 1);
        REQUIRE_FALSE(FixTemplateEngine::apply_template("quote_argument", context).has_value());
    }

    TEST_CASE("FixTemplateEngine quote_argument no-ops on bracket") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (sources [=[src/A.cpp]=])");
        const DslContext context = context_for(cst, 0, 1);
        REQUIRE_FALSE(FixTemplateEngine::apply_template("quote_argument", context).has_value());
    }

    TEST_CASE("FixTemplateEngine replace_command_name needs a name") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(lib)");
        const DslContext context = context_for(cst, 0);
        REQUIRE_FALSE(FixTemplateEngine::apply_template("replace_command_name", context).has_value());
    }

    TEST_CASE("FixTemplateEngine append_command_after needs parameters") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(lib)");
        const DslContext context = context_for(cst, 0);
        REQUIRE_FALSE(FixTemplateEngine::apply_template("append_command_after", context).has_value());
    }

    TEST_CASE("FixTemplateEngine split requires at least two dependencies") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "target_link_libraries(${target} PRIVATE lib_a)");
        const DslContext context = context_for(cst, 0);
        REQUIRE_FALSE(FixTemplateEngine::apply_template("split_target_link_libraries_per_line", context).has_value());
    }

    TEST_CASE("FixTemplateEngine split requires command context") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(lib)");
        DslContext context;
        context.file = &cst;
        REQUIRE_FALSE(FixTemplateEngine::apply_template("split_target_link_libraries_per_line", context).has_value());
    }

    TEST_CASE("DslPredicateEvaluator higher-order sequences") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(a)\nadd_executable(b)\nadd_library(c)\n");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("exists(file.commands, c -> c.name == 'add_executable')", context));
        REQUIRE_FALSE(evaluator.evaluate("exists(file.commands, c -> c.name == 'target_link_libraries')", context));
        REQUIRE(evaluator.evaluate("all(file.commands, c -> c.name in ['add_library', 'add_executable'])", context));
        REQUIRE_FALSE(evaluator.evaluate("all(file.commands, c -> c.name == 'add_library')", context));
        REQUIRE(evaluator.evaluate("len(filter(file.commands, c -> c.name == 'add_library')) == 2", context));
        REQUIRE(evaluator.evaluate("first(file.commands, c -> c.name == 'add_executable') != null", context));
        REQUIRE(evaluator.evaluate("first(file.commands, c -> c.name == 'set') == null", context));
    }

    TEST_CASE("DslPredicateEvaluator comparison operators") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(lib src/A.cpp)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("cmd.argument_count >= 2", context));
        REQUIRE(evaluator.evaluate("cmd.argument_count > 1", context));
        REQUIRE(evaluator.evaluate("cmd.argument_count <= 2", context));
        REQUIRE(evaluator.evaluate("cmd.argument_count < 3", context));
        REQUIRE_FALSE(evaluator.evaluate("cmd.argument_count > 3", context));
        REQUIRE(evaluator.evaluate("cmd.argument(1).index == 1", context));
        REQUIRE(evaluator.evaluate("cmd.argument(1).text != 'lib'", context));
    }

    TEST_CASE("DslPredicateEvaluator span properties") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(lib)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("cmd.span.start_offset == 0", context));
        REQUIRE(evaluator.evaluate("cmd.span.end_offset > cmd.span.start_offset", context));
        REQUIRE(evaluator.evaluate("cmd.span.start_line == 1", context));
        REQUIRE(evaluator.evaluate("cmd.span.start_column == 1", context));
    }

    TEST_CASE("DslPredicateEvaluator quote kinds") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a \"quoted\" raw [=[bracket]=])");
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("cmd.argument(1).is_quoted && cmd.argument(1).quote_kind == 'quoted'", context_for(cst, 0)));
        REQUIRE(evaluator.evaluate("cmd.argument(2).is_raw && cmd.argument(2).quote_kind == 'raw'", context_for(cst, 0)));
        REQUIRE(evaluator.evaluate("cmd.argument(3).is_bracket && cmd.argument(3).quote_kind == 'bracket'", context_for(cst, 0)));
    }

    TEST_CASE("DslPredicateEvaluator string and path utilities") {
        const auto cst = xe::cmake::testing::parse_cst("/v/libxe-core/CMakeLists.txt", "set (a 1)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("to_upper(cmd.name) == 'SET'", context));
        REQUIRE(evaluator.evaluate("to_lower(cmd.name) == 'set'", context));
        REQUIRE(evaluator.evaluate("'CMakeLists.txt' in split(file.path, '/')", context));
        REQUIRE(evaluator.evaluate("contains(file.path, 'libxe-core')", context));
        REQUIRE(evaluator.evaluate("len(split(file.path, '/')) >= 3", context));
        REQUIRE(evaluator.evaluate("path_basename(file.path) == 'CMakeLists.txt'", context));
        REQUIRE(evaluator.evaluate("path_stem(file.path) == 'CMakeLists'", context));
        REQUIRE(evaluator.evaluate("path_extension(file.path) == '.txt'", context));
    }

    TEST_CASE("DslPredicateEvaluator first_arg last_arg properties") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "target_link_libraries(app lib_a lib_b)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("cmd.first_arg.text == 'app'", context));
        REQUIRE(evaluator.evaluate("cmd.last_arg.text == 'lib_b'", context));
    }

    TEST_CASE("DslPredicateEvaluator gracefully handles empty expressions") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE_FALSE(evaluator.evaluate("", context));
        REQUIRE_FALSE(evaluator.evaluate("cmd.name ===", context));
        REQUIRE_FALSE(evaluator.evaluate("cmd.argument(0).text == 'a' &&", context));
    }

    TEST_CASE("DslPredicateEvaluator missing context is safe") {
        const DslPredicateEvaluator evaluator;
        DslContext context;
        REQUIRE_FALSE(evaluator.evaluate("cmd.name == 'set'", context));
    }

    TEST_CASE("DslPredicateEvaluator nested parentheses and comments") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(lib)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("(cmd.name == 'add_library') && (cmd.argument_count == 1)", context));
        REQUIRE(evaluator.evaluate("!((cmd.name == 'set'))", context));
    }

    TEST_CASE("DslPredicateEvaluator list membership and string in") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(lib)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("'add_library' in ['add_library', 'add_executable']", context));
        REQUIRE(evaluator.evaluate("'add_executable' not in ['add_library']", context));
        REQUIRE(evaluator.evaluate("'lib' in 'libxe-core'", context));
        REQUIRE_FALSE(evaluator.evaluate("'xyz' in 'libxe-core'", context));
        REQUIRE(evaluator.evaluate("'xyz' not in 'libxe-core'", context));
    }

    TEST_CASE("DslPredicateEvaluator string comparison operators") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("cmd.name < 'setz'", context));
        REQUIRE(evaluator.evaluate("cmd.name <= 'set'", context));
        REQUIRE(evaluator.evaluate("cmd.name > 'a'", context));
        REQUIRE(evaluator.evaluate("cmd.name >= 'set'", context));
    }

    TEST_CASE("DslPredicateEvaluator lambda captures and method chains") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (sources src/A.cpp src/B.cpp src/C.cpp)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("count(cmd.arguments, a -> a.index > 0 && regex_match(a.text, 'src/.*')) == 3", context));
        REQUIRE(evaluator.evaluate("exists(cmd.arguments, a -> a.text == 'src/B.cpp')", context));
        REQUIRE(evaluator.evaluate("len(filter(cmd.arguments, a -> a.index > 0)) == 3", context));
    }

    TEST_CASE("DslPredicateEvaluator nested lambdas on arguments") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "target_link_libraries(app PRIVATE lib_a lib_b PUBLIC lib_c)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("all(cmd.arguments, a -> a.index == 0 || a.index == 4 || a.text in ['PRIVATE', 'lib_a', 'lib_b', 'lib_c'])", context));
        REQUIRE_FALSE(evaluator.evaluate("all(cmd.arguments, a -> a.text not in ['lib_b'])", context));
    }

    TEST_CASE("DslPredicateEvaluator argument span coordinates") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(lib)\nset (sources src/A.cpp)");
        const DslContext context = context_for(cst, 1, 1);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("arg.span.start_line == 2", context));
        REQUIRE(evaluator.evaluate("arg.span.start_offset > 0", context));
        REQUIRE(evaluator.evaluate("arg.span.end_line >= arg.span.start_line", context));
    }

    TEST_CASE("DslPredicateEvaluator file commands size and path utils") {
        const auto cst = xe::cmake::testing::parse_cst("/v/libxe-core/CMakeLists.txt", "set (a 1)\nadd_library(x)\n");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("len(file.commands) == 2", context));
        REQUIRE(evaluator.evaluate("path_dirname(file.path) == '/v/libxe-core'", context));
        REQUIRE(evaluator.evaluate("to_upper(path_extension(file.path)) == '.TXT'", context));
        REQUIRE(evaluator.evaluate("'core' in path_basename(path_dirname(file.path))", context));
    }

    TEST_CASE("DslPredicateEvaluator count_commands with multiple matches") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)\nadd_library(x)\nadd_library(y)\n");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("file.count_commands('add_library') == 2", context));
        REQUIRE(evaluator.evaluate("file.has_command('set')", context));
    }

    TEST_CASE("DslPredicateEvaluator argument out of range returns null") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(lib)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("cmd.argument(5) == null", context));
        REQUIRE(evaluator.evaluate("cmd.argument(-1) == null", context));
        REQUIRE(evaluator.evaluate("cmd.first_arg != null", context));
    }

    TEST_CASE("DslPredicateEvaluator commands on empty file") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("len(file.commands) == 0", context));
        REQUIRE(evaluator.evaluate("first(file.commands, c -> c.name == 'x') == null", context));
    }

    TEST_CASE("DslPredicateEvaluator double-quoted strings and operators") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(lib)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("cmd.name == \"add_library\"", context));
        REQUIRE(evaluator.evaluate("cmd.name != \"add_executable\"", context));
        REQUIRE(evaluator.evaluate("cmd.argument_count >= 1", context));
        REQUIRE(evaluator.evaluate("cmd.argument_count <= 1", context));
        REQUIRE(evaluator.evaluate("cmd.argument(0).text >= \"lib\"", context));
        REQUIRE(evaluator.evaluate("cmd.argument(0).text <= \"lib\"", context));
        REQUIRE(evaluator.evaluate("cmd.argument(0).text == \"lib\"", context));
    }

    TEST_CASE("DslPredicateEvaluator comments in expressions") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "add_library(lib)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("cmd.name == 'add_library' -- trailing comment\n", context));
    }

    TEST_CASE("DslPredicateEvaluator file directory and folder_name") {
        const auto cst = xe::cmake::testing::parse_cst("/v/libxe-core/CMakeLists.txt", "set (a 1)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("file.directory == '/v/libxe-core'", context));
        REQUIRE(evaluator.evaluate("file.folder_name == 'libxe-core'", context));
        REQUIRE(evaluator.evaluate("path_basename(file.directory) == file.folder_name", context));
    }

    TEST_CASE("DslPredicateEvaluator list size and string length") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a hello)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("cmd.argument(1).text.length == 5", context));
        REQUIRE(evaluator.evaluate("cmd.arguments.size == 2", context));
    }

    TEST_CASE("DslPredicateEvaluator lambda on argument span") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (sources src/A.cpp src/B.cpp)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("count(cmd.arguments, a -> a.index > 0 && a.span.start_offset > 0) == 2", context));
        REQUIRE(evaluator.evaluate("first(cmd.arguments, a -> a.index == 1).text == 'src/A.cpp'", context));
    }

    TEST_CASE("DslPredicateEvaluator second command argument and file path property") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (target \"lib\")\nadd_library(${target})\n");
        const DslContext context = context_for(cst, 1);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("cmd.argument(0).text == '${target}'", context));
        REQUIRE(evaluator.evaluate("file.path == '/v/CMakeLists.txt'", context));
        REQUIRE(evaluator.evaluate("len(file.commands) == 2", context));
    }

    // Exercises the core Lexer through parse_cst with diverse, lossless CMake
    // syntax so that the DSL evaluator runs over genuinely rich inputs.
    TEST_CASE("DslPredicateEvaluator parses diverse CMake syntax") {
        const auto cst = xe::cmake::testing::parse_cst(
            "/v/CMakeLists.txt",
            "# header comment\n"
            "cmake_minimum_required (VERSION 3.25)\n"
            "project (Foo)\n"
            "set (target \"libxe-alpha\")\n"
            "set (sources src/A.cpp src/B.cpp)\n"
            "add_library(${target} ${sources})\n"
            "add_library(xe::alpha ALIAS ${target})\n"
            "target_link_libraries(${target} PRIVATE libxe-beta PUBLIC libxe-gamma)\n"
            "#[=[ block comment ]=]\n"
            "set (banner [=[hello\nworld]=])\n"
        );
        REQUIRE_FALSE(cst.has_errors());
        REQUIRE(cst.commands().size() == 8);

        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("len(file.commands) == 8", context));
        REQUIRE(evaluator.evaluate("file.count_commands('add_library') == 2", context));
        REQUIRE(evaluator.evaluate("file.has_command('target_link_libraries')", context));

        const DslContext alias_ctx = context_for(cst, 5);
        REQUIRE(evaluator.evaluate("cmd.name == 'add_library' && cmd.argument(1).text == 'ALIAS'", alias_ctx));

        const DslContext link_ctx = context_for(cst, 6);
        REQUIRE(evaluator.evaluate("count(cmd.arguments, a -> a.index > 1 && a.text not in ['PUBLIC', 'PRIVATE', 'INTERFACE']) == 2", link_ctx));

        const DslContext bracket_ctx = context_for(cst, 7);
        REQUIRE(evaluator.evaluate("cmd.argument(1).is_bracket", bracket_ctx));
        REQUIRE(evaluator.evaluate("starts_with(cmd.argument(1).text, 'hello')", bracket_ctx));
        REQUIRE(evaluator.evaluate("ends_with(cmd.argument(1).text, 'world')", bracket_ctx));
        REQUIRE(evaluator.evaluate("cmd.argument(1).text.length == 11", bracket_ctx));
    }

    TEST_CASE("DslPredicateEvaluator parses quoted and escaped arguments") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a \"quoted value\" raw\\ value)\n");
        REQUIRE_FALSE(cst.has_errors());
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("cmd.argument(1).is_quoted", context));
        REQUIRE(evaluator.evaluate("cmd.argument(1).text == 'quoted value'", context));
        REQUIRE(evaluator.evaluate("cmd.argument(2).is_raw", context));
        REQUIRE(evaluator.evaluate("cmd.argument(2).text == 'raw value'", context));
    }

    TEST_CASE("DslPredicateEvaluator empty command and trivia") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set ()\n\n\nadd_library(x)\n");
        REQUIRE_FALSE(cst.has_errors());
        REQUIRE(cst.commands().size() == 2);
        const DslContext context = context_for(cst, 1);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("cmd.argument_count == 0 || cmd.name == 'add_library'", context));
        REQUIRE(evaluator.evaluate("cmd.argument_count >= 0", context));
    }

    TEST_CASE("DslPredicateEvaluator split produces all segments") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("'b' in split('a;b;c;d', ';')", context));
        REQUIRE(evaluator.evaluate("'c' in split('a;b;c;d', ';')", context));
        REQUIRE(evaluator.evaluate("'d' in split('a;b;c;d', ';')", context));
        REQUIRE(evaluator.evaluate("'a' in split('a;b;c;d', ';')", context));
        REQUIRE(evaluator.evaluate("len(split('a;b;c;d', ';')) == 4", context));
        REQUIRE(evaluator.evaluate("'abc' in split('abc', ',')", context));
        REQUIRE(evaluator.evaluate("len(split('abc', ',')) == 1", context));
    }

    TEST_CASE("DslPredicateEvaluator span equality through DSL") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (a 1)\nadd_library(lib)\n");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("cmd.argument(0).span == cmd.argument(0).span", context));
        REQUIRE_FALSE(evaluator.evaluate("cmd.argument(0).span == cmd.argument(1).span", context));
        REQUIRE(evaluator.evaluate("cmd.span != cmd.argument(0).span", context));
    }

    TEST_CASE("DslPredicateEvaluator nested property access on commands") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (target \"lib\")\nadd_library(${target} src/A.cpp)\n");
        const DslContext context = context_for(cst, 1);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("cmd.argument(0).text.length == 9", context));
        REQUIRE(evaluator.evaluate("cmd.argument(1).text.length == 9", context));
        REQUIRE(evaluator.evaluate("cmd.argument(0).span.start_offset > 0", context));
        REQUIRE(evaluator.evaluate("cmd.argument(1).span.start_offset > cmd.argument(0).span.end_offset", context));
    }

    TEST_CASE("DslPredicateEvaluator identifier chains with lambdas") {
        const auto cst = xe::cmake::testing::parse_cst("/v/CMakeLists.txt", "set (sources src/One.cpp src/Two.cpp src/Three.cpp)");
        const DslContext context = context_for(cst, 0);
        const DslPredicateEvaluator evaluator;
        REQUIRE(evaluator.evaluate("count(cmd.arguments, a -> a.index > 0 && regex_match(a.text, 'src/.*')) == 3", context));
        REQUIRE(evaluator.evaluate("all(cmd.arguments, a -> a.index == 0 || starts_with(a.text, 'src/'))", context));
        REQUIRE(evaluator.evaluate("first(cmd.arguments, a -> ends_with(a.text, 'Two.cpp')).text == 'src/Two.cpp'", context));
    }

} // namespace xe::cmake::dsl