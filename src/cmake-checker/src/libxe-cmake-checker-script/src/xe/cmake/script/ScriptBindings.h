#ifndef XE_CMAKE_SCRIPT_SCRIPT_BINDINGS_H
#define XE_CMAKE_SCRIPT_SCRIPT_BINDINGS_H

#include "ScriptEngineFacade.h"

#include "xe/cmake/analysis/DirectedDependencyGraph.h"
#include "xe/cmake/core/ConcreteSyntaxTree.h"
#include "xe/cmake/core/MutationEngine.h"

#include <string>
#include <vector>

namespace xe::cmake::script {

    // Copyable proxy objects bound into ChaiScript. They reference the owning
    // CST/graph (which must outlive the script execution) and expose the rule
    // authoring API described in the plan.

    struct ScriptSourceSpan {
        xe::cmake::core::SourceSpan span;

        int start_offset() const {
            return static_cast<int>(span.start_offset);
        }
        int end_offset() const {
            return static_cast<int>(span.end_offset);
        }
        int start_line() const {
            return static_cast<int>(span.start_line);
        }
        int start_column() const {
            return static_cast<int>(span.start_column);
        }
        int end_line() const {
            return static_cast<int>(span.end_line);
        }
        int end_column() const {
            return static_cast<int>(span.end_column);
        }
    };

    struct ScriptTextEdit {
        xe::cmake::core::TextEdit edit;

        const xe::cmake::core::TextEdit &edit_value() const {
            return edit;
        }
    };

    struct ScriptFix {
        xe::cmake::core::Fix fix;

        ScriptFix() = default;

        explicit ScriptFix(const std::string &description) : fix(description) {
        }

        std::string description() const {
            return fix.description;
        }

        void add_edit(ScriptTextEdit edit) {
            fix.add_edit(edit.edit);
        }

        std::vector<ScriptTextEdit> edits() const {
            std::vector<ScriptTextEdit> result;
            for (const xe::cmake::core::TextEdit &edit : fix.edits) {
                ScriptTextEdit proxy;
                proxy.edit = edit;
                result.push_back(proxy);
            }
            return result;
        }
    };

    struct ScriptFinding {
        xe::cmake::core::Finding finding;

        ScriptFinding() = default;

        ScriptFinding(const std::string &rule_id, const std::string &severity, const std::string &message, ScriptSourceSpan span) {
            finding.rule_id = rule_id;
            finding.severity = severity == "Error" ? xe::cmake::core::Severity::Error : (severity == "Info" ? xe::cmake::core::Severity::Info : xe::cmake::core::Severity::Warn);
            finding.message = message;
            finding.span = span.span;
        }

        ScriptFinding(const std::string &rule_id, const std::string &severity, const std::string &message, ScriptSourceSpan span, ScriptFix fix) {
            finding.rule_id = rule_id;
            finding.severity = severity == "Error" ? xe::cmake::core::Severity::Error : (severity == "Info" ? xe::cmake::core::Severity::Info : xe::cmake::core::Severity::Warn);
            finding.message = message;
            finding.span = span.span;
            finding.fix = fix.fix;
        }

        std::string rule_id() const {
            return finding.rule_id;
        }
        std::string message() const {
            return finding.message;
        }
        ScriptSourceSpan span() const {
            ScriptSourceSpan proxy;
            proxy.span = finding.span;
            return proxy;
        }
        bool has_fix() const {
            return finding.has_fix();
        }
        ScriptFix fix() const {
            ScriptFix proxy;
            proxy.fix = *finding.fix;
            return proxy;
        }
    };

    // Execution context handed to every rule hook.
    struct ScriptContext {
        std::vector<xe::cmake::core::Finding> *findings = nullptr;
        std::string file_path;

        void report(ScriptFinding finding) {
            if (findings == nullptr) {
                return;
            }
            finding.finding.file_path = file_path;
            findings->push_back(finding.finding);
        }

        std::string current_file_path() const {
            return file_path;
        }
    };

    struct ScriptArgument {
        const xe::cmake::core::ArgumentNode *node = nullptr;

        std::string text() const {
            return node != nullptr ? node->text : std::string();
        }
        std::string quote_kind() const {
            if (node == nullptr) {
                return "raw";
            }
            if (node->is_quoted()) {
                return "quoted";
            }
            if (node->is_bracket()) {
                return "bracket";
            }
            return "raw";
        }
        ScriptSourceSpan span() const {
            ScriptSourceSpan proxy;
            proxy.span = node != nullptr ? node->span : xe::cmake::core::SourceSpan::invalid();
            return proxy;
        }
        int index() const {
            return node != nullptr ? static_cast<int>(node->index) : 0;
        }
    };

    struct ScriptCommand {
        const xe::cmake::core::CommandNode *node = nullptr;

        std::string name() const {
            return node != nullptr ? node->name : std::string();
        }
        int argument_count() const {
            return node != nullptr ? static_cast<int>(node->argument_count()) : 0;
        }
        ScriptArgument argument(int index) const {
            ScriptArgument proxy;
            proxy.node = (node != nullptr && index >= 0 && static_cast<std::size_t>(index) < node->argument_count()) ? &node->argument(static_cast<std::size_t>(index)) : nullptr;
            return proxy;
        }
        std::vector<ScriptArgument> arguments() const {
            std::vector<ScriptArgument> result;
            if (node == nullptr) {
                return result;
            }
            for (std::size_t i = 0; i < node->argument_count(); ++i) {
                ScriptArgument proxy;
                proxy.node = &node->argument(i);
                result.push_back(proxy);
            }
            return result;
        }
        std::string file_path() const {
            return node != nullptr ? node->file_path : std::string();
        }
        ScriptSourceSpan span() const {
            ScriptSourceSpan proxy;
            proxy.span = node != nullptr ? node->span : xe::cmake::core::SourceSpan::invalid();
            return proxy;
        }
    };

    struct ScriptFile {
        const xe::cmake::core::ConcreteSyntaxTree *tree = nullptr;

        std::string path() const {
            return tree != nullptr ? tree->file_path() : std::string();
        }
        std::vector<ScriptCommand> commands() const {
            std::vector<ScriptCommand> result;
            if (tree == nullptr) {
                return result;
            }
            for (const xe::cmake::core::CommandNode *command : tree->commands()) {
                ScriptCommand proxy;
                proxy.node = command;
                result.push_back(proxy);
            }
            return result;
        }
        std::vector<ScriptCommand> find_commands(const std::string &name) const {
            std::vector<ScriptCommand> result;
            if (tree == nullptr) {
                return result;
            }
            for (const xe::cmake::core::CommandNode *command : tree->commands()) {
                if (command->name == name) {
                    ScriptCommand proxy;
                    proxy.node = command;
                    result.push_back(proxy);
                }
            }
            return result;
        }
    };

    struct ScriptGraphEdge {
        xe::cmake::analysis::GraphEdge edge;

        std::string source() const {
            return edge.source;
        }
        std::string target() const {
            return edge.target;
        }
        std::string attribute(const std::string &key) const {
            return std::string(edge.attribute(key));
        }
    };

    struct ScriptGraph {
        const xe::cmake::analysis::DirectedDependencyGraph *graph = nullptr;

        std::vector<std::string> node_ids() const {
            return graph != nullptr ? graph->node_ids() : std::vector<std::string>();
        }
        std::vector<ScriptGraphEdge> incoming_edges(const std::string &node) const {
            std::vector<ScriptGraphEdge> result;
            if (graph == nullptr) {
                return result;
            }
            for (const xe::cmake::analysis::GraphEdge &edge : graph->incoming_edges(node)) {
                ScriptGraphEdge proxy;
                proxy.edge = edge;
                result.push_back(proxy);
            }
            return result;
        }
        std::vector<ScriptGraphEdge> outgoing_edges(const std::string &node) const {
            std::vector<ScriptGraphEdge> result;
            if (graph == nullptr) {
                return result;
            }
            for (const xe::cmake::analysis::GraphEdge &edge : graph->outgoing_edges(node)) {
                ScriptGraphEdge proxy;
                proxy.edge = edge;
                result.push_back(proxy);
            }
            return result;
        }
        bool has_target(const std::string &node) const {
            return graph != nullptr && graph->has_target(node);
        }
        bool has_edge(const std::string &source, const std::string &target) const {
            return graph != nullptr && graph->has_edge(source, target);
        }
    };

    // Constant objects exposed as dot-access namespaces in ChaiScript
    // (Severity.Warn, QuoteKind.Quoted, ...).
    struct ScriptSeverity {
        const std::string Error = "Error";
        const std::string Warn = "Warn";
        const std::string Info = "Info";
    };

    struct ScriptQuoteKind {
        const std::string Raw = "Raw";
        const std::string Quoted = "Quoted";
        const std::string Bracket = "Bracket";
    };

    // Marker type for the TextEdit factory namespace object.
    struct ScriptTextEditFactory {};

    // The null sentinel used by rule scripts (null comparisons).
    struct ScriptNull {};

    // Registers all wrapper types and primitives into the script engine.
    class ScriptBindings {
    public:
        static void install(ScriptEngineFacade &engine);
    };

} // namespace xe::cmake::script

#endif // XE_CMAKE_SCRIPT_SCRIPT_BINDINGS_H