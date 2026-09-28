#include "CheckerDriver.h"
#include "CliOptionsParser.h"
#include "DiagnosticsReporter.h"

#include "xe/cmake/dsl/YamlRuleLoader.h"
#include "xe/cmake/io/FileSystem.h"
#include "xe/cmake/script/ScriptRuleLoader.h"
#include "xe/cmake/rules/RuleRegistry.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

namespace {

    void load_rules(const std::string &rules_dir, xe::cmake::rules::RuleRegistry &registry) {
        if (rules_dir.empty()) {
            return;
        }
        const std::filesystem::path directory(rules_dir);
        const std::filesystem::path yaml_path = directory / "cmake_guidelines.yaml";
        if (std::filesystem::exists(yaml_path)) {
            registry.add_dsl_rules(xe::cmake::dsl::YamlRuleLoader::load_from_file(yaml_path.string()));
        }
        const std::filesystem::path chai_path = directory / "cmake_guidelines.chai";
        if (std::filesystem::exists(chai_path)) {
            std::ifstream stream(chai_path);
            std::ostringstream buffer;
            buffer << stream.rdbuf();
            auto loader = std::make_shared<xe::cmake::script::ScriptRuleLoader>();
            loader->load(buffer.str());
            registry.add_script_loader(loader);
        }
    }

} // namespace

int main(int argc, char **argv) {
    const xe::cmake::CliOptions options = xe::cmake::CliOptionsParser::parse(argc, argv);

    if (options.help) {
        std::cout << xe::cmake::CliOptionsParser::usage_text() << "\n";
        return 0;
    }
    if (options.project.empty()) {
        std::cerr << "xe-cmake-checker: error: --project is required\n\n" << xe::cmake::CliOptionsParser::usage_text();
        return 2;
    }

    xe::cmake::io::NativeFileSystem filesystem;
    xe::cmake::rules::RuleRegistry registry;

    const std::string rules_dir = options.rules_dir.empty() ? std::filesystem::path("src/cmake-checker/rules").string() : options.rules_dir;
    load_rules(rules_dir, registry);

    xe::cmake::CheckerDriver driver(options, filesystem, registry);
    return driver.run();
}