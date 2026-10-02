#include "FixTemplateEngine.h"

namespace xe::cmake::dsl {

    std::optional<xe::cmake::core::Fix> FixTemplateEngine::apply_template(std::string_view template_name, const DslContext &context) {
        if (template_name == "quote_argument") {
            if (context.argument == nullptr) {
                return std::nullopt;
            }
            const xe::cmake::core::ArgumentNode &argument = *context.argument;
            if (argument.is_quoted() || argument.is_bracket()) {
                return std::nullopt;
            }
            xe::cmake::core::Fix fix("Quote argument");
            fix.add_edit(xe::cmake::core::TextEdit::replace(argument.span, "\"" + argument.text + "\""));
            return fix;
        }

        if (template_name == "unquote_argument") {
            if (context.argument == nullptr || !context.argument->is_quoted()) {
                return std::nullopt;
            }
            const xe::cmake::core::ArgumentNode &argument = *context.argument;
            xe::cmake::core::Fix fix("Unquote argument");
            fix.add_edit(xe::cmake::core::TextEdit::replace(argument.span, argument.text));
            return fix;
        }

        if (template_name == "replace_command_name") {
            if (context.command == nullptr) {
                return std::nullopt;
            }
            // The template needs a new name; not available from context alone.
            return std::nullopt;
        }

        if (template_name == "split_target_link_libraries_per_line") {
            if (context.command == nullptr || context.command->name != "target_link_libraries") {
                return std::nullopt;
            }
            const xe::cmake::core::CommandNode &command = *context.command;
            if (command.argument_count() < 3) {
                return std::nullopt;
            }
            (void)command.argument(0).text;
            std::string scope = "PRIVATE";
            std::vector<std::pair<std::string, std::string>> dependencies;
            for (std::size_t i = 1; i < command.argument_count(); ++i) {
                const std::string &text = command.argument(i).text;
                if (text == "PUBLIC" || text == "PRIVATE" || text == "INTERFACE") {
                    scope = text;
                } else {
                    dependencies.push_back(std::make_pair(scope, text));
                }
            }
            if (dependencies.size() < 2) {
                return std::nullopt;
            }
            std::string replacement = "# one line per dependency\n";
            for (const auto &dependency : dependencies) {
                replacement += "target_link_libraries(${target} " + dependency.first + " " + dependency.second + ")\n";
            }
            xe::cmake::core::Fix fix("Split target_link_libraries into one dependency per line");
            fix.add_edit(xe::cmake::core::TextEdit::replace(command.span, replacement));
            return fix;
        }

        if (template_name == "append_command_after") {
            // Requires command text parameter; not available from context alone.
            return std::nullopt;
        }

        return std::nullopt;
    }

} // namespace xe::cmake::dsl