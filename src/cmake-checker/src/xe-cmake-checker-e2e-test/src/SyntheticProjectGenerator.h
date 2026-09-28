#ifndef XE_CMAKE_TESTING_SYNTHETIC_PROJECT_GENERATOR_H
#define XE_CMAKE_TESTING_SYNTHETIC_PROJECT_GENERATOR_H

#include "xe/cmake/io/FileSystem.h"

#include <cstdint>
#include <string>
#include <vector>

namespace xe::cmake::testing {

    // Parametric builder for synthetic in-memory CMake projects that intentionally
    // violate docs/CMAKE.md conventions to challenge the checkers and fixers.
    class CMakeProjectFixtureBuilder {
    public:
        CMakeProjectFixtureBuilder &withExecutableCount(std::size_t count) {
            exe_count_ = count;
            return *this;
        }

        CMakeProjectFixtureBuilder &withLibraryCount(std::size_t count) {
            lib_count_ = count;
            return *this;
        }

        CMakeProjectFixtureBuilder &withTestCount(std::size_t count) {
            test_count_ = count;
            return *this;
        }

        CMakeProjectFixtureBuilder &withDiverseCMakeSyntaxStyles() {
            diverse_styles_ = true;
            return *this;
        }

        CMakeProjectFixtureBuilder &withSeed(std::uint32_t seed) {
            seed_ = seed;
            return *this;
        }

        // Populates the given in-memory filesystem with a full synthetic project.
        void build(xe::cmake::io::InMemoryFileSystem &fs) const;

    private:
        std::size_t exe_count_ = 1;
        std::size_t lib_count_ = 2;
        std::size_t test_count_ = 1;
        bool diverse_styles_ = true;
        std::uint32_t seed_ = 0;
    };

} // namespace xe::cmake::testing

#endif // XE_CMAKE_TESTING_SYNTHETIC_PROJECT_GENERATOR_H