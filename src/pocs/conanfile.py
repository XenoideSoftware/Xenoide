from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy

import os

class xepocsRecipe(ConanFile):
    name = "xepocs"
    version = "0.0.0"
    package_type = "application"

    # Optional metadata
    license = "MIT"
    author = "Felipe Apablaza <felipe.apablaza@protonmail.com>"
    url = "https://github.com/XenoideSoftware/Xenoide"
    description = "Proof of concepts for the Xenoide project"
    topics = ("ide", "game-engine", "graphics")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    options = {
        "with_poc_clang": [True, False],
        "with_poc_lsp": [True, False],
        "with_poc_protobuf": [True, False],
        "with_poc_wxwidgets": [True, False],
        "with_poc_zeromq": [True, False],
    }

    default_options = {
        "with_poc_clang": True,
        "with_poc_lsp": True,
        "with_poc_protobuf": True,
        "with_poc_wxwidgets": False,
        "with_poc_zeromq": True,
    }

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*"

    def layout(self):
        cmake_layout(self)

    def _require_wx3(self):
        return self.options.get_safe("with_poc_wxwidgets")

    def requirements(self):
        # POCS dependencies
        if self.options.get_safe("with_poc_clang"):
            self.requires("llvm/18.1.8@xenoide/xenoide")

        if self.options.get_safe("with_poc_lsp"):
            self.requires("lsp-framework/1.3.1@xenoide/xenoide")

        if self.options.get_safe("with_poc_protobuf"):
            self.requires("protobuf/3.9.1@xenoide/xenoide")

        if self.options.get_safe("with_poc_zeromq"):
            self.requires("zeromq/4.3.5@xenoide/xenoide")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.variables["XE_ENABLE_POC_CLANG"] = "ON" if self.options.get_safe("with_poc_clang") else "OFF"
        tc.variables["XE_ENABLE_POC_LSP"] = "ON" if self.options.get_safe("with_poc_lsp") else "OFF"
        tc.variables["XE_ENABLE_POC_PROTOBUF"] = "ON" if self.options.get_safe("with_poc_protobuf") else "OFF"
        tc.variables["XE_ENABLE_POC_WXWIDGETS"] = "ON" if self.options.get_safe("with_poc_wxwidgets") else "OFF"
        tc.variables["XE_ENABLE_POC_ZEROMQ"] = "ON" if self.options.get_safe("with_poc_zeromq") else "OFF"

        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
