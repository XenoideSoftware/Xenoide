from conan import ConanFile
from conan.tools.files import get, copy
from conan.tools.scm import Git
import os


class WinLambConan(ConanFile):
    name = "winlamb"
    version = "2026.06.24"
    description = "Header-only C++ library for Win32 development using C++11 lambdas"
    license = "MIT"
    url = "https://github.com/XenoideSoftware/winlamb"
    homepage = "https://github.com/XenoideSoftware/winlamb"
    topics = ("windows", "win32", "header-only", "gui")
    package_type = "header-library"
    no_copy_source = True

    exports = "cmake/*"

    def source(self):
        src = self.conan_data["sources"][self.version]
        if "commit" in src:
            git = Git(self)
            git.clone(url=src["url"], target=".")
            git.checkout(commit=src["commit"])
        else:
            get(self, **src, strip_root=True)

    def package(self):
        copy(self, "*.h",
             src=self.source_folder,
             dst=os.path.join(self.package_folder, "include", "winlamb"))
        copy(self, "*.manifest",
             src=self.source_folder,
             dst=os.path.join(self.package_folder, "res"))
        copy(self, "*.cmake",
             src=os.path.join(self.recipe_folder, "cmake"),
             dst=os.path.join(self.package_folder, "lib", "cmake", "winlamb"))

    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        self.cpp_info.set_property("cmake_file_name", "winlamb")
        self.cpp_info.set_property("cmake_target_name", "winlamb::winlamb")
        self.cpp_info.set_property(
            "cmake_build_modules",
            [os.path.join("lib", "cmake", "winlamb", "winlamb-extras.cmake")],
        )
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.system_libs = ["comctl32"]
