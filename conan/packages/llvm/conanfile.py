from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout
from conan.tools.files import get, copy, rmdir
import os


class LLVMConanfile(ConanFile):
    name = "llvm"
    version = "18.1.8"
    description = (
        "LLVM and Clang compiler infrastructure built as libraries, "
        "providing code analysis, transformation, and IDE tooling support."
    )
    license = "Apache-2.0 WITH LLVM-exception"
    homepage = "https://llvm.org/"
    url = "https://github.com/llvm/llvm-project"
    topics = ("llvm", "clang", "compiler", "code-analysis", "ide-tooling")

    settings = "os", "compiler", "build_type", "arch"

    options = {
        "shared": [True, False],
        "fPIC": [True, False],

        # Semicolon-separated list of LLVM target backends.
        # Use "host" to build only the current machine's backend.
        # Common values: "X86", "AArch64", "ARM", "host", "all"
        "targets": ["ANY"],

        # Enable the Clang frontend as a library (libclang, clangTooling, clangFormat, etc.)
        "with_clang": [True, False],

        # Enable clang-tools-extra (clang-tidy, clang-apply-replacements, etc.)
        # Requires with_clang=True.
        "with_clang_tools_extra": [True, False],

        # Enable C++ RTTI — required to use dynamic_cast / typeid with LLVM/Clang types.
        # Must be consistent with how the consuming project is compiled.
        "enable_rtti": [True, False],

        # Enable LLVM assertions. Useful for Debug builds when developing tooling.
        "enable_assertions": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "targets": "X86",
        "with_clang": True,
        "with_clang_tools_extra": True,
        "enable_rtti": True,
        "enable_assertions": False,
    }

    exports_sources = "conandata.yml"

    def build_requirements(self):
        self.tool_requires("cpython/3.12.7", options={"shared": True})

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        # clang-tools-extra lives inside the Clang sub-project tree.
        if self.options.with_clang_tools_extra and not self.options.with_clang:
            self.options.with_clang = True

    def layout(self):
        # Source is unpacked into <cache>/src; build artefacts go to <cache>/build.
        cmake_layout(self, src_folder="src")

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def generate(self):
        tc = CMakeToolchain(self)

        # ── Sub-projects ──────────────────────────────────────────────────────
        # LLVM core is always built. Clang and clang-tools-extra are optional
        # sub-projects that are compiled together in one CMake invocation by
        # setting LLVM_ENABLE_PROJECTS before configuring the llvm/ root.
        enabled_projects = []
        if self.options.with_clang:
            enabled_projects.append("clang")
        if self.options.with_clang_tools_extra:
            enabled_projects.append("clang-tools-extra")

        tc.variables["LLVM_ENABLE_PROJECTS"] = ";".join(enabled_projects)

        # ── Target backends ───────────────────────────────────────────────────
        # Limiting to the host architecture drastically reduces build time.
        tc.variables["LLVM_TARGETS_TO_BUILD"] = str(self.options.targets)

        # ── Minimize build ────────────────────────────────────────────────────
        tc.variables["LLVM_BUILD_TESTS"]         = False
        tc.variables["LLVM_INCLUDE_TESTS"]        = False
        tc.variables["LLVM_BUILD_EXAMPLES"]       = False
        tc.variables["LLVM_INCLUDE_EXAMPLES"]     = False
        tc.variables["LLVM_BUILD_BENCHMARKS"]     = False
        tc.variables["LLVM_INCLUDE_BENCHMARKS"]   = False
        tc.variables["LLVM_INCLUDE_DOCS"]         = False
        tc.variables["LLVM_ENABLE_BINDINGS"]      = False
        tc.variables["LLVM_ENABLE_OCAMLDOC"]      = False

        # ── Optional third-party libs — off for a self-contained build ────────
        tc.variables["LLVM_ENABLE_ZLIB"]     = "OFF"
        tc.variables["LLVM_ENABLE_ZSTD"]     = "OFF"
        tc.variables["LLVM_ENABLE_LIBXML2"]  = False
        tc.variables["LLVM_ENABLE_TERMINFO"] = False
        tc.variables["LLVM_ENABLE_LIBEDIT"]  = False
        tc.variables["LLVM_ENABLE_LIBPFM"]   = False

        # ── C++ ABI ───────────────────────────────────────────────────────────
        # enable_rtti must match the consuming project's compiler flags.
        tc.variables["LLVM_ENABLE_RTTI"]       = self.options.enable_rtti
        # Exceptions depend on RTTI in LLVM's build system.
        tc.variables["LLVM_ENABLE_EH"]         = self.options.enable_rtti
        tc.variables["LLVM_ENABLE_ASSERTIONS"] = self.options.enable_assertions

        # ── Shared vs. static ─────────────────────────────────────────────────
        if self.options.shared:
            # Build a single libLLVM shared library and link tools against it.
            tc.variables["LLVM_BUILD_LLVM_DYLIB"]  = True
            tc.variables["LLVM_LINK_LLVM_DYLIB"]   = True
        else:
            tc.variables["BUILD_SHARED_LIBS"] = False

        if self.settings.os != "Windows":
            fPIC = self.options.get_safe("fPIC", default=True)
            tc.variables["CMAKE_POSITION_INDEPENDENT_CODE"] = fPIC

        # ── Clang sub-project overrides ───────────────────────────────────────
        if self.options.with_clang:
            tc.variables["CLANG_BUILD_TESTS"]   = False
            tc.variables["CLANG_INCLUDE_TESTS"] = False
            tc.variables["CLANG_INCLUDE_DOCS"]  = False

        tc.generate()

    def build(self):
        cmake = CMake(self)
        # LLVM's root CMakeLists.txt lives inside the "llvm/" sub-directory of
        # the monorepo. Clang and clang-tools-extra are pulled in via
        # LLVM_ENABLE_PROJECTS — they are NOT configured as standalone projects.
        cmake.configure(build_script_folder=os.path.join(self.source_folder, "llvm"))
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

        # Drop man pages and other documentation — not useful to library consumers.
        rmdir(self, os.path.join(self.package_folder, "share"))

    def package_info(self):
        # LLVM installs its own CMake config packages after `cmake --install`:
        #
        #   lib/cmake/llvm/LLVMConfig.cmake   → find_package(LLVM CONFIG)
        #   lib/cmake/clang/ClangConfig.cmake → find_package(Clang CONFIG)
        #
        # Consumers should use those packages directly rather than relying on
        # Conan-generated targets, because LLVM provides its own helpers such as
        # llvm_map_components_to_libnames() and separate_arguments().
        #
        # In CMakeLists.txt:
        #   find_package(LLVM  REQUIRED CONFIG)
        #   find_package(Clang REQUIRED CONFIG)   # if with_clang=True
        #   llvm_map_components_to_libnames(llvm_libs support core irreader)
        #   target_link_libraries(mytarget PRIVATE ${llvm_libs})
        #   target_link_libraries(mytarget PRIVATE clangTooling clangFormat libclang)

        # Disable Conan's own CMake file generation for this package so it
        # does not shadow LLVM's native config files.
        self.cpp_info.set_property("cmake_find_mode", "none")

        # Expose the directories that contain LLVMConfig.cmake / ClangConfig.cmake
        # so that Conan's toolchain file adds them to CMAKE_PREFIX_PATH.
        self.cpp_info.builddirs = [os.path.join("lib", "cmake", "llvm")]
        if self.options.with_clang:
            self.cpp_info.builddirs.append(os.path.join("lib", "cmake", "clang"))

        self.cpp_info.includedirs = ["include"]
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.bindirs = ["bin"]
