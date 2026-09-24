import os

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.files import get, copy, rmdir
from conan.tools.gnu import PkgConfigDeps
from conan.tools.meson import Meson, MesonToolchain


class MesaConan(ConanFile):
    name = "mesa"

    description = "Mesa 3D Graphics Library — software OpenGL/Vulkan renderers for debugging and testing"
    license = "MIT"
    url = "https://mesa3d.org"

    settings = "os", "compiler", "build_type", "arch"

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_llvmpipe": [True, False],
        "with_lavapipe": [True, False],
        "with_osmesa": [True, False],
        "with_egl": [True, False],
        "with_glx": [True, False],
    }

    default_options = {
        "shared": True,
        "fPIC": True,
        "with_llvmpipe": True,
        "with_lavapipe": True,
        "with_osmesa": True,
        "with_egl": False,
        "with_glx": False,
    }

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC
            del self.options.with_glx
            del self.options.with_egl
        elif self.settings.os == "Macos":
            del self.options.with_glx

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

        has_driver = (
            self.options.with_llvmpipe
            or self.options.with_lavapipe
            or self.options.with_osmesa
        )
        if not has_driver:
            raise ConanInvalidConfiguration(
                "At least one driver must be enabled (with_llvmpipe, with_lavapipe, or with_osmesa)"
            )

    def requirements(self):
        self.requires("zlib/[>=1.2.11 <2]")
        self.requires("expat/[>=2.0 <3]")
        if self.settings.os in ("Linux", "FreeBSD"):
            self.requires("libdrm/[>=2.4.0]")

    def build_requirements(self):
        self.tool_requires("cpython/3.10.14", options={"shared": True, "with_tkinter": False})
        self.tool_requires("meson/[>=1.1.0]")
        self.tool_requires("ninja/[>=1.11.0]")
        self.tool_requires("flex/[>=2.6.0]")
        self.tool_requires("bison/[>=3.8.0]")
        self.tool_requires("pkgconf/[>=2.0.0]")

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def layout(self):
        self.folders.source = "src"
        self.folders.build = "build"

    def generate(self):
        deps = PkgConfigDeps(self)
        deps.generate()

        tc = MesonToolchain(self)

        gallium_drivers = []
        if self.options.with_llvmpipe:
            gallium_drivers.append("llvmpipe")

        vulkan_drivers = []
        if self.options.with_lavapipe:
            vulkan_drivers.append("swrast")

        tc.project_options["gallium-drivers"] = ",".join(gallium_drivers) if gallium_drivers else ""
        tc.project_options["vulkan-drivers"] = ",".join(vulkan_drivers) if vulkan_drivers else ""

        tc.project_options["osmesa"] = "true" if self.options.with_osmesa else "false"

        want_egl = self.options.get_safe("with_egl", False)
        want_glx = self.options.get_safe("with_glx", False)

        if self.settings.os != "Windows":
            tc.project_options["egl"] = "enabled" if want_egl else "disabled"
        else:
            tc.project_options["egl"] = "disabled"

        if self.settings.os == "Linux":
            if want_glx and want_egl:
                tc.project_options["glx"] = "dri"
            elif want_glx:
                tc.project_options["glx"] = "xlib"
            else:
                tc.project_options["glx"] = "disabled"
            tc.project_options["platforms"] = "x11" if want_glx else ""
        else:
            tc.project_options["glx"] = "disabled"
            tc.project_options["platforms"] = ""

        use_llvm = self.options.with_llvmpipe or self.options.with_lavapipe
        tc.project_options["llvm"] = "enabled" if use_llvm else "disabled"

        tc.project_options["shared-glapi"] = "enabled"
        tc.project_options["gles1"] = "disabled"
        tc.project_options["gles2"] = "enabled"
        tc.project_options["gbm"] = "disabled"
        tc.project_options["gallium-vdpau"] = "disabled"
        tc.project_options["gallium-va"] = "disabled"
        tc.project_options["gallium-xa"] = "disabled"
        tc.project_options["gallium-nine"] = "false"
        tc.project_options["gallium-opencl"] = "disabled"
        tc.project_options["lmsensors"] = "disabled"
        tc.project_options["video-codecs"] = ""

        tc.generate()

    def _get_python(self):
        return self.dependencies.build["cpython"].conf_info.get("user.cpython:python")

    def _install_python_deps(self):
        python = self._get_python()
        site_pkgs = os.path.join(self.build_folder, "site-packages")
        self.run(f'"{python}" -m pip install --target "{site_pkgs}" mako PyYAML')
        os.environ["PYTHONPATH"] = site_pkgs + os.pathsep + os.environ.get("PYTHONPATH", "")

    def build(self):
        self._install_python_deps()
        meson = Meson(self)
        meson.configure()
        meson.build()

    def package(self):
        meson = Meson(self)
        meson.install()

        copy(self, "LICENSE*", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))

        rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))
        rmdir(self, os.path.join(self.package_folder, "share"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "mesa")

        # GL component requires GLX (on Linux) to produce libGL
        want_glx = self.options.get_safe("with_glx", False)
        if self.options.with_llvmpipe and (want_glx or self.settings.os != "Linux"):
            gl = self.cpp_info.components["GL"]
            gl.set_property("cmake_target_name", "mesa::GL")
            gl.libs = ["GL"]
            gl.includedirs = ["include"]
            if self.settings.os in ("Linux", "FreeBSD"):
                gl.system_libs = ["pthread", "dl", "m"]

        # EGL component
        if self.options.get_safe("with_egl"):
            egl = self.cpp_info.components["EGL"]
            egl.set_property("cmake_target_name", "mesa::EGL")
            egl.libs = ["EGL"]
            egl.includedirs = ["include"]
            if self.settings.os in ("Linux", "FreeBSD"):
                egl.system_libs = ["pthread", "dl"]

        # GLESv2 component
        if self.options.get_safe("with_egl") or want_glx:
            glesv2 = self.cpp_info.components["GLESv2"]
            glesv2.set_property("cmake_target_name", "mesa::GLESv2")
            glesv2.libs = ["GLESv2"]
            glesv2.includedirs = ["include"]
            if self.settings.os in ("Linux", "FreeBSD"):
                glesv2.system_libs = ["pthread", "dl", "m"]

        # OSMesa component
        if self.options.with_osmesa:
            osmesa = self.cpp_info.components["OSMesa"]
            osmesa.set_property("cmake_target_name", "mesa::OSMesa")
            osmesa.libs = ["OSMesa"]
            osmesa.includedirs = ["include"]
            if self.settings.os in ("Linux", "FreeBSD"):
                osmesa.system_libs = ["pthread", "dl", "m"]

        # Lavapipe Vulkan ICD
        if self.options.with_lavapipe:
            lvp = self.cpp_info.components["vulkan_lvp"]
            lvp.set_property("cmake_target_name", "mesa::vulkan_lvp")
            lvp.includedirs = []
            lvp.bindirs = []
            lvp.libdirs = []
            icd_dir = os.path.join(self.package_folder, "share", "vulkan", "icd.d")
            self.runenv_info.prepend_path("VK_ICD_FILENAMES", icd_dir)
