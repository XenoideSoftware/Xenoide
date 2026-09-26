workspace "xe-cmake-checker" "Architecture model for the Xenoide CMake style checker and semantic refactoring toolsuite." {
    !identifiers hierarchical

    model {
        developer = person "Software Engineer" "Developer writing, building, and refactoring CMake projects in Xenoide."
        ci = softwareSystem "CI / Dev Pipeline" "Automated build and continuous integration runner (Mise, GitHub Actions)."

        targetProject = softwareSystem "CMake Project Tree" "Target repository CMakeLists.txt files and cmake modules (e.g. src/engine)." "External"
        cmakeBuild = softwareSystem "CMake Build System" "CMake binary generating File API (codemodel-v2) and trace (json-v1) records." "External"

        xeCmake = softwareSystem "xe-cmake" "Custom CMake style checking, validation, and semantic refactoring platform for Xenoide." {

            // Frontends
            xe_cmake_checker = container "xe-cmake-checker" "CLI tool for checking CMake files, emitting diagnostics, and applying surgical fixits." "C++17 Executable" {
                cli_opts = component "CliOptionsParser" "Parses CLI flags (--check, --fix, --diff, --project) using cxxopts." "C++17 / cxxopts"
                diag_reporter = component "DiagnosticsReporter" "Formats findings with color highlights, source spans, and manual intervention tags." "C++17 / fmt"
                checker_driver = component "CheckerDriver" "Orchestrates project loading, analysis, rule execution, and fix write-back." "C++17"
            }

            xe_cmake_refactor = container "xe-cmake-refactor" "Future CLI tool for project-wide semantic refactoring transformations." "C++17 Executable" {
                refactor_opts = component "RefactorOptionsParser" "Parses refactoring commands, recipes, and target selectors." "C++17 / cxxopts"
                refactor_driver = component "RefactorDriver" "Coordinates project-wide AST surgeries and commits atomic multi-file edits." "C++17"
            }

            // Orchestration Layer
            libxe_cmake_checker_rule_engine = container "libxe-cmake-checker-rule-engine" "Unified rule orchestration, execution runner, and fixit conflict resolution." "C++17 Static Library" {
                rule_registry = component "RuleRegistry" "Aggregates rules from both YAML DSL and ChaiScript, managing rule IDs and severities." "C++17"
                check_runner = component "CheckRunner" "Coordinates AST/graph traversal, executes rule checks, and aggregates findings." "C++17"
                fix_resolver = component "FixConflictResolver" "Validates edit compatibility, sorts in reverse-offset order, and resolves overlaps." "C++17"
            }

            // Rule Definitions Layers
            libxe_cmake_checker_dsl = container "libxe-cmake-checker-dsl" "Declarative YAML rule parsing, predicate evaluation, and fix template application." "C++17 Static Library" {
                yaml_loader = component "YamlRuleLoader" "Parses declarative YAML rule definitions using rapidyaml." "C++17 / rapidyaml"
                dsl_evaluator = component "DslPredicateEvaluator" "Evaluates declarative boolean and collection match conditions over CST and graph." "C++17"
                fix_templates = component "FixTemplateEngine" "Instantiates declarative fix templates into concrete TextEdits." "C++17"
            }

            libxe_cmake_checker_script = container "libxe-cmake-checker-script" "Embedded ChaiScript engine and primitive bindings for procedural rules and refactoring." "C++17 Static Library" {
                script_facade = component "ScriptEngineFacade" "Opaque Pimpl facade embedding ChaiScript 6.1.0, isolating compilation overhead." "C++17 / ChaiScript"
                script_bindings = component "ScriptBindings" "Binds low-level CST nodes, graph edges, findings, and fixits into ChaiScript." "C++17"
                script_loader = component "ScriptRuleLoader" "Discovers and loads custom *.chai procedural rule scripts." "C++17"
            }

            // Analysis Layer
            libxe_cmake_checker_analysis = container "libxe-cmake-checker-analysis" "Semantic model and generic directed dependency graph." "C++17 Static Library" {
                semantic_model = component "SemanticModel" "Binds build tree facts (File API/Trace) to listfiles, targets, and compiler options." "C++17"
                directed_graph = component "DirectedDependencyGraph" "Generic directed multigraph storing targets and incoming/outgoing dependency edges." "C++17"
            }

            // I/O & Storage Layer
            libxe_cmake_checker_io = container "libxe-cmake-checker-io" "Filesystem abstractions, project listfile loading, trace/file-API reading, and surgical file write-back." "C++17 Static Library" {
                file_system = component "FileSystem" "Abstract IFileSystem with NativeFileSystem and InMemoryFileSystem implementations." "C++17"
                project_loader = component "ProjectLoader" "Discovers and reads CMakeLists.txt and cmake modules into memory." "C++17"
                build_tree_reader = component "BuildTreeReader" "Parses CMake File API (codemodel-v2) and trace (json-v1) records." "C++17 / nlohmann_json"
                sync_writer = component "SyncWriter" "Applies WorkspaceEdit minimal diffs safely to disk or in-memory filesystem." "C++17"
            }

            // Core Primitives Layer
            libxe_cmake_checker_core = container "libxe-cmake-checker-core" "Concrete Syntax Tree with trivia, lexer, low-level query primitives, TextEdit, and WorkspaceEdit." "C++17 Static Library" {
                cst = component "ConcreteSyntaxTree" "Lossless syntax tree (Listfile, Command, Argument, Trivia) with byte-exact spans." "C++17"
                lexer = component "Lexer" "Lossless tokenizer producing tokens and trivia with exact source coordinates." "C++17"
                query_prims = component "QueryPrimitives" "Orthogonal node navigation, string matching, and regex operations." "C++17"
                mutation_engine = component "MutationEngine" "TextEdit, WorkspaceEdit, and surgical minimal-diff text splicer." "C++17"
                refactor_prims = component "RefactoringPrimitives" "High-level structural transformation building blocks (target rename, function extract/inline)." "C++17"
            }

            // Shared Testing Library
            libxe_cmake_checker_testing = container "libxe-cmake-checker-testing" "Shared testing facilities, synthetic generators, and property assertions." "C++17 Static Library" {
                cst_generator = component "CstSyntheticGenerator" "Deterministic random builder for CST nodes using Catch2 seed." "C++17"
                graph_generator = component "GraphSyntheticGenerator" "Deterministic random builder for dependency graphs using Catch2 seed." "C++17"
                property_assertions = component "PropertyAssertions" "Reusable Catch2 property assertions (requireLosslessRoundTrip, requireValidSpans)." "C++17 / Catch2"
            }

            // End-to-End In-Memory Integration Test Target
            xe_cmake_checker_e2e_test = container "xe-cmake-checker-e2e-test" "End-to-end in-memory synthetic fixture integration test suite for DSL and ChaiScript." "C++17 Executable / Catch2" {
                synthetic_proj_gen = component "SyntheticProjectGenerator" "Generates parametric in-memory CMake project fixtures with diverse valid syntax styles." "C++17"
                dsl_e2e_test = component "DslEndToEndTest" "Validates full-stack parsing, graph, DSL rule execution, fixits, reparsing, and clean second pass." "C++17 / Catch2"
                chai_e2e_test = component "ChaiScriptEndToEndTest" "Validates full-stack parsing, graph, ChaiScript execution, fixits, reparsing, and clean second pass." "C++17 / Catch2"
            }
        }

        // Top-Level Relationships
        developer -> xeCmake.xe_cmake_checker "Invokes CLI to check style, view diffs, and apply fixits" "CLI"
        ci -> xeCmake.xe_cmake_checker "Runs automated style verification during CI passes" "CLI"
        developer -> xeCmake.xe_cmake_refactor "Runs structural refactorings and migrations" "CLI"

        // CLI Internal Relationships
        xeCmake.xe_cmake_checker.cli_opts -> xeCmake.xe_cmake_checker.checker_driver "Provides parsed options"
        xeCmake.xe_cmake_checker.checker_driver -> xeCmake.xe_cmake_checker.diag_reporter "Dispatches findings for display"

        // Container-Level Cross-References
        xeCmake.xe_cmake_checker -> xeCmake.libxe_cmake_checker_rule_engine "Invokes CheckRunner to execute rules" "C++ API"
        xeCmake.xe_cmake_checker -> xeCmake.libxe_cmake_checker_io "Loads project listfiles and writes diffs" "C++ API"
        xeCmake.xe_cmake_checker -> xeCmake.libxe_cmake_checker_core "Inspects findings and spans" "C++ API"

        xeCmake.xe_cmake_refactor -> xeCmake.libxe_cmake_checker_rule_engine "Orchestrates refactoring recipes" "C++ API"
        xeCmake.xe_cmake_refactor -> xeCmake.libxe_cmake_checker_core "Invokes structural refactoring building blocks" "C++ API"
        xeCmake.xe_cmake_refactor -> xeCmake.libxe_cmake_checker_io "Loads project and commits transactions" "C++ API"

        xeCmake.libxe_cmake_checker_rule_engine -> xeCmake.libxe_cmake_checker_dsl "Dispatches declarative checks" "C++ API"
        xeCmake.libxe_cmake_checker_rule_engine -> xeCmake.libxe_cmake_checker_script "Dispatches procedural script checks" "C++ API"
        xeCmake.libxe_cmake_checker_rule_engine -> xeCmake.libxe_cmake_checker_analysis "Queries semantic model and graph" "C++ API"
        xeCmake.libxe_cmake_checker_rule_engine -> xeCmake.libxe_cmake_checker_core "Inspects AST and collects edits" "C++ API"
        xeCmake.libxe_cmake_checker_rule_engine -> xeCmake.libxe_cmake_checker_io "Passes WorkspaceEdit for write-back" "C++ API"

        xeCmake.libxe_cmake_checker_dsl -> xeCmake.libxe_cmake_checker_core "Navigates CST nodes and generates edits" "C++ API"
        xeCmake.libxe_cmake_checker_dsl -> xeCmake.libxe_cmake_checker_analysis "Queries dependency relationships" "C++ API"

        xeCmake.libxe_cmake_checker_script -> xeCmake.libxe_cmake_checker_core "Binds CST nodes, spans, and edits" "C++ API"
        xeCmake.libxe_cmake_checker_script -> xeCmake.libxe_cmake_checker_analysis "Binds directed graph edges" "C++ API"

        xeCmake.libxe_cmake_checker_analysis -> xeCmake.libxe_cmake_checker_core "Indexes CST nodes into semantic graph" "C++ API"

        xeCmake.libxe_cmake_checker_io -> xeCmake.libxe_cmake_checker_core "Populates CST and executes WorkspaceEdit" "C++ API"
        xeCmake.libxe_cmake_checker_io -> targetProject "Reads CMakeLists.txt and writes surgical diffs" "File I/O"
        xeCmake.libxe_cmake_checker_io -> cmakeBuild "Reads codemodel-v2 and trace JSON" "File I/O"

        // Test Framework Relationships
        xeCmake.libxe_cmake_checker_testing -> xeCmake.libxe_cmake_checker_core "Generates synthetic CSTs and tests roundtrips" "C++ API"
        xeCmake.libxe_cmake_checker_testing -> xeCmake.libxe_cmake_checker_analysis "Generates synthetic dependency graphs" "C++ API"
        xeCmake.libxe_cmake_checker_testing -> xeCmake.libxe_cmake_checker_io "Provides InMemoryFileSystem for tests" "C++ API"

        // End-to-End Test Suite Relationships
        xeCmake.xe_cmake_checker_e2e_test -> xeCmake.libxe_cmake_checker_rule_engine "Executes unified rule checks" "C++ API"
        xeCmake.xe_cmake_checker_e2e_test -> xeCmake.libxe_cmake_checker_dsl "Evaluates YAML rules and fix templates" "C++ API"
        xeCmake.xe_cmake_checker_e2e_test -> xeCmake.libxe_cmake_checker_script "Executes ChaiScript rules and procedural fixits" "C++ API"
        xeCmake.xe_cmake_checker_e2e_test -> xeCmake.libxe_cmake_checker_analysis "Builds and queries semantic graph" "C++ API"
        xeCmake.xe_cmake_checker_e2e_test -> xeCmake.libxe_cmake_checker_io "Operates on InMemoryFileSystem" "C++ API"
        xeCmake.xe_cmake_checker_e2e_test -> xeCmake.libxe_cmake_checker_core "Parses CST and splices mutations" "C++ API"
        xeCmake.xe_cmake_checker_e2e_test -> xeCmake.libxe_cmake_checker_testing "Uses shared Catch2 assertion utilities" "C++ API"
    }

    views {
        systemContext xeCmake "SystemContext" "System context diagram for the xe-cmake toolsuite." {
            include *
            autoLayout lr
        }

        container xeCmake "Containers" "Container diagram showing frontends and static libraries of xe-cmake." {
            include *
            autoLayout tb
        }

        component xeCmake.xe_cmake_checker "CheckerCliComponents" "Components inside the xe-cmake-checker CLI frontend." {
            include *
            autoLayout lr
        }

        component xeCmake.libxe_cmake_checker_rule_engine "RuleEngineComponents" "Components inside libxe-cmake-checker-rule-engine." {
            include *
            autoLayout lr
        }

        component xeCmake.libxe_cmake_checker_dsl "DslComponents" "Components inside libxe-cmake-checker-dsl." {
            include *
            autoLayout lr
        }

        component xeCmake.libxe_cmake_checker_script "ScriptComponents" "Components inside libxe-cmake-checker-script." {
            include *
            autoLayout lr
        }

        component xeCmake.libxe_cmake_checker_analysis "AnalysisComponents" "Components inside libxe-cmake-checker-analysis." {
            include *
            autoLayout lr
        }

        component xeCmake.libxe_cmake_checker_io "IoComponents" "Components inside libxe-cmake-checker-io." {
            include *
            autoLayout lr
        }

        component xeCmake.libxe_cmake_checker_core "CoreComponents" "Components inside libxe-cmake-checker-core." {
            include *
            autoLayout lr
        }

        component xeCmake.libxe_cmake_checker_testing "TestingComponents" "Components inside libxe-cmake-checker-testing." {
            include *
            autoLayout lr
        }

        component xeCmake.xe_cmake_checker_e2e_test "E2eTestComponents" "Components inside xe-cmake-checker-e2e-test." {
            include *
            autoLayout lr
        }

        styles {
            element "Person" {
                shape Person
                background #08427B
                color #ffffff
            }
            element "Software System" {
                background #1168BD
                color #ffffff
            }
            element "Container" {
                background #438DD5
                color #ffffff
            }
            element "Component" {
                background #85BBF0
                color #000000
            }
            element "External" {
                background #999999
                color #ffffff
            }
            element "Element" {
                strokeWidth 2
                shape roundedbox
            }
            relationship "Relationship" {
                thickness 2
            }
        }
    }

    configuration {
        scope softwaresystem
    }
}
