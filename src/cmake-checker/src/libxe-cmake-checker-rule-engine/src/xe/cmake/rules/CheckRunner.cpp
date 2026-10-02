#include "CheckRunner.h"

#include "xe/cmake/core/QueryPrimitives.h"
#include "xe/cmake/dsl/DslPredicateEvaluator.h"
#include "xe/cmake/dsl/FixTemplateEngine.h"

namespace xe::cmake::rules {
    namespace {

        xe::cmake::core::Severity to_finding_severity(xe::cmake::core::RuleSeverity severity) {
            switch (severity) {
            case xe::cmake::core::RuleSeverity::Error:
                return xe::cmake::core::Severity::Error;
            case xe::cmake::core::RuleSeverity::Warn:
                return xe::cmake::core::Severity::Warn;
            case xe::cmake::core::RuleSeverity::Info:
            case xe::cmake::core::RuleSeverity::Off:
                return xe::cmake::core::Severity::Info;
            }
            return xe::cmake::core::Severity::Warn;
        }

    } // namespace

    std::vector<xe::cmake::core::Finding>
    CheckRunner::run(const std::vector<xe::cmake::core::ConcreteSyntaxTree> &files, const xe::cmake::analysis::DirectedDependencyGraph &graph) const {
        std::vector<xe::cmake::core::Finding> findings = run_dsl(files);
        std::vector<xe::cmake::core::Finding> script_findings = run_scripts(files, graph);
        findings.insert(findings.end(), script_findings.begin(), script_findings.end());
        return findings;
    }

    std::vector<xe::cmake::core::Finding> CheckRunner::run_dsl(const std::vector<xe::cmake::core::ConcreteSyntaxTree> &files) const {
        std::vector<xe::cmake::core::Finding> findings;
        const xe::cmake::dsl::DslPredicateEvaluator evaluator;

        for (const xe::cmake::core::ConcreteSyntaxTree &file : files) {
            const std::vector<const xe::cmake::core::CommandNode *> commands = file.commands();

            for (const xe::cmake::dsl::DslRule &rule : registry_.dsl_rules()) {
                const xe::cmake::core::RuleSeverity effective = registry_.effective_severity(rule.id, rule.severity);
                if (effective == xe::cmake::core::RuleSeverity::Off) {
                    continue;
                }

                switch (rule.match_node) {
                case xe::cmake::dsl::DslNodeMatch::File: {
                    xe::cmake::dsl::DslContext context;
                    context.file = &file;
                    if (!evaluator.evaluate(rule.when, context)) {
                        continue;
                    }
                    xe::cmake::core::Finding finding;
                    finding.rule_id = rule.id;
                    finding.severity = to_finding_severity(effective);
                    finding.file_path = file.file_path();
                    finding.span = !commands.empty() ? commands.front()->span : xe::cmake::core::SourceSpan{0, 0, 1, 1, 1, 1};
                    finding.message = evaluator.interpolate(rule.message, context);
                    const std::optional<xe::cmake::dsl::DslFixSpec> &fix_spec = rule.fix;
                    if (fix_spec.has_value() && fix_spec->template_name.has_value()) {
                        const std::string &template_name = *fix_spec->template_name;
                        finding.fix = xe::cmake::dsl::FixTemplateEngine::apply_template(template_name, context);
                    }
                    findings.push_back(std::move(finding));
                    break;
                }
                case xe::cmake::dsl::DslNodeMatch::Command: {
                    for (const xe::cmake::core::CommandNode *command : commands) {
                        if (!rule.match_name.empty() && command->name != rule.match_name) {
                            continue;
                        }
                        xe::cmake::dsl::DslContext context;
                        context.file = &file;
                        context.command = command;
                        if (!evaluator.evaluate(rule.when, context)) {
                            continue;
                        }
                        xe::cmake::core::Finding finding;
                        finding.rule_id = rule.id;
                        finding.severity = to_finding_severity(effective);
                        finding.file_path = command->file_path;
                        finding.span = command->span;
                        finding.message = evaluator.interpolate(rule.message, context);
                        const std::optional<xe::cmake::dsl::DslFixSpec> &fix_spec = rule.fix;
                        if (fix_spec.has_value() && fix_spec->template_name.has_value()) {
                            const std::string &template_name = *fix_spec->template_name;
                            finding.fix = xe::cmake::dsl::FixTemplateEngine::apply_template(template_name, context);
                        }
                        findings.push_back(std::move(finding));
                    }
                    break;
                }
                case xe::cmake::dsl::DslNodeMatch::Argument: {
                    for (const xe::cmake::core::CommandNode *command : commands) {
                        if (!rule.match_name.empty() && command->name != rule.match_name) {
                            continue;
                        }
                        for (std::size_t i = 0; i < command->argument_count(); ++i) {
                            const xe::cmake::core::ArgumentNode &argument = command->argument(i);
                            xe::cmake::dsl::DslContext context;
                            context.file = &file;
                            context.command = command;
                            context.argument = &argument;
                            if (!rule.match_pattern.empty() && !xe::cmake::core::StringPrimitives::regex_match(argument.text, rule.match_pattern)) {
                                continue;
                            }
                            if (!evaluator.evaluate(rule.when, context)) {
                                continue;
                            }
                            xe::cmake::core::Finding finding;
                            finding.rule_id = rule.id;
                            finding.severity = to_finding_severity(effective);
                            finding.file_path = command->file_path;
                            finding.span = argument.span;
                            finding.message = evaluator.interpolate(rule.message, context);
                            const std::optional<xe::cmake::dsl::DslFixSpec> &fix_spec = rule.fix;
                            if (fix_spec.has_value() && fix_spec->template_name.has_value()) {
                                const std::string &template_name = *fix_spec->template_name;
                                finding.fix = xe::cmake::dsl::FixTemplateEngine::apply_template(template_name, context);
                            }
                            findings.push_back(std::move(finding));
                        }
                    }
                    break;
                }
                case xe::cmake::dsl::DslNodeMatch::Block:
                    break;
                }
            }
        }
        return findings;
    }

    std::vector<xe::cmake::core::Finding>
    CheckRunner::run_scripts(const std::vector<xe::cmake::core::ConcreteSyntaxTree> &files, const xe::cmake::analysis::DirectedDependencyGraph &graph) const {
        std::vector<xe::cmake::core::Finding> findings;
        for (const auto &loader : registry_.script_loaders()) {
            std::vector<xe::cmake::core::Finding> file_findings = loader->run_check_file(files);
            std::vector<xe::cmake::core::Finding> command_findings = loader->run_check_command(files);
            std::vector<xe::cmake::core::Finding> project_findings = loader->run_check_project(files, graph);
            findings.insert(findings.end(), file_findings.begin(), file_findings.end());
            findings.insert(findings.end(), command_findings.begin(), command_findings.end());
            findings.insert(findings.end(), project_findings.begin(), project_findings.end());
        }
        return findings;
    }

} // namespace xe::cmake::rules