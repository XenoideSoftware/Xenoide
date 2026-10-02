import os

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy


class GlazerConan(ConanFile):
    name = "glazer"
    version = "1.0.0"
    description = "Glaze OpenGL binding generator CLI tool"
    license = "MIT"
    package_type = "application"

    settings = "os", "compiler", "build_type", "arch"

    options = {
        "with_tests": [True, False],
    }

    default_options = {
        "with_tests": False,
    }

    exports_sources = (
        "CMakeLists.txt",
        "cmake/*",
        "src/*",
    )

    def requirements(self):
        self.requires("pugixml/1.15")
        self.requires("inja/3.4.0")
        self.requires("nlohmann_json/3.12.0")
        self.requires("cxxopts/3.3.1")

    def build_requirements(self):
        if self.options.with_tests:
            self.test_requires("catch2/3.7.1")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.variables["GLAZE_CPP_WITH_TESTS"] = bool(self.options.with_tests)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        if self.options.with_tests:
            cmake.test()

    def package(self):
        # Package only the glaze CLI binary. Patterns are wildcarded so they
        # match through the per-config subdir (Debug/Release) that MSVC's
        # multi-config generator inserts between bin/ and the executable.
        copy(self, "*glaze",
             src=os.path.join(self.build_folder, "bin"),
             dst=os.path.join(self.package_folder, "bin"),
             keep_path=False)
        copy(self, "*glaze.exe",
             src=os.path.join(self.build_folder, "bin"),
             dst=os.path.join(self.package_folder, "bin"),
             keep_path=False)

    def package_info(self):
        # Standard pattern for tool_requires application packages.
        self.cpp_info.frameworkdirs = []
        self.cpp_info.libdirs = []
        self.cpp_info.resdirs = []
        self.cpp_info.includedirs = []

        bindir = os.path.join(self.package_folder, "bin")
        self.buildenv_info.prepend_path("PATH", bindir)
