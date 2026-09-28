#include "ScriptBindings.h"

#include "xe/cmake/core/QueryPrimitives.h"

#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_stdlib.hpp>

namespace xe::cmake::script {

    void ScriptBindings::install(ScriptEngineFacade &engine) {
        engine.register_bindings([](void *raw) {
            chaiscript::ChaiScript *chai = static_cast<chaiscript::ChaiScript *>(raw);

            chai->add(chaiscript::user_type<ScriptSourceSpan>(), "SourceSpan");
            chai->add(chaiscript::constructor<ScriptSourceSpan()>(), "SourceSpan");
            chai->add(chaiscript::fun(&ScriptSourceSpan::start_offset), "start_offset");
            chai->add(chaiscript::fun(&ScriptSourceSpan::end_offset), "end_offset");
            chai->add(chaiscript::fun(&ScriptSourceSpan::start_line), "start_line");
            chai->add(chaiscript::fun(&ScriptSourceSpan::start_column), "start_column");
            chai->add(chaiscript::fun(&ScriptSourceSpan::end_line), "end_line");
            chai->add(chaiscript::fun(&ScriptSourceSpan::end_column), "end_column");

            chai->add(chaiscript::user_type<ScriptTextEdit>(), "TextEditValue");
            chai->add(chaiscript::fun(&ScriptTextEdit::edit_value), "edit");

            // TextEdit.replace(...), TextEdit.insert_before(...), ... are exposed
            // as member functions of the TextEdit factory namespace object.
            chai->add(chaiscript::user_type<ScriptTextEditFactory>(), "__TextEditFactory");
            chai->add(
                chaiscript::fun([](ScriptTextEditFactory &, ScriptSourceSpan span, const std::string &text) {
                    ScriptTextEdit result;
                    result.edit = xe::cmake::core::TextEdit::replace(span.span, text);
                    return result;
                }),
                "replace"
            );
            chai->add(
                chaiscript::fun([](ScriptTextEditFactory &, int offset, const std::string &text) {
                    ScriptTextEdit result;
                    result.edit = xe::cmake::core::TextEdit::insert_before(static_cast<std::size_t>(offset), text);
                    return result;
                }),
                "insert_before"
            );
            chai->add(
                chaiscript::fun([](ScriptTextEditFactory &, int offset, const std::string &text) {
                    ScriptTextEdit result;
                    result.edit = xe::cmake::core::TextEdit::insert_after(static_cast<std::size_t>(offset), text);
                    return result;
                }),
                "insert_after"
            );
            chai->add(
                chaiscript::fun([](ScriptTextEditFactory &, ScriptSourceSpan span) {
                    ScriptTextEdit result;
                    result.edit = xe::cmake::core::TextEdit::remove(span.span);
                    return result;
                }),
                "remove"
            );
            ScriptTextEditFactory text_edit_factory;
            chai->add_global(chaiscript::Boxed_Value(std::ref(text_edit_factory)), "TextEdit");

            chai->add(chaiscript::user_type<ScriptFix>(), "Fix");
            chai->add(chaiscript::constructor<ScriptFix()>(), "Fix");
            chai->add(chaiscript::constructor<ScriptFix(const std::string &)>(), "Fix");
            chai->add(chaiscript::fun(&ScriptFix::description), "description");
            chai->add(chaiscript::fun(&ScriptFix::add_edit), "add_edit");
            chai->add(chaiscript::fun(&ScriptFix::edits), "edits");

            chai->add(chaiscript::user_type<ScriptFinding>(), "Finding");
            chai->add(chaiscript::constructor<ScriptFinding()>(), "Finding");
            chai->add(chaiscript::constructor<ScriptFinding(const std::string &, const std::string &, const std::string &, ScriptSourceSpan)>(), "Finding");
            chai->add(chaiscript::constructor<ScriptFinding(const std::string &, const std::string &, const std::string &, ScriptSourceSpan, ScriptFix)>(), "Finding");
            chai->add(chaiscript::fun(&ScriptFinding::rule_id), "rule_id");
            chai->add(chaiscript::fun(&ScriptFinding::message), "message");
            chai->add(chaiscript::fun(&ScriptFinding::span), "span");
            chai->add(chaiscript::fun(&ScriptFinding::has_fix), "has_fix");
            chai->add(chaiscript::fun(&ScriptFinding::fix), "fix");

            chai->add(chaiscript::user_type<ScriptContext>(), "ExecutionContext");
            chai->add(chaiscript::constructor<ScriptContext()>(), "ExecutionContext");
            chai->add(chaiscript::fun(&ScriptContext::report), "report");
            chai->add(chaiscript::fun(&ScriptContext::current_file_path), "file_path");

            chai->add(chaiscript::user_type<ScriptArgument>(), "ArgumentNode");
            chai->add(chaiscript::fun(&ScriptArgument::text), "text");
            chai->add(chaiscript::fun(&ScriptArgument::quote_kind), "quote_kind");
            chai->add(chaiscript::fun(&ScriptArgument::span), "span");
            chai->add(chaiscript::fun(&ScriptArgument::index), "index");

            chai->add(chaiscript::user_type<ScriptCommand>(), "CommandNode");
            chai->add(chaiscript::fun(&ScriptCommand::name), "name");
            chai->add(chaiscript::fun(&ScriptCommand::argument_count), "argument_count");
            chai->add(chaiscript::fun(&ScriptCommand::argument), "argument");
            chai->add(chaiscript::fun(&ScriptCommand::arguments), "arguments");
            chai->add(chaiscript::fun(&ScriptCommand::file_path), "file_path");
            chai->add(chaiscript::fun(&ScriptCommand::span), "span");

            chai->add(chaiscript::user_type<ScriptFile>(), "ListfileNode");
            chai->add(chaiscript::fun(&ScriptFile::path), "path");
            chai->add(chaiscript::fun(&ScriptFile::commands), "commands");
            chai->add(chaiscript::fun(&ScriptFile::find_commands), "find_commands");

            chai->add(chaiscript::user_type<ScriptGraphEdge>(), "GraphEdge");
            chai->add(chaiscript::fun(&ScriptGraphEdge::source), "source");
            chai->add(chaiscript::fun(&ScriptGraphEdge::target), "target");
            chai->add(chaiscript::fun(&ScriptGraphEdge::attribute), "attribute");

            chai->add(chaiscript::user_type<ScriptGraph>(), "DirectedDependencyGraph");
            chai->add(chaiscript::fun(&ScriptGraph::node_ids), "node_ids");
            chai->add(chaiscript::fun(&ScriptGraph::incoming_edges), "incoming_edges");
            chai->add(chaiscript::fun(&ScriptGraph::outgoing_edges), "outgoing_edges");
            chai->add(chaiscript::fun(&ScriptGraph::has_target), "has_target");
            chai->add(chaiscript::fun(&ScriptGraph::has_edge), "has_edge");

            // Severity and QuoteKind are exposed as constant objects with member
            // accessors so rules can write Severity.Warn, QuoteKind.Quoted, etc.
            chai->add(chaiscript::user_type<ScriptSeverity>(), "Severity");
            chai->add(chaiscript::fun(&ScriptSeverity::Error), "Error");
            chai->add(chaiscript::fun(&ScriptSeverity::Warn), "Warn");
            chai->add(chaiscript::fun(&ScriptSeverity::Info), "Info");
            chai->add_global_const(chaiscript::const_var(ScriptSeverity()), "Severity");

            chai->add(chaiscript::user_type<ScriptQuoteKind>(), "QuoteKind");
            chai->add(chaiscript::fun(&ScriptQuoteKind::Raw), "Raw");
            chai->add(chaiscript::fun(&ScriptQuoteKind::Quoted), "Quoted");
            chai->add(chaiscript::fun(&ScriptQuoteKind::Bracket), "Bracket");
            chai->add_global_const(chaiscript::const_var(ScriptQuoteKind()), "QuoteKind");

            // Assignment operators for the proxy value types (scripts reassign
            // variables captured from a loop, e.g. `var span = SourceSpan();
            // span = cmd.span();`).
            chai->add(
                chaiscript::fun([](ScriptSourceSpan &a, const ScriptSourceSpan &b) -> ScriptSourceSpan & {
                    a.span = b.span;
                    return a;
                }),
                "="
            );
            chai->add(
                chaiscript::fun([](ScriptTextEdit &a, const ScriptTextEdit &b) -> ScriptTextEdit & {
                    a.edit = b.edit;
                    return a;
                }),
                "="
            );
            chai->add(
                chaiscript::fun([](ScriptFix &a, const ScriptFix &b) -> ScriptFix & {
                    a.fix = b.fix;
                    return a;
                }),
                "="
            );
            chai->add(
                chaiscript::fun([](ScriptFinding &a, const ScriptFinding &b) -> ScriptFinding & {
                    a.finding = b.finding;
                    return a;
                }),
                "="
            );

            // ChaiScript has no null literal; provide a global null object plus == / !=
            // comparisons against every proxy type.
            chai->add(chaiscript::user_type<ScriptNull>(), "null");
            chai->add(chaiscript::constructor<ScriptNull()>(), "null");
            chai->add(chaiscript::fun([](const ScriptNull &n) { return n; }), "clone");
            chai->add_global(chaiscript::Boxed_Value(ScriptNull()), "null");
            chai->add(chaiscript::fun([](const ScriptCommand &, const ScriptNull &) { return false; }), "==");
            chai->add(chaiscript::fun([](const ScriptNull &, const ScriptCommand &) { return false; }), "==");
            chai->add(chaiscript::fun([](const ScriptArgument &, const ScriptNull &) { return false; }), "==");
            chai->add(chaiscript::fun([](const ScriptNull &, const ScriptArgument &) { return false; }), "==");
            chai->add(chaiscript::fun([](const ScriptFile &, const ScriptNull &) { return false; }), "==");
            chai->add(chaiscript::fun([](const ScriptNull &, const ScriptFile &) { return false; }), "==");
            chai->add(chaiscript::fun([](const ScriptFix &, const ScriptNull &) { return false; }), "==");
            chai->add(chaiscript::fun([](const ScriptNull &, const ScriptFix &) { return false; }), "==");
            chai->add(chaiscript::fun([](const ScriptNull &, const ScriptNull &) { return true; }), "==");
            chai->add(chaiscript::fun([](const ScriptCommand &, const ScriptNull &) { return true; }), "!=");
            chai->add(chaiscript::fun([](const ScriptNull &, const ScriptCommand &) { return true; }), "!=");
            chai->add(chaiscript::fun([](const ScriptArgument &, const ScriptNull &) { return true; }), "!=");
            chai->add(chaiscript::fun([](const ScriptNull &, const ScriptArgument &) { return true; }), "!=");
            chai->add(chaiscript::fun([](const ScriptFile &, const ScriptNull &) { return true; }), "!=");
            chai->add(chaiscript::fun([](const ScriptNull &, const ScriptFile &) { return true; }), "!=");
            chai->add(chaiscript::fun([](const ScriptFix &, const ScriptNull &) { return true; }), "!=");
            chai->add(chaiscript::fun([](const ScriptNull &, const ScriptFix &) { return true; }), "!=");
            chai->add(chaiscript::fun([](const ScriptNull &, const ScriptNull &) { return false; }), "!=");

            // Sequence indexing and length operators for the proxy vectors used in
            // rule scripts (file.commands(), cmd.arguments(), graph edges, ...).
            chai->add(chaiscript::fun([](const std::vector<ScriptCommand> &v, int i) { return v[i]; }), "[]");
            chai->add(chaiscript::fun([](const std::vector<ScriptCommand> &v) { return v.size(); }), "size");
            chai->add(chaiscript::fun([](const std::vector<ScriptArgument> &v, int i) { return v[i]; }), "[]");
            chai->add(chaiscript::fun([](const std::vector<ScriptArgument> &v) { return v.size(); }), "size");
            chai->add(chaiscript::fun([](const std::vector<ScriptGraphEdge> &v, int i) { return v[i]; }), "[]");
            chai->add(chaiscript::fun([](const std::vector<ScriptGraphEdge> &v) { return v.size(); }), "size");
            chai->add(chaiscript::fun([](const std::vector<std::string> &v, int i) { return v[i]; }), "[]");
            chai->add(chaiscript::fun([](const std::vector<std::string> &v) { return v.size(); }), "size");
            chai->add(chaiscript::fun([](const std::vector<ScriptTextEdit> &v, int i) { return v[i]; }), "[]");
            chai->add(chaiscript::fun([](const std::vector<ScriptTextEdit> &v) { return v.size(); }), "size");
            chai->add(chaiscript::fun([](const std::vector<ScriptFile> &v, int i) { return v[i]; }), "[]");
            chai->add(chaiscript::fun([](const std::vector<ScriptFile> &v) { return v.size(); }), "size");

            // Ranged-for support for the proxy vector types (used by rule hooks).
            // ChaiScript's ranged-for protocol calls range/empty/front/pop_front.
            chai->add(chaiscript::fun([](std::vector<ScriptCommand> &v) -> std::vector<ScriptCommand> & { return v; }), "range");
            chai->add(chaiscript::fun([](std::vector<ScriptArgument> &v) -> std::vector<ScriptArgument> & { return v; }), "range");
            chai->add(chaiscript::fun([](std::vector<ScriptGraphEdge> &v) -> std::vector<ScriptGraphEdge> & { return v; }), "range");
            chai->add(chaiscript::fun([](std::vector<std::string> &v) -> std::vector<std::string> & { return v; }), "range");
            chai->add(chaiscript::fun([](std::vector<ScriptFile> &v) -> std::vector<ScriptFile> & { return v; }), "range");
            chai->add(chaiscript::fun([](const std::vector<ScriptCommand> &v) { return v.empty(); }), "empty");
            chai->add(chaiscript::fun([](const std::vector<ScriptArgument> &v) { return v.empty(); }), "empty");
            chai->add(chaiscript::fun([](const std::vector<ScriptGraphEdge> &v) { return v.empty(); }), "empty");
            chai->add(chaiscript::fun([](const std::vector<std::string> &v) { return v.empty(); }), "empty");
            chai->add(chaiscript::fun([](const std::vector<ScriptFile> &v) { return v.empty(); }), "empty");
            chai->add(chaiscript::fun([](const std::vector<ScriptCommand> &v) { return v.front(); }), "front");
            chai->add(chaiscript::fun([](const std::vector<ScriptArgument> &v) { return v.front(); }), "front");
            chai->add(chaiscript::fun([](const std::vector<ScriptGraphEdge> &v) { return v.front(); }), "front");
            chai->add(chaiscript::fun([](const std::vector<std::string> &v) { return v.front(); }), "front");
            chai->add(chaiscript::fun([](const std::vector<ScriptFile> &v) { return v.front(); }), "front");
            chai->add(chaiscript::fun([](std::vector<ScriptCommand> &v) { v.erase(v.begin()); }), "pop_front");
            chai->add(chaiscript::fun([](std::vector<ScriptArgument> &v) { v.erase(v.begin()); }), "pop_front");
            chai->add(chaiscript::fun([](std::vector<ScriptGraphEdge> &v) { v.erase(v.begin()); }), "pop_front");
            chai->add(chaiscript::fun([](std::vector<std::string> &v) { v.erase(v.begin()); }), "pop_front");
            chai->add(chaiscript::fun([](std::vector<ScriptFile> &v) { v.erase(v.begin()); }), "pop_front");

            chai->add(chaiscript::fun([](const std::string &a, const std::string &b) { return xe::cmake::core::StringPrimitives::regex_match(a, b); }), "regex_match");
            chai->add(chaiscript::fun([](const std::string &a, const std::string &b) { return xe::cmake::core::StringPrimitives::regex_search(a, b); }), "regex_search");
            chai->add(chaiscript::fun([](const std::string &a, const std::string &b) { return xe::cmake::core::StringPrimitives::str_contains(a, b); }), "str_contains");
            chai->add(chaiscript::fun([](const std::string &a, const std::string &b) { return xe::cmake::core::StringPrimitives::str_starts_with(a, b); }), "str_starts_with");
            chai->add(chaiscript::fun([](const std::string &a, const std::string &b) { return xe::cmake::core::StringPrimitives::str_ends_with(a, b); }), "str_ends_with");
            chai->add(chaiscript::fun([](const std::string &a, const std::string &b) { return xe::cmake::core::StringPrimitives::str_split(a, b); }), "str_split");

            chai->add(chaiscript::fun([](long long value) { return std::to_string(value); }), "to_string");
            chai->add(chaiscript::fun([](int value) { return std::to_string(value); }), "to_string");
            chai->add(chaiscript::fun([](const std::string &value) { return value; }), "to_string");
        });
    }

} // namespace xe::cmake::script