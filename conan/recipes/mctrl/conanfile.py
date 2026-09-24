from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy
from conan.tools.scm import Git
from os.path import join

import os


class MCtrlConanfile(ConanFile):
    name = "mctrl"
    version = "0.11.5-xenoide"
    description = "mCtrl - additional native Win32 controls (XenoideSoftware fork)"
    license = "LGPL-2.1-or-later"
    homepage = "https://github.com/XenoideSoftware/mctrl"
    url = "https://github.com/XenoideSoftware/mctrl"
    topics = ("windows", "win32", "controls", "gui")

    settings = "os", "compiler", "build_type", "arch"

    options = {
        "unicode": [True, False],
    }
    default_options = {
        "unicode": False,
    }

    exports_sources = "conandata.yml"

    def source(self):
        src = self.conan_data["sources"][self.version]
        git = Git(self)
        git.clone(url=src["url"], target=".")
        git.checkout(commit=src["commit"])

    def layout(self):
        cmake_layout(self, src_folder="src")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        # Bundled hsluv-c declares cmake_minimum_required(3.1); silence the removal error.
        tc.cache_variables["CMAKE_POLICY_VERSION_MINIMUM"] = "3.5"
        tc.cache_variables["MCTRL_BUILD_EXAMPLES"] = "OFF"
        tc.cache_variables["MCTRL_BUILD_TESTS"] = "OFF"
        tc.cache_variables["MCTRL_BUILD_UNICODE"] = "ON" if self.options.unicode else "OFF"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "*.h",
             join(self.source_folder, "include"),
             join(self.package_folder, "include"))

        copy(self, "*.lib", self.build_folder, join(self.package_folder, "lib"), keep_path=False)
        copy(self, "*.dll", self.build_folder, join(self.package_folder, "bin"), keep_path=False)
        copy(self, "*.pdb", self.build_folder, join(self.package_folder, "bin"), keep_path=False)
        copy(self, "*.a", self.build_folder, join(self.package_folder, "lib"), keep_path=False)
        copy(self, "*.so*", self.build_folder, join(self.package_folder, "lib"), keep_path=False)
        copy(self, "*.dylib", self.build_folder, join(self.package_folder, "lib"), keep_path=False)

    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.bindirs = ["bin"]
        self.cpp_info.libs = ["mCtrl"]
        self.cpp_info.set_property("cmake_file_name", "mctrl")
        self.cpp_info.set_property("cmake_target_name", "mctrl::mctrl")

        self.runenv_info.prepend_path("PATH", os.path.join(self.package_folder, "bin"))
