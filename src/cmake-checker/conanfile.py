from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps


class CmakeCheckerConan(ConanFile):
    name = "cmake-checker"
    version = "0.0.0"
    description = "Xenoide CMake style checker"
    license = "MIT"
    package_type = "application"

    settings = "os", "compiler", "build_type", "arch"

    options = {"with_tests": [True, False]}
    default_options = {"with_tests": False}

    exports_sources = "CMakeLists.txt", "src/*"

    def requirements(self):
        self.requires("nlohmann_json/3.12.0")
        self.requires("rapidyaml/0.7.1", options={"with_default_callback_uses_exceptions": True})
        self.requires("cxxopts/3.3.1")
        self.requires("fmt/[>=11 <12]")

    def build_requirements(self):
        if self.options.with_tests:
            self.test_requires("catch2/3.14.0")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.variables["XE_BUILD_VERSION"] = self.version
        tc.variables["XE_ENABLE_TESTING"] = "ON" if self.options.with_tests else "OFF"
        tc.generate()