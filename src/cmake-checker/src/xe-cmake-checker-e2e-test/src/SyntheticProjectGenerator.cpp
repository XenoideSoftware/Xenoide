#include "SyntheticProjectGenerator.h"

#include <random>

namespace xe::cmake::testing {
    namespace {

        std::string library_folder(std::size_t index) {
            return "libxe-" + std::to_string(index);
        }

        std::string executable_folder(std::size_t index) {
            return "xe-app-" + std::to_string(index);
        }

        std::string test_folder(std::size_t index) {
            return "libxe-" + std::to_string(index) + "-test";
        }

        // Style 1: direct inline sources in target declarations (omits set(sources...)).
        std::string style1_library_listfile(const std::string &folder, const std::string &) {
            return "set (target \"" + folder +
                   "\")\n"
                   "add_library(${target} STATIC src/Alpha.cpp)\n";
        }

        // Style 4/7/8: unquoted sources, missing alias and include dirs.
        std::string style2_library_listfile(const std::string &folder, const std::string &source) {
            return "set (target \"" + folder +
                   "\")\n"
                   "set (sources " +
                   source +
                   ")\n"
                   "add_library(${target} ${sources})\n";
        }

        // Style 6: multiple targets declared in one directory.
        std::string style3_library_listfile(const std::string &folder, const std::string &) {
            return "set (target \"" + folder +
                   "\")\n"
                   "set (sources \"src/A.cpp\")\n"
                   "add_library(${target} ${sources})\n"
                   "add_executable(" +
                   folder + "-tool ${sources})\n";
        }

    } // namespace

    void CMakeProjectFixtureBuilder::build(xe::cmake::io::InMemoryFileSystem &fs) const {
        std::mt19937 rng(seed_);
        std::uniform_int_distribution<int> style_dist(0, 2);

        const std::string root = "/virtual_project";
        fs.create_directories(root);

        // Root listfile.
        fs.write_file(root + "/CMakeLists.txt", "cmake_minimum_required (VERSION 3.25)\nproject (Synthetic)\n");

        // Libraries.
        std::vector<std::string> library_folders;
        for (std::size_t i = 0; i < lib_count_; ++i) {
            const std::string folder = library_folder(i);
            const std::string dir = root + "/" + folder;
            fs.create_directories(dir + "/src");
            const std::string header = dir + "/src/Alpha.h";
            const std::string source_file = dir + "/src/Alpha.cpp";
            fs.write_file(header, "#pragma once\nint " + folder + "_calculate_constant();\n");
            fs.write_file(source_file, "#include \"Alpha.h\"\nint " + folder + "_calculate_constant() {\n    return 107;\n}\n");

            const int style = diverse_styles_ ? style_dist(rng) : 0;
            if (style == 0) {
                fs.write_file(dir + "/CMakeLists.txt", style1_library_listfile(folder, "src/Alpha.cpp"));
            } else if (style == 1) {
                fs.write_file(dir + "/CMakeLists.txt", style2_library_listfile(folder, "src/Alpha.cpp"));
            } else {
                fs.write_file(dir + "/CMakeLists.txt", style3_library_listfile(folder, "src/Alpha.cpp"));
            }
            library_folders.push_back(folder);
        }

        // Executables link a random subset of libraries.
        std::uniform_int_distribution<int> pick_dist(0, static_cast<int>(lib_count_) - 1);
        for (std::size_t i = 0; i < exe_count_; ++i) {
            const std::string folder = executable_folder(i);
            const std::string dir = root + "/" + folder;
            fs.create_directories(dir + "/src");
            fs.write_file(dir + "/src/main.cpp", "#include \"Alpha.h\"\nint main() {\n    return 0;\n}\n");

            std::string listfile = "set (target \"" + folder +
                                   "\")\n"
                                   "set (sources \"src/main.cpp\")\n"
                                   "add_executable(${target} ${sources})\n";
            if (lib_count_ > 0) {
                listfile += "\n# one line per dependency\n";
                const int dep_count = pick_dist(rng) + 1;
                for (int d = 0; d < dep_count; ++d) {
                    const int lib_index = pick_dist(rng);
                    listfile += "target_link_libraries(${target} PRIVATE " + library_folders[static_cast<std::size_t>(lib_index)] + ")\n";
                }
            }
            fs.write_file(dir + "/CMakeLists.txt", listfile);
        }

        // Test targets.
        for (std::size_t i = 0; i < test_count_; ++i) {
            const std::string folder = test_folder(i);
            const std::string dir = root + "/" + folder;
            fs.create_directories(dir + "/src");
            fs.write_file(dir + "/src/AlphaTest.cpp", "#include <catch2/catch_test_macros.hpp>\nTEST_CASE(\"test\") { REQUIRE(true); }\n");
            fs.write_file(
                dir + "/CMakeLists.txt",
                "set (target \"" + folder +
                    "\")\n"
                    "set (sources \"src/AlphaTest.cpp\")\n"
                    "add_executable(${target} ${sources})\n"
                    "target_include_directories(${target} PUBLIC \"src\")\n"
            );
        }
    }

} // namespace xe::cmake::testing