from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy

import os

class xenoideRecipe(ConanFile):
    name = "xenoide"
    version = "0.0.0"
    package_type = "application"

    # Optional metadata
    license = "MIT"
    author = "Felipe Apablaza <felipe.apablaza@protonmail.com>"
    url = "https://github.com/XenoideSoftware/Xenoide"
    description = "IDE for Native C++ development"
    topics = ("ide", "c++", "development")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    options = {
        "with_tests": [True, False],
        "with_ide": [True, False],
        "with_ide_winlamb": [True, False],
        "with_ide_qt6": [True, False],
        "with_ide_wx3": [True, False],
        "with_ide_gtk4": [True, False],
    }

    default_options = {
        "with_tests": True,
        "with_ide_winlamb": True,
        "with_ide_qt6": True,
        "with_ide_wx3": False,
        "with_ide_gtk4": False,
    }

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*"

    def layout(self):
        cmake_layout(self)

    def config_options(self):
        """
        if self._require_gtk4() and self._require_wx3():
            print("Can't build with gtk4 and wxWidgets3 at the same time")
        """

        if self.options.with_ide_winlamb:
            if self.settings.os != "Windows":
                print("with_ide_winlamb is enabled but target OS is not windows. Changing to False")
                self.options.rm_safe("with_ide_winlamb")

    def _require_cli(self):
        # Add here additional options that might build a custom CLI tool
        return self.options.get_safe("with_engine_tool_ktxc") or self.options.get_safe("with_engine_tool_gltfc")

    def _require_wx3(self):
        return (self.options.get_safe("with_ide") and self.options.get_safe("with_ide_wx3")) or self.options.get_safe("with_poc_wxwidgets")

    def _require_gtk4(self):
        return self.options.get_safe("with_ide") and self.options.get_safe("with_ide_gtk4")

    def requirements(self):
        # required by all
        if self.options.with_tests:
            self.requires("catch2/3.14.0")

        # required by the ide
        # TODO: Parametrize dependency target OS
        if self.options.get_safe("with_ide_winlamb"):
            self.requires("winlamb/2026.06.24@xenoide/xenoide")
            self.requires("scintilla3/3.7.6@xenoide/xenoide")

        if self.options.get_safe("with_ide_qt6"):
            self.requires("qt/6.8.3")
            self.requires("scintilla/5.5.7@xenoide/xenoide")
            self.requires("lexilla/5.4.6@xenoide/xenoide")

        if self._require_gtk4():
            self.requires("gtk/system", options={"version" : 4})

        if self._require_wx3():
            self.requires("wxwidgets/3.3.3")

        if self._require_cli():
            self.requires("cxxopts/3.3.1")

    def configure(self):
        if self.options.get_safe("with_ide_wx3"):
            self.options["wxwidgets"].stc = True
            self.options["wxwidgets"].gtk = "3"

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)

        # Common
        tc.variables["XE_BUILD_VERSION"] = self.version
        tc.variables["XE_ENABLE_TESTING"] = "ON" if self.options.with_tests else "OFF"

        # IDE
        tc.variables["XE_ENABLE_IDE_WINLAMB"] = "ON" if self.options.get_safe("with_ide_winlamb") else "OFF"
        tc.variables["XE_ENABLE_IDE_QT6"] = "ON" if self.options.get_safe("with_ide_qt6") else "OFF"
        tc.variables["XE_ENABLE_IDE_WX3"] = "ON" if self.options.get_safe("with_ide_wx3") else "OFF"
        tc.variables["XE_ENABLE_IDE_GTK4"] = "ON" if self.options.get_safe("with_ide_gtk4") else "OFF"

        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
