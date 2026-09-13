#include "glaze/cli/ArgParse.h"

#include <cxxopts.hpp>

#include <sstream>
#include <stdexcept>

namespace glaze::cli {

namespace {

//! Split a comma-separated list into a set of tokens, trimming whitespace.
std::set<std::string> splitCsvSet(const std::string &csv) {
    std::set<std::string> out;
    std::string current;
    for (char ch : csv) {
        if (ch == ',') {
            if (!current.empty()) {
                out.insert(current);
            }
            current.clear();
        } else if (ch != ' ') {
            current.push_back(ch);
        }
    }
    if (!current.empty()) {
        out.insert(current);
    }
    return out;
}

//! Parse "api:version" into an ApiRequest. Throws if the colon is missing.
ApiRequest parseApiSpec(const std::string &spec) {
    const auto colon = spec.find(':');
    if (colon == std::string::npos) {
        throw std::invalid_argument("expected api:version form for --api, got '" + spec + "'");
    }
    return ApiRequest{spec.substr(0, colon), spec.substr(colon + 1)};
}

} // namespace

std::optional<GenerateOptions> parseGenerate(int argc, const char *const *argv) {
    cxxopts::Options opts{"glaze generate", "Generate OpenGL bindings"};
    opts.add_options()
        ("registry", "Path to gl.xml", cxxopts::value<std::string>()->default_value("gl.xml"))
        ("api", "API spec in api:version form (repeatable)",
            cxxopts::value<std::vector<std::string>>())
        ("lang", "Output language: c or cpp (repeatable)",
            cxxopts::value<std::vector<std::string>>())
        ("output-dir", "Output directory", cxxopts::value<std::string>()->default_value("."))
        ("extension-vendors", "Comma-separated vendor prefixes (e.g. ARB,KHR)",
            cxxopts::value<std::string>()->default_value(""))
        ("extensions", "Comma-separated extension names (e.g. GL_KHR_debug)",
            cxxopts::value<std::string>()->default_value(""))
        ("refpages-dir", "Path to OpenGL-Refpages for Doxygen brief attachment",
            cxxopts::value<std::string>()->default_value(""))
        ("h,help", "Print help");

    try {
        const auto parsed = opts.parse(argc, const_cast<char **>(argv));
        if (parsed.count("help") != 0) {
            return std::nullopt;
        }

        GenerateOptions out;
        out.registryPath = parsed["registry"].as<std::string>();
        out.outputDir = parsed["output-dir"].as<std::string>();

        if (parsed.count("api") == 0) {
            throw std::invalid_argument("at least one --api is required");
        }
        for (const auto &spec : parsed["api"].as<std::vector<std::string>>()) {
            out.apis.push_back(parseApiSpec(spec));
        }

        if (parsed.count("lang") == 0) {
            throw std::invalid_argument("at least one --lang is required");
        }
        for (const auto &lang : parsed["lang"].as<std::vector<std::string>>()) {
            if (lang == "c") {
                out.languages.push_back(Language::C);
            } else if (lang == "cpp" || lang == "c++") {
                out.languages.push_back(Language::Cpp);
            } else {
                throw std::invalid_argument("unknown --lang value: " + lang);
            }
        }

        out.extensionVendors = splitCsvSet(parsed["extension-vendors"].as<std::string>());
        out.extensionNames = splitCsvSet(parsed["extensions"].as<std::string>());
        if (const auto &refpages = parsed["refpages-dir"].as<std::string>();
            !refpages.empty()) {
            out.refpagesDir = refpages;
        }
        return out;
    } catch (const cxxopts::exceptions::exception &e) {
        throw std::invalid_argument(std::string{"cxxopts error: "} + e.what());
    }
}

std::string parseListApisRegistryPath(int argc, const char *const *argv) {
    cxxopts::Options opts{"glaze list-apis", "List available APIs and versions"};
    opts.add_options()
        ("registry", "Path to gl.xml", cxxopts::value<std::string>()->default_value("gl.xml"))
        ("h,help", "Print help");
    try {
        const auto parsed = opts.parse(argc, const_cast<char **>(argv));
        return parsed["registry"].as<std::string>();
    } catch (const cxxopts::exceptions::exception &e) {
        throw std::invalid_argument(std::string{"cxxopts error: "} + e.what());
    }
}

} // namespace glaze::cli
