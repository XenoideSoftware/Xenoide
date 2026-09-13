from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.env import VirtualRunEnv

class xenoideRecipe(ConanFile):
    name = "xenoide"
    version = "1.0"
    package_type = "application"

    # Optional metadata
    license = "<Put the package license here>"
    author = "<Put your name here> <And your email here>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of xenoide package>"
    topics = ("<Put some tag here>", "<here>", "<and here>")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    # Build options
    options = {
        "with_wxwidgets": [True, False],
        "with_qt6": [True, False],
        "with_tests": [True, False],
        "with_pocs": [True, False],
        "with_winlamb": [True, False],
        "with_win32xx": [True, False],
    }
    default_options = {
        "with_wxwidgets": False,
        "with_qt6": False,
        "with_tests": False,
        "with_pocs": False,
        "with_winlamb": False,
        "with_win32xx": False,
        "zeromq/*:encryption": "tweetnacl",
    }

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*"

    def layout(self):
        cmake_layout(self)

    def build_requirements(self):
        pass
        # self.tool_requires("protobuf/3.9.1")

    def requirements(self):
        # TODO: Skipped bc of huge compilation times. Put it behind an option
        # self.requires("llvm/18.1.8")

        # self.requires("zmqpp/4.2.0")
        # self.requires("protobuf/3.9.1")

        # TODO: scintilla3 requires mingw32-make to build. Fix it
        self.requires("scintilla3/3.7.6")
        self.requires("gsl-lite/1.0.1")
        # self.requires("fruit/3.7.1", options={"with_boost": False})

        # self.requires("spdlog/1.17.0")

        # TODO: Bring back the recipe
        # self.requires("lsp-framework/1.3.1")

        if self.options.with_tests:
            self.requires("catch2/3.7.1")
        
        if self.options.with_qt6:
            self.requires("qt/6.8.3")
            self.requires("scintilla/5.5.7")
            self.requires("lexilla/5.4.6")

        # TODO: Remove before merging into master
        self.requires("winlamb/2026.06.24")
        
        if self.options.with_winlamb:
            self.requires("winlamb/2026.06.24")

        if self.options.with_win32xx:
            self.requires("win32xx/10.2")

        if self.options.with_wxwidgets:
            if self.settings.os != "Windows":
                self.requires("gtk/system", options={"version" : 3})
            
            self.requires("wxwidgets/3.2.8")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.variables["XENOIDE_UI_WX"] = self.options.with_wxwidgets
        tc.variables["XENOIDE_UI_QT6"] = self.options.with_qt6
        tc.variables["XENOIDE_TESTS"] = self.options.with_tests
        tc.variables["XENOIDE_POCS"] = self.options.with_pocs
        tc.variables["XENOIDE_UI_WINLAMB"] = self.options.with_winlamb
        tc.variables["XENOIDE_UI_WIN32XX"] = self.options.with_win32xx

        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def configure(self):
        if self.options.with_wxwidgets:
            self.options["wxwidgets"].stc = True
            self.options["wxwidgets"].gtk = "3"
