#ifndef XE_CMAKE_CLI_OPTIONS_PARSER_H
#define XE_CMAKE_CLI_OPTIONS_PARSER_H

#include <optional>
#include <string>
#include <vector>

namespace xe::cmake {

    // Parsed command-line options for the xe-cmake-checker CLI.
    struct CliOptions {
        std::string project;
        std::string project_build_dir;
        std::string config;
        std::string rules_dir;
        std::vector<std::string> rule_files;
        bool check = false;
        bool diff = false;
        bool fix = false;
        bool werror = false;
        bool help = false;
    };

    // Thin wrapper over cxxopts.
    class CliOptionsParser {
    public:
        // Parses argv. Returns the options; help sets options.help.
        static CliOptions parse(int argc, char **argv);

        // Prints usage text to stdout.
        static std::string usage_text();
    };

} // namespace xe::cmake

#endif // XE_CMAKE_CLI_OPTIONS_PARSER_H