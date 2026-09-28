#include "ScriptRuleLoader.h"

#include <chaiscript/chaiscript.hpp>

namespace xe::cmake::script {

    namespace {

        void set_global(chaiscript::ChaiScript &chai, const std::string &name, chaiscript::Boxed_Value value) {
            chai.set_global(value, name);
        }

    } // namespace

    ScriptRuleLoader::ScriptRuleLoader() {
        ScriptBindings::install(facade_);
    }

    void ScriptRuleLoader::load(std::string_view source) {
        load_script(source);
    }

    void ScriptRuleLoader::load_script(std::string_view source) {
        facade_.eval(source);
        HookSet hooks;
        hooks.has_check_file = facade_.has_function("check_file");
        hooks.has_check_command = facade_.has_function("check_command");
        hooks.has_check_project = facade_.has_function("check_project");
        hooks_.push_back(hooks);
    }

    std::vector<xe::cmake::core::Finding> ScriptRuleLoader::run_check_file(const std::vector<xe::cmake::core::ConcreteSyntaxTree> &files) const {
        std::vector<xe::cmake::core::Finding> findings;
        chaiscript::ChaiScript *chai = static_cast<chaiscript::ChaiScript *>(facade_.raw_engine());

        for (const HookSet &hooks : hooks_) {
            if (!hooks.has_check_file) {
                continue;
            }
            for (const xe::cmake::core::ConcreteSyntaxTree &tree : files) {
                ScriptContext context;
                context.findings = &findings;
                context.file_path = tree.file_path();
                ScriptFile file;
                file.tree = &tree;
                set_global(*chai, "__xe_ctx", chaiscript::Boxed_Value(context));
                set_global(*chai, "__xe_file", chaiscript::Boxed_Value(file));
                try {
                    chai->eval("check_file(__xe_ctx, __xe_file)");
                } catch (const chaiscript::exception::eval_error &error) {
                    (void)error;
                }
            }
        }
        return findings;
    }

    std::vector<xe::cmake::core::Finding> ScriptRuleLoader::run_check_command(const std::vector<xe::cmake::core::ConcreteSyntaxTree> &files) const {
        std::vector<xe::cmake::core::Finding> findings;
        chaiscript::ChaiScript *chai = static_cast<chaiscript::ChaiScript *>(facade_.raw_engine());

        for (const HookSet &hooks : hooks_) {
            if (!hooks.has_check_command) {
                continue;
            }
            for (const xe::cmake::core::ConcreteSyntaxTree &tree : files) {
                for (const xe::cmake::core::CommandNode *command : tree.commands()) {
                    ScriptContext context;
                    context.findings = &findings;
                    context.file_path = tree.file_path();
                    ScriptCommand cmd;
                    cmd.node = command;
                    set_global(*chai, "__xe_ctx", chaiscript::Boxed_Value(context));
                    set_global(*chai, "__xe_cmd", chaiscript::Boxed_Value(cmd));
                    try {
                        chai->eval("check_command(__xe_ctx, __xe_cmd)");
                    } catch (const chaiscript::exception::eval_error &error) {
                        (void)error;
                    }
                }
            }
        }
        return findings;
    }

    std::vector<xe::cmake::core::Finding>
    ScriptRuleLoader::run_check_project(const std::vector<xe::cmake::core::ConcreteSyntaxTree> &files, const xe::cmake::analysis::DirectedDependencyGraph &graph) const {
        std::vector<xe::cmake::core::Finding> findings;
        chaiscript::ChaiScript *chai = static_cast<chaiscript::ChaiScript *>(facade_.raw_engine());

        for (const HookSet &hooks : hooks_) {
            if (!hooks.has_check_project) {
                continue;
            }
            ScriptContext context;
            context.findings = &findings;
            ScriptGraph script_graph;
            script_graph.graph = &graph;
            std::vector<ScriptFile> script_files;
            for (const xe::cmake::core::ConcreteSyntaxTree &tree : files) {
                ScriptFile file;
                file.tree = &tree;
                script_files.push_back(file);
            }
            set_global(*chai, "__xe_ctx", chaiscript::Boxed_Value(context));
            set_global(*chai, "__xe_graph", chaiscript::Boxed_Value(script_graph));
            set_global(*chai, "__xe_project", chaiscript::Boxed_Value(script_files));
            try {
                chai->eval("check_project(__xe_ctx, __xe_project, __xe_graph)");
            } catch (const chaiscript::exception::eval_error &error) {
                (void)error;
            }
        }
        return findings;
    }

} // namespace xe::cmake::script