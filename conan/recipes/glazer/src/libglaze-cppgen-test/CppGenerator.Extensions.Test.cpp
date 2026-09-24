#include "MiniRegistry.h"

#include "glaze/cppgen/CppGenerator.h"

#include <catch2/catch_test_macros.hpp>

#include <string>

using glaze::cppgen::CppGenerator;
using glaze::testfixtures::loadMiniRegistry;

namespace {

bool contains(const std::string &haystack, std::string_view needle) {
    return haystack.find(needle) != std::string::npos;
}

} // namespace

TEST_CASE("cppgen opt-in emits ARB extension guards and flag vars",
          "[glaze][cppgen][extensions]") {
    const auto registry = loadMiniRegistry();
    CppGenerator gen{registry};
    const auto files = gen.generate("gl", "4.5", {"ARB"}, {});
    const auto &hpp = files.at("include/glaze/gl.hpp");

    // Guard macro and flag var extern for GL_ARB_buffer_storage.
    REQUIRE(contains(hpp, "#ifndef GLAZE_GL_NO_EXT_ARB_buffer_storage"));
    REQUIRE(contains(hpp, "extern int GLAZE_EXT_GL_ARB_buffer_storage"));

    // gl::exts::* namespace block.
    REQUIRE(contains(hpp, "namespace exts"));
    REQUIRE(contains(hpp, "inline bool ARB_buffer_storage()"));
    REQUIRE(contains(hpp, "inline bool ARB_draw_instanced()"));
}

TEST_CASE("cppgen extension-only commands emit functors wrapped in their guard",
          "[glaze][cppgen][extensions]") {
    const auto registry = loadMiniRegistry();
    CppGenerator gen{registry};
    const auto files = gen.generate("gl", "4.5", {"ARB"}, {});
    const auto &hpp = files.at("include/glaze/gl.hpp");

    // glDrawArraysInstancedARB is an ARB-only command from the MiniRegistry
    // extension block. Its functor must be wrapped in the ARB_draw_instanced
    // guard so clients can compile it out.
    REQUIRE(contains(hpp, "DrawArraysInstancedARBFn"));
    const auto fnPos = hpp.find("DrawArraysInstancedARBFn");
    REQUIRE(fnPos != std::string::npos);
    // Walk back to find the nearest enclosing #ifndef — it must be the
    // ARB_draw_instanced guard, not some earlier block.
    const auto guardPos = hpp.rfind("#ifndef GLAZE_GL_NO_EXT_ARB_draw_instanced", fnPos);
    REQUIRE(guardPos != std::string::npos);
    REQUIRE(guardPos < fnPos);
}

TEST_CASE("cppgen with empty filters emits no extension block",
          "[glaze][cppgen][extensions]") {
    const auto registry = loadMiniRegistry();
    CppGenerator gen{registry};
    const auto files = gen.generate("gl", "4.5", {}, {});
    const auto &hpp = files.at("include/glaze/gl.hpp");

    // With neither vendor nor name filters set, no extension is opted in
    // so no flag vars or guards appear.
    REQUIRE_FALSE(contains(hpp, "GLAZE_GL_NO_EXT_ARB_buffer_storage"));
    REQUIRE_FALSE(contains(hpp, "GLAZE_EXT_GL_ARB_buffer_storage"));
    REQUIRE_FALSE(contains(hpp, "DrawArraysInstancedARBFn"));
}

TEST_CASE("cppgen named extension filter opts in a single extension",
          "[glaze][cppgen][extensions]") {
    const auto registry = loadMiniRegistry();
    CppGenerator gen{registry};
    const auto files = gen.generate("gl", "4.5", {}, {"GL_KHR_debug"});
    const auto &hpp = files.at("include/glaze/gl.hpp");

    REQUIRE(contains(hpp, "GLAZE_GL_NO_EXT_KHR_debug"));
    REQUIRE(contains(hpp, "inline bool KHR_debug()"));
    // Other extensions from different vendors stay out.
    REQUIRE_FALSE(contains(hpp, "ARB_buffer_storage"));
}
