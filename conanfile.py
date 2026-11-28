from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy

class xenoideRecipe(ConanFile):
    name = "xenoide"
    version = "0.0.0"
    package_type = "application"

    # Optional metadata
    license = "MIT"
    author = "Felipe Apablaza <felipe.apablaza@protonmail.com>"
    url = "https://github.com/XenoideSoftware/xenoide"
    description = "IDE for Native C++ development"
    topics = ("ide", "c++", "development")
    
    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    options = {
        "with_tests": [True, False],
        "with_ui_qt6": [True, False],
    }

    default_options = {
        "with_tests": True,
        "with_ui_qt6": True,
    }

    # Sources are located in the same place as this recipe.
    exports_sources = "CMakeLists.txt", "src/*"

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        if self.options.get_safe("with_tests"):
            self.requires("catch2/3.16.1")

        if self.options.get_safe("with_ui_qt6"):
            # NOTE: pick a version that is available as a binary package
            self.requires("qt/6.12.0")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.variables["XENOIDE_ENABLE_TESTING"] = "ON" if self.options.with_tests else "OFF"
        tc.variables["XENOIDE_ENABLE_UI_QT6"] = "ON" if self.options.get_safe("with_ui_qt6") else "OFF"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
