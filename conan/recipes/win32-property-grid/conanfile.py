from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy
from conan.tools.scm import Git
from os.path import join

import os
import shutil


class Win32PropertyGridConanfile(ConanFile):
    name = "win32-property-grid"
    version = "2026.04.24"
    description = "A property grid control for Win32 (XenoideSoftware fork)"
    license = "MIT"
    homepage = "https://github.com/XenoideSoftware/win32-property-grid"
    url = "https://github.com/XenoideSoftware/win32-property-grid"
    topics = ("windows", "win32", "property-grid", "gui")

    settings = "os", "compiler", "build_type", "arch"

    exports_sources = "conandata.yml", "CMakeLists.txt"

    def source(self):
        src = self.conan_data["sources"][self.version]
        git = Git(self)
        git.clone(url=src["url"], target=".")
        git.checkout(commit=src["commit"])

    def layout(self):
        cmake_layout(self, src_folder="src")

    def generate(self):
        base = os.path.dirname(self.source_folder)
        cmakelists_src = os.path.join(base, "CMakeLists.txt")
        if os.path.isfile(cmakelists_src):
            shutil.copy2(cmakelists_src, self.source_folder)

        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "propertyGrid.h",
             self.source_folder,
             join(self.package_folder, "include"))

        copy(self, "*.lib", self.build_folder, join(self.package_folder, "lib"), keep_path=False)
        copy(self, "*.a", self.build_folder, join(self.package_folder, "lib"), keep_path=False)
        copy(self, "*.pdb", self.build_folder, join(self.package_folder, "lib"), keep_path=False)

    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.libs = ["win32-property-grid"]
        self.cpp_info.system_libs = ["user32", "comctl32", "gdi32"]
        self.cpp_info.set_property("cmake_file_name", "win32-property-grid")
        self.cpp_info.set_property("cmake_target_name", "win32-property-grid::win32-property-grid")
