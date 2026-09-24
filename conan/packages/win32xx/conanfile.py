from conan import ConanFile
from conan.tools.files import get, copy
from conan.tools.scm import Git
import os


class Win32xxConan(ConanFile):
    name = "win32xx"
    version = "10.2"
    description = "Header-only C++ wrapper library for the Win32 API"
    license = "MIT"
    url = "https://github.com/DavidNash2024/Win32xx"
    homepage = "https://github.com/DavidNash2024/Win32xx"
    topics = ("windows", "win32", "header-only", "gui", "mfc-alternative")
    package_type = "header-library"
    no_copy_source = True

    options = {
        "no_using_namespace": [True, False],
        "unicode": [True, False],
        "lean_and_mean": [True, False],
    }
    default_options = {
        "no_using_namespace": False,
        "unicode": True,
        "lean_and_mean": False,
    }

    def package_id(self):
        self.info.clear()

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
             src=os.path.join(self.source_folder, "include"),
             dst=os.path.join(self.package_folder, "include"))
        copy(self, "*.rc",
             src=os.path.join(self.source_folder, "include"),
             dst=os.path.join(self.package_folder, "res"))
        copy(self, "copyright.txt",
             src=os.path.join(self.source_folder, "include"),
             dst=os.path.join(self.package_folder, "licenses"))

    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        self.cpp_info.set_property("cmake_file_name", "win32xx")
        self.cpp_info.set_property("cmake_target_name", "win32xx::win32xx")
        self.cpp_info.includedirs = ["include"]

        self.cpp_info.system_libs = [
            "comctl32",
            "comdlg32",
            "shlwapi",
            "shell32",
            "ole32",
            "oleaut32",
            "uuid",
            "winspool",
            "gdi32",
        ]

        if self.options.no_using_namespace:
            self.cpp_info.defines.append("NO_USING_NAMESPACE")
        if self.options.unicode:
            self.cpp_info.defines.extend(["UNICODE", "_UNICODE"])
        if self.options.lean_and_mean:
            self.cpp_info.defines.append("WIN32_LEAN_AND_MEAN")
