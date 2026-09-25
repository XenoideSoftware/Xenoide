#include <cxxopts.hpp>

#include <exception>
#include <iostream>
#include <string>

#include <cmcheck/Check.h>
#include <cmcheck/Report.h>

int main(int argc, char **argv) {
    try {
        cxxopts::Options options("cmake-checker", "Xenoide CMake style checker");

        options
            .add_options()("project", "Path to the CMake project to check", cxxopts::value<std::string>())("project-build-dir", "Path to the project build tree holding trace.json and the File API replies", cxxopts::value<std::string>())("config", "Override .cmake-check.yaml discovery", cxxopts::value<std::string>())("werror", "Promote warn findings to failures")(
                "help",
                "Print usage"
            );

        const cxxopts::ParseResult result = options.parse(argc, argv);

        if (result.count("help") > 0) {
            std::cout << options.help() << "\n";
            return 0;
        }

        cmcheck::CheckOptions checkOptions;
        if (result.count("project") == 0) {
            std::cerr << "cmake-checker: error: --project is required\n\n" << options.help();
            return 2;
        }
        checkOptions.project = result["project"].as<std::string>();
        if (result.count("project-build-dir") > 0) {
            checkOptions.build_dir = result["project-build-dir"].as<std::string>();
        }
        if (result.count("config") > 0) {
            checkOptions.config_file = result["config"].as<std::string>();
        }
        checkOptions.werror = result.count("werror") > 0;

        const cmcheck::CheckResult checkResult = cmcheck::runCheck(checkOptions, std::cout);
        if (checkResult.tool_error) {
            std::cerr << "cmake-checker: error: " << checkResult.error_message << "\n";
            return 2;
        }

        cmcheck::Report report;
        return report.exitCode(checkResult.findings, checkOptions.werror);
    } catch (const std::exception &error) {
        std::cerr << "cmake-checker: error: " << error.what() << "\n";
        return 2;
    }
}