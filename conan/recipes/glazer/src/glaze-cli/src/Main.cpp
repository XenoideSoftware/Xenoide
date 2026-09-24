#include "glaze/BindingGenerator.h"
#include "glaze/cli/ArgParse.h"
#include "glaze/producer/ParseError.h"

#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void printUsage() {
    std::cout
        << "glaze — OpenGL binding generator\n"
        << "\n"
        << "Usage:\n"
        << "  glaze generate --registry <path> --api <api:version> --lang <c|cpp>\n"
        << "                    --output-dir <dir> [--extension-vendors csv]\n"
        << "                    [--extensions csv]\n"
        << "  glaze list-apis [--registry <path>]\n"
        << "\n"
        << "Flags:\n"
        << "  --registry <path>          Path to gl.xml (default: ./gl.xml)\n"
        << "  --api <name>:<version>     Repeatable: e.g. 'gl:3.3' or 'gles2:3.2'\n"
        << "  --lang <c|cpp>             Repeatable: c and/or cpp\n"
        << "  --output-dir <dir>         Where files are written (default: '.')\n"
        << "  --extension-vendors <csv>  Additive vendor prefix filter\n"
        << "  --extensions <csv>         Additive full-name extension filter\n"
        << "  -h, --help                 Show this help\n";
}

int runGenerate(int argc, const char *const *argv) {
    const auto options = glaze::cli::parseGenerate(argc, argv);
    if (!options) {
        printUsage();
        return 0;
    }

    const auto result = glaze::BindingGenerator{}.generate(*options);
    for (const auto &path : result.writtenFiles) {
        std::cout << "wrote " << path.string() << "\n";
    }
    return 0;
}

int runListApis(int argc, const char *const *argv) {
    const auto registryPath = glaze::cli::parseListApisRegistryPath(argc, argv);
    const auto apis = glaze::BindingGenerator{}.listApis(registryPath);
    std::cout << "Available APIs and versions:\n";
    for (const auto &[api, versions] : apis) {
        std::cout << "  " << api << ":";
        for (const auto &v : versions) {
            std::cout << " " << v;
        }
        std::cout << "\n";
    }
    return 0;
}

} // namespace

int main(int argc, char **argv) {
    try {
        if (argc < 2) {
            printUsage();
            return 1;
        }
        const std::string cmd = argv[1];
        if (cmd == "-h" || cmd == "--help") {
            printUsage();
            return 0;
        }
        if (cmd == "generate") {
            // Pass argv+1 as the "subcommand argv" so cxxopts treats "generate"
            // as the program name. This mirrors the Python glaze_cli.py
            // subcommand dispatch shape.
            return runGenerate(argc - 1, argv + 1);
        }
        if (cmd == "list-apis") {
            return runListApis(argc - 1, argv + 1);
        }
        std::cerr << "error: unknown subcommand: " << cmd << "\n\n";
        printUsage();
        return 1;
    } catch (const glaze::producer::ParseError &e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    } catch (const std::invalid_argument &e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    } catch (const std::exception &e) {
        std::cerr << "error: " << e.what() << "\n";
        return 2;
    }
}
