from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy

import os

class xenoideRecipe(ConanFile):
    name = "xe"
    version = "0.0.0"
    package_type = "application"

    # Optional metadata
    license = "MIT"
    author = "Felipe Apablaza <felipe.apablaza@protonmail.com>"
    url = "https://github.com/XenoideSoftware/Xenoide"
    description = "IDE for game development with an integrated engine"
    topics = ("ide", "game-engine", "graphics")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    options = {
        "with_ide": [True, False],
        "with_ide_winlamb": [True, False],
        "with_tests": [True, False],
        "with_engine": [True, False],
        "with_engine_renderer_gl": [True, False],
        "with_engine_renderer_vk": [True, False],
        "with_engine_tool_ktxc": [True, False],
        "with_engine_tool_gltfc": [True, False],
        "with_engine_tool_gltfv": [True, False],
        "with_poc_clang": [True, False],
        "with_poc_lsp": [True, False],
        "with_poc_protobuf": [True, False],
        "with_poc_wxwidgets": [True, False],
        "with_poc_zeromq": [True, False],
    }

    default_options = {
        "with_ide": True,
        "with_ide_winlamb": True,
        "with_tests": True,
        "with_engine": True,
        "with_engine_renderer_gl": True,
        "with_engine_renderer_vk": True,
        "with_engine_tool_ktxc": True,
        "with_engine_tool_gltfc": True,
        "with_engine_tool_gltfv": True,
        "with_poc_clang": True,
        "with_poc_lsp": True,
        "with_poc_protobuf": True,
        "with_poc_wxwidgets": True,
        "with_poc_zeromq": True,
    }

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*"

    def layout(self):
        cmake_layout(self)

    def config_options(self):
        if self.options.with_ide_winlamb:
            if self.settings.os != "Windows":
                print("with_ide_winlamb is enabled but target OS is not windows. Changing to False")
                self.options.rm_safe("with_ide_winlamb")

    def _require_cli(self):
        # Add here additional options that might build a custom CLI tool
        return self.options.get_safe("with_engine_tool_ktxc") or self.options.get_safe("with_engine_tool_gltfc")

    def requirements(self):        
        # required by all
        if self.options.with_tests and (self.options.get_safe("with_ide") or self.options.get_safe("with_engine")):
            self.requires("catch2/3.14.0")

        if self.options.get_safe("with_ide"):
            # required by the ide
            # TODO: Parametrize dependency target OS
            if self.options.get_safe("with_ide_winlamb"):
                self.requires("winlamb/2026.06.24")
                self.requires("scintilla3/3.7.6")

        if self._require_cli():
            self.requires("cxxopts/3.3.1")

        # required by the engine
        if self.options.get_safe("with_engine"):
            self.requires("fmt/[>=11 <12]")
            self.requires("ms-gsl/4.2.0")
            self.requires("backport-cpp/1.2.0")
            self.requires("tl-expected/1.2.0")
            self.requires("gsl-lite/1.1.0")

            if self.options.get_safe("with_engine_renderer_vk"):
                self.requires("vulkan-loader/1.4.313.0")

            if self.options.get_safe("with_engine_renderer_gl"):
                self.requires("glfw/3.4")
                self.requires("glazed/1.0.0", options={"language": "both", "apis": "gl:4.6,gles2:3.2,gl_compat:2.1"})

            if self.options.get_safe("with_engine_tool_ktxc"):
                self.requires("ktx/4.4.2")
                self.requires("devil/1.8.0")

            if self.options.get_safe("with_engine_tool_gltfc"):
                self.requires("assimp/6.0.2")
                self.requires("cgltf/1.13")

            if self.options.get_safe("with_engine_tool_gltfv"):
                self.requires("imgui/1.92.2b")
                self.requires("sdl/2.32.10")

            """
            # self.requires("lodepng/cci.20230410")
            # self.requires("nlohmann_json/3.12.0")
            """

        # POCS dependencies
        if self.options.get_safe("with_poc_clang"):
            self.requires("llvm/18.1.8")

        if self.options.get_safe("with_poc_lsp"):
            self.requires("lsp-framework/1.3.1")

        if self.options.get_safe("with_poc_protobuf"):
            self.requires("protobuf/3.9.1")

        if self.options.get_safe("with_poc_wxwidgets"):
            self.requires("wxwidgets/3.2.8")

        if self.options.get_safe("with_poc_zeromq"):
            self.requires("zeromq/4.3.5")

    def _enumerate_imgui_backends(self):
        # TODO: Check specific package versions to pick correct imgui backend version

        if "imgui" not in self.dependencies:
            return []

        backends = []

        if "glazed" in self.dependencies:
            backends.append("opengl3")

        if "glfw" in self.dependencies:
            backends.append("glfw")

        if "sdl" in self.dependencies:
            backends.append("sdl2")

        return backends

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)

        # Common
        tc.variables["XE_BUILD_VERSION"] = self.version
        tc.variables["XE_ENABLE_TESTING"] = "ON" if self.options.with_tests else "OFF"

        # IDE
        tc.variables["XE_ENABLE_IDE"] = "ON" if self.options.with_ide else "OFF"
        tc.variables["XE_ENABLE_IDE_WINLAMB"] = "ON" if self.options.get_safe("with_ide_winlamb") else "OFF"

        # Engine
        tc.variables["XE_ENABLE_ENGINE"] = "ON" if self.options.with_engine else "OFF"
        tc.variables["XE_ENABLE_ENGINE_TOOL_KTXC"] = "ON" if self.options.get_safe("with_engine_tool_ktxc") else "OFF"
        tc.variables["XE_ENABLE_ENGINE_TOOL_GLTFC"] = "ON" if self.options.get_safe("with_engine_tool_gltfc") else "OFF"
        tc.variables["XE_ENABLE_ENGINE_TOOL_GLTFV"] = "ON" if self.options.get_safe("with_engine_tool_gltfv") else "OFF"

        # POCs
        tc.variables["XE_ENABLE_POC_CLANG"] = "ON" if self.options.get_safe("with_poc_clang") else "OFF"
        tc.variables["XE_ENABLE_POC_LSP"] = "ON" if self.options.get_safe("with_poc_lsp") else "OFF"
        tc.variables["XE_ENABLE_POC_PROTOBUF"] = "ON" if self.options.get_safe("with_poc_protobuf") else "OFF"
        tc.variables["XE_ENABLE_POC_WXWIDGETS"] = "ON" if self.options.get_safe("with_poc_wxwidgets") else "OFF"
        tc.variables["XE_ENABLE_POC_ZEROMQ"] = "ON" if self.options.get_safe("with_poc_zeromq") else "OFF"

        # populate imgui backends
        imgui_backends = self._enumerate_imgui_backends()
        if len(imgui_backends) > 0:
            package_folder = self.dependencies["imgui"].package_folder
            bindings_folder_src = os.path.join(package_folder, "res", "bindings")
            bindings_folder_dest = os.path.join(self.source_folder, "src", "engine", "tools", "libxe-imgui", "src", "xe", "imgui")

            for backend in imgui_backends:
                copy(self, f"imgui_impl_{backend}*", bindings_folder_src, bindings_folder_dest)

            tc.variables["XE_ENGINE_IMGUI_BACKENDS"] = ";".join(imgui_backends)

        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
