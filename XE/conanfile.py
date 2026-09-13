from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy

import os

class xeRecipe(ConanFile):
    name = "xe"
    version = "1.0"
    package_type = "application"

    # Optional metadata
    license = "<Put the package license here>"
    author = "<Put your name here> <And your email here>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of xenoide package here>"
    topics = ("<Put some tag here>", "<here>", "<and here>")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    options = {
        "with_docs": [True, False],
    }
    default_options = {
        "with_docs": False,
    }

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*"

    def layout(self):
        cmake_layout(self)

    def build_requirements(self):
        if self.options.with_docs:
            # Doxygen is consumed at build time only — never linked, never
            # shipped — so it goes through tool_requires rather than the
            # regular requires() list. Pinning to the 1.9.x line keeps the
            # config syntax in Doxyfile.in stable.
            self.tool_requires("doxygen/[>=1.9 <2]")

    def requirements(self):
        self.requires("imgui/1.92.2b")

        # self.requires("assimp/6.0.2")
        self.requires("glfw/3.4")
        # self.requires("cgltf/1.13")
        self.requires("fmt/[>=11 <12]")
        # self.requires("lodepng/cci.20230410")
        self.requires("ms-gsl/4.2.0")
        # self.requires("devil/1.8.0")
        self.requires("glm/1.0.1")
        # self.requires("nlohmann_json/3.12.0")
        # self.requires("vulkan-loader/1.4.313.0")
        
        """
        compiler = self.settings.get_safe("compiler")
        compiler_version = self.settings.get_safe("compiler.version")
        if not (compiler == "gcc" and str(compiler_version).startswith("5")):
            self.requires("ktx/4.4.2")
        """

        self.requires("cxxopts/3.3.1")
        self.requires("tl-expected/1.2.0")
        self.requires("backport-cpp/1.2.0")
        self.requires("catch2/3.14.0")
        self.requires("glazed/1.0.0", options={"language": "both", "apis": "gl:4.6,gles2:3.2,gl_compat:2.1"})
        # NOTE: sdl2 and sdl3 both don't build under ArchLinux
        # Neither with gcc-x86 (gcc5)
        self.requires("sdl/2.32.10")
        
    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        if self.options.with_docs:
            # Hoist the conan-side opt-in into the engine-wide CMake option
            # so a single `-o with_docs=True` is all the user needs to type.
            tc.variables["XE_DEV_DOCS"] = "ON"
        tc.generate()

        # imgui backends
        package_folder = self.dependencies["imgui"].package_folder
        bindings_folder_src = os.path.join(package_folder, "res", "bindings")
        bindings_folder_dest = os.path.join(self.source_folder, "src", "xe-gltf-view", "src", "bindings")

        backends = ["glfw", "opengl3", "sdl2", "sdl3"]
        for backend in backends:
            copy(self, f"*{backend}*", bindings_folder_src, bindings_folder_dest)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
