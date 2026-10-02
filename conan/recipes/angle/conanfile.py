import os

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.env import Environment
from conan.tools.files import copy, save
from conan.tools.scm import Git


class AngleConan(ConanFile):
    name = "angle"

    description = "Google ANGLE — OpenGL ES implementation on top of Vulkan, D3D, Metal, and desktop GL"
    license = "BSD-3-Clause"
    url = "https://chromium.googlesource.com/angle/angle"

    settings = "os", "compiler", "build_type", "arch"

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_vulkan": [True, False],
        "with_gl": [True, False],
        "with_metal": [True, False],
        "with_d3d11": [True, False],
    }

    default_options = {
        "shared": True,
        "fPIC": True,
        "with_vulkan": True,
        "with_gl": False,
        "with_metal": False,
        "with_d3d11": False,
    }

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC
            self.options.with_d3d11 = True
        else:
            del self.options.with_d3d11

        if self.settings.os != "Macos":
            del self.options.with_metal
        else:
            self.options.with_metal = True

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

        has_backend = self.options.with_vulkan or self.options.with_gl
        if self.settings.os == "Windows":
            has_backend = has_backend or self.options.get_safe("with_d3d11")
        if self.settings.os == "Macos":
            has_backend = has_backend or self.options.get_safe("with_metal")

        if not has_backend:
            raise ConanInvalidConfiguration(
                "At least one rendering backend must be enabled "
                "(with_vulkan, with_gl, with_d3d11, or with_metal)"
            )

    def build_requirements(self):
        self.tool_requires("ninja/[>=1.11.0]")
        self.tool_requires("pkgconf/[>=2.0.0]")

    def _chromium_branch(self):
        return self.version.replace("_", "/")

    def _depot_tools_dir(self):
        return os.path.join(self.source_folder, "depot_tools")

    def _apply_depot_env(self):
        depot_dir = self._depot_tools_dir()
        # gn binary lives in buildtools/<platform>
        buildtools_bin = os.path.join(self.source_folder, "buildtools", "linux64")
        path = os.environ.get("PATH", "")
        for d in (buildtools_bin, depot_dir):
            if d not in path:
                path = d + os.pathsep + path
        os.environ["PATH"] = path
        os.environ["DEPOT_TOOLS_UPDATE"] = "0"
        os.environ["GCLIENT_PY3"] = "1"

    def source(self):
        branch = self._chromium_branch()
        git = Git(self)
        git.clone(url="https://chromium.googlesource.com/angle/angle",
                  target=".", args=["--depth", "1", "--branch", branch])

        depot_git = Git(self, folder="depot_tools")
        depot_git.clone(url="https://chromium.googlesource.com/chromium/tools/depot_tools.git",
                        target=".", args=["--depth", "1"])

        gclient_content = (
            'solutions = [{\n'
            '  "name": ".",\n'
            '  "url": "https://chromium.googlesource.com/angle/angle.git",\n'
            '  "managed": False,\n'
            '  "custom_deps": {},\n'
            '}]\n'
        )
        save(self, os.path.join(self.source_folder, ".gclient"), gclient_content)

        self._apply_depot_env()
        self.run("gclient sync --no-history --shallow")

    def layout(self):
        self.folders.source = "src"
        self.folders.build = "build"

    def _gn_args(self):
        args = []

        is_debug = str(self.settings.build_type) == "Debug"
        args.append(f"is_debug={'true' if is_debug else 'false'}")

        args.append(f"angle_enable_vulkan={'true' if self.options.with_vulkan else 'false'}")
        args.append(f"angle_enable_gl={'true' if self.options.with_gl else 'false'}")
        args.append("angle_enable_d3d9=false")
        args.append(f"angle_enable_d3d11={'true' if self.options.get_safe('with_d3d11', False) else 'false'}")
        args.append(f"angle_enable_metal={'true' if self.options.get_safe('with_metal', False) else 'false'}")
        args.append("angle_enable_null=false")
        args.append("angle_enable_wgpu=false")
        args.append("angle_enable_swiftshader=false")

        args.append("build_angle_deqp_tests=false")
        args.append("angle_build_all=false")

        # Use chromium's bundled clang (ANGLE is designed for it)
        args.append("is_clang=true")
        args.append("use_custom_libcxx=false")
        args.append("treat_warnings_as_errors=false")

        args.append(f'install_prefix="{self.package_folder}"')

        return " ".join(args)

    def _setup_build_tools(self):
        """Create a bin/ dir with pkg-config symlink and ensure ninja/gn are findable."""
        bin_dir = os.path.join(self.build_folder, "bin")
        os.makedirs(bin_dir, exist_ok=True)

        # Create a pkg-config symlink pointing to Conan's pkgconf
        pkg_config_link = os.path.join(bin_dir, "pkg-config")
        if not os.path.exists(pkg_config_link):
            pkgconf_info = self.dependencies.build["pkgconf"]
            pkgconf_path = os.path.join(pkgconf_info.package_folder, "bin", "pkgconf")
            os.symlink(pkgconf_path, pkg_config_link)

        # Create a ninja symlink pointing to Conan's ninja (depot_tools' wrapper won't find it)
        ninja_link = os.path.join(bin_dir, "ninja")
        if not os.path.exists(ninja_link):
            ninja_info = self.dependencies.build["ninja"]
            ninja_path = os.path.join(ninja_info.package_folder, "bin", "ninja")
            os.symlink(ninja_path, ninja_link)

        # Put our bin/ first so pkg-config and ninja are found before depot_tools wrappers
        os.environ["PATH"] = bin_dir + os.pathsep + os.environ.get("PATH", "")

    def build(self):
        self._apply_depot_env()

        # Bootstrap depot_tools in the build folder (needed after Conan copies sources)
        depot_dir = self._depot_tools_dir()
        bootstrap = os.path.join(depot_dir, "ensure_bootstrap")
        if os.path.isfile(bootstrap):
            self.run(f'"{bootstrap}"')

        self._setup_build_tools()

        out_dir = os.path.join(self.source_folder, "out", "Conan")
        gn_args = self._gn_args()

        self.output.info(f"GN args: {gn_args}")
        self.run(f'gn gen "{out_dir}" --root="{self.source_folder}" --args=\'{gn_args}\'')
        self.run(f'ninja -C "{out_dir}" libEGL libGLESv2')

    def package(self):
        copy(self, "LICENSE", src=self.source_folder,
             dst=os.path.join(self.package_folder, "licenses"))

        include_src = os.path.join(self.source_folder, "include")
        include_dst = os.path.join(self.package_folder, "include")
        for subdir in ("EGL", "GLES", "GLES2", "GLES3", "KHR"):
            copy(self, "*.h", src=os.path.join(include_src, subdir),
                 dst=os.path.join(include_dst, subdir))
        copy(self, "angle_gl.h", src=include_src, dst=include_dst)
        copy(self, "export.h", src=include_src, dst=include_dst)

        out_dir = os.path.join(self.source_folder, "out", "Conan")
        for pattern in ("libEGL.so*", "libGLESv2.so*", "libEGL.dylib", "libGLESv2.dylib"):
            copy(self, pattern, src=out_dir,
                 dst=os.path.join(self.package_folder, "lib"), keep_path=False)
        for pattern in ("libEGL.a", "libGLESv2.a", "*.lib"):
            copy(self, pattern, src=out_dir,
                 dst=os.path.join(self.package_folder, "lib"), keep_path=False)
        for pattern in ("libEGL.dll", "libGLESv2.dll"):
            copy(self, pattern, src=out_dir,
                 dst=os.path.join(self.package_folder, "bin"), keep_path=False)

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "angle")

        egl = self.cpp_info.components["EGL"]
        egl.set_property("cmake_target_name", "angle::EGL")
        egl.libs = ["EGL"]
        egl.includedirs = ["include"]
        if self.settings.os in ("Linux", "FreeBSD"):
            egl.system_libs = ["pthread", "dl"]
        elif self.settings.os == "Windows":
            egl.system_libs = ["d3d9", "dxguid", "gdi32", "user32"]

        glesv2 = self.cpp_info.components["GLESv2"]
        glesv2.set_property("cmake_target_name", "angle::GLESv2")
        glesv2.libs = ["GLESv2"]
        glesv2.includedirs = ["include"]
        glesv2.requires = ["EGL"]
        if self.settings.os in ("Linux", "FreeBSD"):
            glesv2.system_libs = ["pthread", "dl", "m"]
        elif self.settings.os == "Windows":
            if self.options.get_safe("with_d3d11"):
                glesv2.system_libs.extend(["d3d11", "dxgi", "dxguid"])
