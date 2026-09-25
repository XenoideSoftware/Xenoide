#include "cmcheck/Rules.h"

#include <fmt/core.h>

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "cmcheck/ConfigLoader.h"
#include "cmcheck/FileUtil.h"
#include "cmcheck/Glob.h"

namespace cmcheck {

    namespace {

        bool isScopeKeyword(const std::string &value) {
            return value == "PUBLIC" || value == "PRIVATE" || value == "INTERFACE";
        }

        int scopeRank(const std::string &value) {
            if (value == "PRIVATE") {
                return 1;
            }
            if (value == "INTERFACE") {
                return 2;
            }
            return 0;
        }

        bool isLibraryType(const std::string &type) {
            return contains(type, "LIBRARY");
        }

        bool isExecutableType(const std::string &type) {
            return type == "EXECUTABLE";
        }

        std::string lowerCase(std::string value) {
            std::transform(value.begin(), value.end(), value.begin(), [](const unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return value;
        }

        std::size_t findCommandParen(const std::vector<Token> &tokens, std::size_t wordIndex) {
            if (wordIndex >= tokens.size() || tokens[wordIndex].kind != TokenKind::Word) {
                return std::string::npos;
            }
            std::size_t j = wordIndex + 1;
            while (j < tokens.size() && (tokens[j].kind == TokenKind::Whitespace || tokens[j].kind == TokenKind::Newline || tokens[j].kind == TokenKind::Comment)) {
                ++j;
            }
            if (j < tokens.size() && tokens[j].kind == TokenKind::LeftParen) {
                return j;
            }
            return std::string::npos;
        }

        std::string resolveTargetReference(const ProjectModel &model, const TraceCommand &command, std::size_t argIndex) {
            if (argIndex >= command.args.size()) {
                return std::string();
            }
            const std::string &reference = command.args[argIndex];
            if (!startsWith(reference, "${")) {
                return reference;
            }
            for (const auto &target : model.targets) {
                if (!target.definition_file.empty() && target.definition_file == command.file) {
                    return target.name;
                }
            }
            return std::string();
        }

        Finding makeFinding(const std::string &file, int line, int column, const std::string &ruleId, const std::string &message) {
            Finding finding;
            finding.file = file;
            finding.line = line > 0 ? line : 1;
            finding.column = column > 0 ? column : 1;
            finding.severity = Severity::Warn;
            finding.rule_id = ruleId;
            finding.message = message;
            return finding;
        }

        void checkR1(const RuleContext &context, std::vector<Finding> &findings) {
            std::map<std::string, std::vector<const TargetInfo *>> byFolder;
            for (const auto &target : context.model.targets) {
                if (target.is_alias || target.source_dir.empty() || target.definition_file.empty()) {
                    continue;
                }
                if (!isSubPath(target.definition_file, context.model.root)) {
                    continue;
                }
                byFolder[target.source_dir].push_back(&target);
            }

            for (const auto &entry : byFolder) {
                const auto &targets = entry.second;
                if (targets.size() <= 1) {
                    continue;
                }
                std::string names;
                for (std::size_t i = 0; i < targets.size(); ++i) {
                    if (i > 0) {
                        names += ", ";
                    }
                    names += targets[i]->name;
                }
                const std::string folderName = baseName(entry.first);
                for (std::size_t i = 1; i < targets.size(); ++i) {
                    findings.push_back(makeFinding(
                        targets[i]->definition_file,
                        targets[i]->definition_line,
                        1,
                        "R1.one_target_per_folder",
                        fmt::format("folder '{}' defines {} real targets ({}); one target per folder is required", folderName, targets.size(), names)
                    ));
                }
            }
        }

        void checkR2(const RuleContext &context, std::vector<Finding> &findings) {
            for (const auto &target : context.model.targets) {
                if (target.is_alias || target.source_dir.empty() || target.definition_file.empty()) {
                    continue;
                }
                if (!isSubPath(target.definition_file, context.model.root)) {
                    continue;
                }
                const std::string folderName = baseName(target.source_dir);
                const std::string stripped = stripLibPrefix(folderName);

                if (target.is_test) {
                    if (target.name != stripped || !endsWith(target.name, "-test")) {
                        findings.push_back(makeFinding(
                            target.definition_file,
                            target.definition_line,
                            1,
                            "R2c.test_name",
                            fmt::format(
                                "test target '{}' must be named '{}' (folder '{}' with 'lib' stripped) and end "
                                "with '-test'",
                                target.name,
                                stripped,
                                folderName
                            )
                        ));
                    }
                } else if (isLibraryType(target.type)) {
                    if (target.name != stripped) {
                        findings.push_back(makeFinding(
                            target.definition_file,
                            target.definition_line,
                            1,
                            "R2a.library_name",
                            fmt::format("library target '{}' must match its folder name '{}' (leading 'lib' stripped)", target.name, stripped)
                        ));
                    }
                } else if (isExecutableType(target.type)) {
                    if (target.name != folderName) {
                        findings.push_back(makeFinding(
                            target.definition_file,
                            target.definition_line,
                            1,
                            "R2b.executable_name",
                            fmt::format("executable target '{}' must match its folder name '{}' exactly", target.name, folderName)
                        ));
                    }
                }
            }
        }

        void checkR3(const RuleContext &context, std::vector<Finding> &findings) {
            for (const auto &target : context.model.targets) {
                if (target.is_alias || target.source_dir.empty()) {
                    continue;
                }
                if (!isSubPath(target.source_dir, context.model.root)) {
                    findings.push_back(makeFinding(
                        target.definition_file,
                        target.definition_line,
                        1,
                        "R3.target_location",
                        fmt::format("target '{}' is defined outside the project tree ('{}' is not under '{}')", target.name, target.source_dir, context.model.root)
                    ));
                }
            }
        }

        void checkR4(const RuleContext &context, std::vector<Finding> &findings) {
            for (const auto &command : context.model.commands) {
                if (command.cmd != "target_link_libraries" || command.args.empty()) {
                    continue;
                }
                int lastRank = -1;
                bool hasScope = false;
                for (std::size_t i = 1; i < command.args.size(); ++i) {
                    const std::string &arg = command.args[i];
                    if (isScopeKeyword(arg)) {
                        const int rank = scopeRank(arg);
                        if (rank < lastRank) {
                            findings.push_back(makeFinding(
                                command.file,
                                command.line,
                                1,
                                "R4.link_keywords",
                                fmt::format(
                                    "target_link_libraries('{}'): link keyword groups must appear in the fixed "
                                    "order PUBLIC -> PRIVATE -> INTERFACE",
                                    resolveTargetReference(context.model, command, 0)
                                )
                            ));
                        }
                        lastRank = rank;
                        hasScope = true;
                    } else {
                        if (!hasScope) {
                            findings.push_back(makeFinding(
                                command.file,
                                command.line,
                                1,
                                "R4.link_keywords",
                                fmt::format(
                                    "target_link_libraries('{}'): dependency '{}' lacks an explicit "
                                    "PUBLIC/PRIVATE/INTERFACE keyword",
                                    resolveTargetReference(context.model, command, 0),
                                    arg
                                )
                            ));
                        }
                    }
                }
            }
        }

        void checkOneDependencyPerLine(const RuleContext &context, std::vector<Finding> &findings) {
            for (const auto &entry : context.file_tokens) {
                const auto &tokens = entry.second;
                for (std::size_t i = 0; i < tokens.size(); ++i) {
                    const std::size_t paren = findCommandParen(tokens, i);
                    if (paren == std::string::npos || tokens[i].text != "target_link_libraries") {
                        continue;
                    }
                    std::vector<const Token *> args;
                    int depth = 1;
                    for (std::size_t j = paren + 1; j < tokens.size(); ++j) {
                        if (tokens[j].kind == TokenKind::LeftParen) {
                            ++depth;
                        } else if (tokens[j].kind == TokenKind::RightParen) {
                            --depth;
                            if (depth == 0) {
                                break;
                            }
                        }
                        if (depth == 1 && (tokens[j].kind == TokenKind::Word || tokens[j].kind == TokenKind::Quoted || tokens[j].kind == TokenKind::Bracket)) {
                            args.push_back(&tokens[j]);
                        }
                    }
                    if (args.size() < 2) {
                        continue;
                    }
                    std::vector<const Token *> deps;
                    for (std::size_t k = 1; k < args.size(); ++k) {
                        const Token *token = args[k];
                        if (isScopeKeyword(token->text) || startsWith(token->text, "${")) {
                            continue;
                        }
                        deps.push_back(token);
                    }
                    for (std::size_t k = 1; k < deps.size(); ++k) {
                        if (deps[k]->line == deps[k - 1]->line) {
                            findings.push_back(
                                makeFinding(entry.first, deps[k]->line, deps[k]->column, "R4.link_keywords", "target_link_libraries must list one dependency per line")
                            );
                            break;
                        }
                    }
                }
            }
        }

        void checkR5(const RuleContext &context, std::vector<Finding> &findings) {
            std::set<std::string> targetsWithIncludes;
            for (const auto &command : context.model.commands) {
                if (command.cmd != "target_include_directories" || command.args.empty()) {
                    continue;
                }
                const std::string target = resolveTargetReference(context.model, command, 0);
                if (target.empty()) {
                    continue;
                }
                targetsWithIncludes.insert(target);
                if (command.args.size() < 2 || !isScopeKeyword(command.args[1])) {
                    findings.push_back(makeFinding(
                        command.file,
                        command.line,
                        1,
                        "R5.include_directories",
                        fmt::format(
                            "target_include_directories('{}') must declare a scope "
                            "keyword (PUBLIC/PRIVATE/INTERFACE) followed by a directory",
                            target
                        )
                    ));
                } else if (command.args.size() < 3) {
                    findings.push_back(makeFinding(
                        command.file,
                        command.line,
                        1,
                        "R5.include_directories",
                        fmt::format(
                            "target_include_directories('{}') is missing the directory "
                            "argument",
                            target
                        )
                    ));
                }
            }

            for (const auto &target : context.model.targets) {
                if (target.is_alias || target.source_dir.empty()) {
                    continue;
                }
                if (isLibraryType(target.type) && !targetsWithIncludes.count(target.name)) {
                    findings.push_back(makeFinding(
                        target.definition_file,
                        target.definition_line,
                        1,
                        "R5.include_directories",
                        fmt::format("library target '{}' must declare target_include_directories(${{target}} <scope> <dir>)", target.name)
                    ));
                }
            }
        }

        void checkR6(const RuleContext &context, std::vector<Finding> &findings) {
            static const std::set<std::string> knownCommands = {
                "add_compile_definitions",
                "add_compile_options",
                "add_custom_command",
                "add_custom_target",
                "add_definitions",
                "add_dependencies",
                "add_executable",
                "add_library",
                "add_link_options",
                "add_subdirectory",
                "add_test",
                "aux_source_directory",
                "block",
                "break",
                "build_command",
                "cmake_host_system_information",
                "cmake_language",
                "cmake_minimum_required",
                "cmake_parse_arguments",
                "cmake_path",
                "cmake_policy",
                "configure_file",
                "configure_package_config_file",
                "continue",
                "create_test_sourcelist",
                "define_property",
                "else",
                "elseif",
                "enable_language",
                "enable_testing",
                "endblock",
                "endforeach",
                "endfunction",
                "endif",
                "endmacro",
                "endwhile",
                "execute_process",
                "export",
                "export_library_dependencies",
                "file",
                "find_file",
                "find_library",
                "find_package",
                "find_path",
                "find_program",
                "foreach",
                "function",
                "get_cmake_property",
                "get_directory_property",
                "get_filename_component",
                "get_property",
                "get_source_file_property",
                "get_target_property",
                "get_test_property",
                "if",
                "include",
                "include_directories",
                "include_external_msproject",
                "include_guard",
                "include_regular_expression",
                "install",
                "link_directories",
                "link_libraries",
                "list",
                "load_cache",
                "load_command",
                "macro",
                "mark_as_advanced",
                "math",
                "message",
                "option",
                "project",
                "remove_definitions",
                "return",
                "separate_arguments",
                "set",
                "set_directory_properties",
                "set_property",
                "set_source_files_properties",
                "set_target_properties",
                "set_tests_properties",
                "site_name",
                "source_group",
                "string",
                "target_compile_definitions",
                "target_compile_features",
                "target_compile_options",
                "target_include_directories",
                "target_link_directories",
                "target_link_libraries",
                "target_link_options",
                "target_precompile_headers",
                "target_sources",
                "try_compile",
                "try_run",
                "unset",
                "variable_watch",
                "while",
                "write_basic_package_version_file",
                "catch_discover_tests",
                "catch_add_tests",
                "catch_add_executable",
                "catch_parse_args",
                "cmake_dependent_option",
                "feature_summary",
                "set_package_properties",
                "add_feature_info",
                "find_package_handle_standard_args",
                "generate_export_header",
                "check_c_compiler_flag",
                "check_cxx_compiler_flag",
                "check_compiler_flag",
                "check_c_source_compiles",
                "check_cxx_source_compiles",
                "check_c_source_runs",
                "check_cxx_source_runs",
                "check_c_symbol_exists",
                "check_cxx_symbol_exists",
                "check_function_exists",
                "check_include_file",
                "check_include_file_cxx",
                "check_include_files",
                "check_library_exists",
                "check_struct_has_member",
                "check_struct_has_member_cxx",
                "check_type_size",
                "check_variable_exists",
                "check_prototype_definition",
                "check_language",
                "check_ipos_supported",
                "check_linker_flag",
                "check_ios_deployment_target",
                "check_swig_import",
                "ctest_build",
                "ctest_configure",
                "ctest_coverage",
                "ctest_empty_binary_directory",
                "ctest_memcheck",
                "ctest_read_files",
                "ctest_run_script",
                "ctest_sleep",
                "ctest_start",
                "ctest_submit",
                "ctest_test",
                "ctest_update",
                "ctest_upload",
            };

            std::set<std::string> userDefined;
            for (const auto &entry : context.file_tokens) {
                const auto &tokens = entry.second;
                for (std::size_t i = 0; i < tokens.size(); ++i) {
                    const std::size_t paren = findCommandParen(tokens, i);
                    if (paren == std::string::npos) {
                        continue;
                    }
                    if (tokens[i].text == "function" || tokens[i].text == "macro") {
                        for (std::size_t j = paren + 1; j < tokens.size(); ++j) {
                            if (tokens[j].kind == TokenKind::Word) {
                                userDefined.insert(tokens[j].text);
                                break;
                            }
                            if (tokens[j].kind == TokenKind::RightParen) {
                                break;
                            }
                        }
                    }
                }
            }

            for (const auto &entry : context.file_tokens) {
                const auto &tokens = entry.second;
                for (std::size_t i = 0; i < tokens.size(); ++i) {
                    const std::size_t paren = findCommandParen(tokens, i);
                    if (paren == std::string::npos) {
                        continue;
                    }
                    const std::string &name = tokens[i].text;
                    if (name.empty() || name.front() == '$') {
                        continue;
                    }
                    if (knownCommands.count(name) || userDefined.count(name)) {
                        continue;
                    }
                    const std::string message = name == "target_link_librarie" ? fmt::format("unknown or misspelled command '{}' (did you mean 'target_link_libraries'?)", name)
                                                                               : fmt::format("unknown or misspelled command '{}'", name);
                    findings.push_back(makeFinding(entry.first, tokens[i].line, tokens[i].column, "R6.known_commands", message));
                }
            }
        }

        void checkR7(const RuleContext &context, std::vector<Finding> &findings) {
            bool includeCatch = false;
            for (const auto &command : context.model.commands) {
                if (command.cmd == "include" && !command.args.empty() && command.args[0] == "Catch") {
                    includeCatch = true;
                    break;
                }
            }

            for (const auto &target : context.model.targets) {
                if (!target.is_test || target.source_dir.empty()) {
                    continue;
                }
                bool linksCatch2Private = false;
                for (const auto &command : context.model.commands) {
                    if (command.cmd != "target_link_libraries" || command.args.empty()) {
                        continue;
                    }
                    if (resolveTargetReference(context.model, command, 0) != target.name) {
                        continue;
                    }
                    for (std::size_t i = 1; i < command.args.size(); ++i) {
                        if (command.args[i] != "Catch2::Catch2WithMain") {
                            continue;
                        }
                        std::string scope;
                        for (std::size_t j = i; j > 0; --j) {
                            if (isScopeKeyword(command.args[j - 1])) {
                                scope = command.args[j - 1];
                                break;
                            }
                        }
                        if (scope == "PRIVATE") {
                            linksCatch2Private = true;
                        }
                    }
                }
                bool discoversTests = false;
                for (const auto &command : context.model.commands) {
                    if (command.cmd == "catch_discover_tests" && !command.args.empty() && resolveTargetReference(context.model, command, 0) == target.name) {
                        discoversTests = true;
                        break;
                    }
                }

                if (!linksCatch2Private) {
                    findings.push_back(makeFinding(
                        target.definition_file,
                        target.definition_line,
                        1,
                        "R7.catch2_test_pattern",
                        fmt::format("test target '{}' must link Catch2::Catch2WithMain as PRIVATE", target.name)
                    ));
                }
                if (!includeCatch) {
                    findings.push_back(makeFinding(target.definition_file, target.definition_line, 1, "R7.catch2_test_pattern", "test targets require 'include(Catch)'"));
                }
                if (!discoversTests) {
                    findings.push_back(makeFinding(
                        target.definition_file,
                        target.definition_line,
                        1,
                        "R7.catch2_test_pattern",
                        fmt::format("test target '{}' must call catch_discover_tests(${{target}})", target.name)
                    ));
                }
            }
        }

        void checkL1(const RuleContext &context, std::vector<Finding> &findings) {
            for (const auto &entry : context.file_tokens) {
                const auto &tokens = entry.second;
                for (std::size_t i = 0; i < tokens.size(); ++i) {
                    const std::size_t paren = findCommandParen(tokens, i);
                    if (paren == std::string::npos) {
                        continue;
                    }
                    if (paren != i + 1) {
                        findings.push_back(makeFinding(
                            entry.first,
                            tokens[paren].line,
                            tokens[paren].column,
                            "L1.space_before_paren",
                            fmt::format("command '{}' must be written without a space before '('", tokens[i].text)
                        ));
                    }
                }
            }
        }

        void checkL2(const RuleContext &context, std::vector<Finding> &findings) {
            for (const auto &entry : context.file_contents) {
                const std::string &content = entry.second;
                int line = 1;
                std::string::size_type start = 0;
                const auto checkLine = [&](std::string::size_type lineStart, std::string::size_type lineEnd) {
                    const std::string text = content.substr(lineStart, lineEnd - lineStart);
                    if (text.find('\t') != std::string::npos) {
                        const std::size_t tab = text.find('\t');
                        findings.push_back(makeFinding(entry.first, line, static_cast<int>(tab) + 1, "L2.whitespace", "tabs are not allowed; use spaces only"));
                    }
                    const bool onlySpaces = text.find_first_not_of(' ') == std::string::npos;
                    if (!onlySpaces && !text.empty()) {
                        const std::size_t first = text.find_first_not_of(' ');
                        if (first != std::string::npos && text.substr(0, first).find('\t') == std::string::npos && (first % static_cast<std::size_t>(context.config.indent)) != 0) {
                            findings.push_back(makeFinding(
                                entry.first,
                                line,
                                static_cast<int>(first) + 1,
                                "L2.whitespace",
                                fmt::format("indentation must be a multiple of {} spaces", context.config.indent)
                            ));
                        }
                    }
                    const std::size_t trailing = text.find_last_not_of(" \t");
                    if (trailing != std::string::npos && trailing + 1 < text.size()) {
                        findings.push_back(makeFinding(entry.first, line, static_cast<int>(trailing) + 2, "L2.whitespace", "trailing whitespace is not allowed"));
                    }
                };

                while (start <= content.size()) {
                    const std::string::size_type newline = content.find('\n', start);
                    if (newline == std::string::npos) {
                        checkLine(start, content.size());
                        break;
                    }
                    checkLine(start, newline);
                    start = newline + 1;
                    ++line;
                }
            }
        }

        void checkL3(const RuleContext &context, std::vector<Finding> &findings) {
            for (const auto &entry : context.file_contents) {
                const std::string &content = entry.second;
                int line = 1;
                std::string::size_type start = 0;
                while (start <= content.size()) {
                    const std::string::size_type newline = content.find('\n', start);
                    const std::string::size_type end = (newline == std::string::npos) ? content.size() : newline;
                    const int width = static_cast<int>(end - start);
                    if (width > context.config.line_length) {
                        findings.push_back(makeFinding(
                            entry.first,
                            line,
                            context.config.line_length + 1,
                            "L3.line_length",
                            fmt::format("line is {} columns long; maximum is {}", width, context.config.line_length)
                        ));
                    }
                    if (newline == std::string::npos) {
                        break;
                    }
                    start = newline + 1;
                    ++line;
                }
            }
        }

        void checkL4(const RuleContext &context, std::vector<Finding> &findings) {
            for (const auto &entry : context.file_tokens) {
                const auto &tokens = entry.second;
                for (std::size_t i = 0; i < tokens.size(); ++i) {
                    const std::size_t paren = findCommandParen(tokens, i);
                    if (paren == std::string::npos || tokens[i].text != "set") {
                        continue;
                    }
                    std::vector<const Token *> args;
                    int depth = 1;
                    for (std::size_t j = paren + 1; j < tokens.size(); ++j) {
                        if (tokens[j].kind == TokenKind::LeftParen) {
                            ++depth;
                        } else if (tokens[j].kind == TokenKind::RightParen) {
                            --depth;
                            if (depth == 0) {
                                break;
                            }
                        }
                        if (depth == 1 && (tokens[j].kind == TokenKind::Word || tokens[j].kind == TokenKind::Quoted || tokens[j].kind == TokenKind::Bracket)) {
                            args.push_back(&tokens[j]);
                        }
                    }
                    if (args.empty()) {
                        continue;
                    }
                    const std::string variable = lowerCase(args[0]->text);
                    if (!contains(variable, "source") && !contains(variable, "header")) {
                        continue;
                    }
                    std::size_t quoted = 0;
                    std::size_t unquoted = 0;
                    for (std::size_t k = 1; k < args.size(); ++k) {
                        const Token *token = args[k];
                        if (startsWith(token->text, "${")) {
                            continue;
                        }
                        if (token->kind == TokenKind::Quoted || token->kind == TokenKind::Bracket) {
                            ++quoted;
                        } else {
                            ++unquoted;
                        }
                    }
                    if (quoted > 0 && unquoted > 0) {
                        findings.push_back(makeFinding(
                            entry.first,
                            tokens[i].line,
                            tokens[i].column,
                            "L4.quoting",
                            fmt::format(
                                "source list '{}' must use consistent quoting: all "
                                "arguments quoted or all unquoted",
                                args[0]->text
                            )
                        ));
                    }
                }
            }
        }

        void checkL5(const RuleContext &context, std::vector<Finding> &findings) {
            for (const auto &entry : context.file_contents) {
                const std::vector<std::string> lines = splitLines(entry.second);
                bool previousBlank = false;
                for (std::size_t i = 0; i < lines.size(); ++i) {
                    const bool blank = trim(lines[i]).empty();
                    if (blank && previousBlank) {
                        findings.push_back(makeFinding(entry.first, static_cast<int>(i) + 1, 1, "L5.blank_lines", "at most one consecutive blank line is allowed"));
                    }
                    previousBlank = blank;
                }
            }
        }

        void checkL6(const RuleContext &context, std::vector<Finding> &findings) {
            for (const auto &entry : context.file_tokens) {
                const auto &tokens = entry.second;
                for (const Token &token : tokens) {
                    if (token.kind != TokenKind::Comment) {
                        continue;
                    }
                    std::string content = token.text;
                    const std::size_t hash = content.find('#');
                    if (hash != std::string::npos) {
                        content = content.substr(hash + 1);
                    }
                    content = trim(content);
                    if (contains(content, "\"")) {
                        findings.push_back(
                            makeFinding(entry.first, token.line, token.column, "L6.no_commented_out_code", "commented-out code fragment detected (quoted string in comment)")
                        );
                    }
                }
            }
        }

    } // namespace

    std::vector<RuleDefinition> allRuleDefinitions() {
        return {
            {"R1.one_target_per_folder", "one target per folder", Severity::Warn},
            {"R2a.library_name", "library target naming", Severity::Warn},
            {"R2b.executable_name", "executable target naming", Severity::Warn},
            {"R2c.test_name", "test target naming", Severity::Warn},
            {"R3.target_location", "target location", Severity::Warn},
            {"R4.link_keywords", "link keyword policy", Severity::Warn},
            {"R5.include_directories", "include directories policy", Severity::Warn},
            {"R6.known_commands", "known commands", Severity::Warn},
            {"R7.catch2_test_pattern", "catch2 test pattern", Severity::Warn},
            {"L1.space_before_paren", "no space before parenthesis", Severity::Warn},
            {"L2.whitespace", "whitespace policy", Severity::Warn},
            {"L3.line_length", "line length", Severity::Warn},
            {"L4.quoting", "consistent quoting", Severity::Warn},
            {"L5.blank_lines", "blank line policy", Severity::Warn},
            {"L6.no_commented_out_code", "no commented out code", Severity::Warn},
        };
    }

    void markTestTargets(ProjectModel &model) {
        std::set<std::string> testNames;
        for (const auto &command : model.commands) {
            if (command.cmd == "catch_discover_tests" && !command.args.empty()) {
                testNames.insert(resolveTargetReference(model, command, 0));
            }
            if (command.cmd == "target_link_libraries" && !command.args.empty()) {
                for (std::size_t i = 1; i < command.args.size(); ++i) {
                    if (command.args[i] == "Catch2::Catch2WithMain") {
                        testNames.insert(resolveTargetReference(model, command, 0));
                    }
                }
            }
        }
        for (auto &target : model.targets) {
            if (endsWith(baseName(target.source_dir), "-test")) {
                target.is_test = true;
            }
            if (testNames.count(target.name)) {
                target.is_test = true;
            }
        }
    }

    void markDefinitionLocations(ProjectModel &model) {
        std::map<std::string, std::vector<int>> definitionLinesByFile;
        for (const auto &command : model.commands) {
            if (command.cmd != "add_library" && command.cmd != "add_executable" && command.cmd != "add_custom_target") {
                continue;
            }
            if (command.args.empty()) {
                continue;
            }
            definitionLinesByFile[command.file].push_back(command.line);
        }

        for (auto &target : model.targets) {
            if (target.source_dir.empty()) {
                continue;
            }
            target.definition_file.clear();
            target.definition_line = 0;
            const std::string file = joinPath(target.source_dir, "CMakeLists.txt");
            const auto it = definitionLinesByFile.find(file);
            if (it == definitionLinesByFile.end() || it->second.empty()) {
                continue;
            }
            target.definition_file = file;
            target.definition_line = it->second.front();
        }
    }

    std::vector<Finding> RuleEngine::run(const RuleContext &context) const {
        std::vector<Finding> findings;

        checkR1(context, findings);
        checkR2(context, findings);
        checkR3(context, findings);
        checkR4(context, findings);
        checkOneDependencyPerLine(context, findings);
        checkR5(context, findings);
        checkR6(context, findings);
        checkR7(context, findings);
        checkL1(context, findings);
        checkL2(context, findings);
        checkL3(context, findings);
        checkL4(context, findings);
        checkL5(context, findings);
        checkL6(context, findings);

        std::map<std::string, Severity> defaults;
        for (const auto &definition : allRuleDefinitions()) {
            defaults[definition.id] = definition.default_severity;
        }

        std::vector<Finding> filtered;
        for (Finding &finding : findings) {
            if (isExcluded(context.config, finding.file)) {
                continue;
            }
            const auto configured = context.config.rule_severities.find(finding.rule_id);
            const Severity severity = configured != context.config.rule_severities.end() ? configured->second : defaults[finding.rule_id];
            if (severity == Severity::Off) {
                continue;
            }
            finding.severity = severity;
            filtered.push_back(finding);
        }

        std::sort(filtered.begin(), filtered.end(), [](const Finding &a, const Finding &b) {
            if (a.file != b.file) {
                return a.file < b.file;
            }
            if (a.line != b.line) {
                return a.line < b.line;
            }
            return a.column < b.column;
        });

        return filtered;
    }

} // namespace cmcheck