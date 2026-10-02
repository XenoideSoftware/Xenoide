#include "CliOptionsParser.h"

#include <cxxopts.hpp>

#include <iostream>

namespace xe::cmake {

    CliOptions CliOptionsParser::parse(int argc, char **argv) {
        cxxopts::Options options("xe-cmake-checker", "Xenoide CMake style checker (v2.2)");
        options
            .add_options()("project", "Path to the CMake project to check", cxxopts::value<std::string>())("project-build-dir", "Path to the project build tree holding trace.json and the File API replies", cxxopts::value<std::string>())("config", "Override .cmake-check.yaml discovery", cxxopts::value<std::string>())("rules", "Path to the rules directory (cmake_guidelines.yaml / .chai)", cxxopts::value<std::string>())("check", "Check only; never apply fixes")("diff", "Compute and display the unified diff of all fixable findings without writing to disk")("fix", "Apply all fixable findings atomically")("werror", "Promote warn findings to failures")(
                "help",
                "Print usage"
            );

        const cxxopts::ParseResult result = options.parse(argc, argv);

        CliOptions cli;
        if (result.count("help") > 0) {
            cli.help = true;
            return cli;
        }
        if (result.count("project") > 0) {
            cli.project = result["project"].as<std::string>();
        }
        if (result.count("project-build-dir") > 0) {
            cli.project_build_dir = result["project-build-dir"].as<std::string>();
        }
        if (result.count("config") > 0) {
            cli.config = result["config"].as<std::string>();
        }
        if (result.count("rules") > 0) {
            cli.rules_dir = result["rules"].as<std::string>();
        }
        cli.check = result.count("check") > 0;
        cli.diff = result.count("diff") > 0;
        cli.fix = result.count("fix") > 0;
        cli.werror = result.count("werror") > 0;
        return cli;
    }

    std::string CliOptionsParser::usage_text() {
        cxxopts::Options options("xe-cmake-checker", "Xenoide CMake style checker (v2.2)");
        options
            .add_options()("project", "Path to the CMake project to check", cxxopts::value<std::string>())("project-build-dir", "Path to the project build tree holding trace.json and the File API replies", cxxopts::value<std::string>())("config", "Override .cmake-check.yaml discovery", cxxopts::value<std::string>())("rules", "Path to the rules directory (cmake_guidelines.yaml / .chai)", cxxopts::value<std::string>())("check", "Check only; never apply fixes")("diff", "Compute and display the unified diff of all fixable findings without writing to disk")("fix", "Apply all fixable findings atomically")("werror", "Promote warn findings to failures")(
                "help",
                "Print usage"
            );
        return options.help();
    }

} // namespace xe::cmake