from conan import ConanFile


class xenoideRecipe(ConanFile):
    name = "xenoide"
    version = "0.1.0"  # version file (source of truth); tags are v0.x.y markers
    python_requires = "xenoide-base/0.1.0"
    python_requires_extend = "xenoide-base.XenoideCMake"
    package_type = "application"
    build_info_prefix = "XE"

    # Optional metadata
    license = "MIT"
    author = "Felipe Apablaza <felipe.apablaza@protonmail.com>"
    url = "https://github.com/XenoideSoftware/xenoide"
    description = "IDE for Native C++ development"
    topics = ("ide", "c++", "development")

    options = {
        "with_ide_winlamb": [True, False],
        "with_ide_qt6": [True, False],
        "with_ide_wx3": [True, False],
        "with_ide_gtk4": [True, False],
        "with_ide_gtk3": [True, False],
    }

    default_options = {
        "with_ide_winlamb": False,
        "with_ide_qt6": False,
        "with_ide_wx3": False,
        "with_ide_gtk4": False,
        "with_ide_gtk3": False,
        # Qt/cpython options moved out of the retired unix/windows profiles.
        "qt/*:shared": True,
        "qt/*:qt5compat": True,
        "qt/*:qtshadertools": True,
        "qt/*:with_sqlite3": False,
        "cpython/*:with_gdbm": False,
        "cpython/*:with_tkinter": False,
    }

    # Sources are located in the same place as this recipe.
    exports_sources = "CMakeLists.txt", "src/*", "assets/*"

    def config_options(self):
        if self.options.with_ide_winlamb and self.settings.os != "Windows":
            self.output.warning(
                "with_ide_winlamb is only supported on Windows; disabling it")
            self.options.rm_safe("with_ide_winlamb")

    def _require_gtk4(self):
        return self.options.get_safe("with_ide_gtk4")

    def _require_gtk3(self):
        return self.options.get_safe("with_ide_gtk3")

    def _require_wx3(self):
        return self.options.get_safe("with_ide_wx3")

    def requirements(self):
        self.requires("gsl-lite/1.1.0")

        if self.options.get_safe("with_ide_winlamb"):
            self.requires("winlamb/2026.06.24")
            self.requires("scintilla3/3.7.6")

        if self.options.get_safe("with_ide_qt6"):
            self.requires("qt/6.8.3")
            self.requires("scintilla/5.5.7")
            self.requires("lexilla/5.4.6")

        if self._require_gtk4():
            self.requires("gtk/system", options={"version": 4})

        if self._require_gtk3():
            self.requires("gtk/system", options={"version": 3})

        if self._require_wx3():
            self.requires("wxwidgets/3.3.3")

    def configure(self):
        if self.options.get_safe("with_ide_wx3"):
            self.options["wxwidgets"].stc = True
            self.options["wxwidgets"].gtk = "3"

    def xenoide_toolchain(self, tc):
        tc.variables["XE_ENABLE_IDE_WINLAMB"] = "ON" if self.options.get_safe("with_ide_winlamb") else "OFF"
        tc.variables["XE_ENABLE_IDE_QT6"] = "ON" if self.options.get_safe("with_ide_qt6") else "OFF"
        tc.variables["XE_ENABLE_IDE_WX3"] = "ON" if self.options.get_safe("with_ide_wx3") else "OFF"
        tc.variables["XE_ENABLE_IDE_GTK4"] = "ON" if self.options.get_safe("with_ide_gtk4") else "OFF"
        tc.variables["XE_ENABLE_IDE_GTK3"] = "ON" if self.options.get_safe("with_ide_gtk3") else "OFF"
