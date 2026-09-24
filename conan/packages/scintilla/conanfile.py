from conan import ConanFile
from conan.tools.files import get, copy
import os
from conan.errors import ConanInvalidConfiguration
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.env import VirtualBuildEnv, Environment
from conan.tools.files import get, replace_in_file, patch
from conan import tools
from os.path import join

import sys
import subprocess
import shutil

# other custom settings
scintilla_qt_modules = [
    "ScintillaEdit", 
    # "ScintillaEditBase"
]

class ScintillaConanfile(ConanFile):
    name = "scintilla"
    version = "5.5.7"
    description = "Scintilla - A free source code editing component"
    license = "HPND"
    homepage = "https://www.scintilla.org/"
    url = "https://www.scintilla.org/"

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    exports_sources = "conandata.yml"

    def source(self):
        print(f'*** source() source_folder={self.source_folder} ***')

        # Get source URL from conandata.yml
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def requirements(self):
        # qt5compat is required to build Scintilla with Qt6
        self.requires("qt/[>=6.0.0]")

    def build_requirements(self):
        self.tool_requires("qt/<host_version>")

        # we need cpython to generate the CMakeLists.txt files to build Scintilla 
        self.tool_requires("cpython/3.12.7", options={"shared": True})
        
    def layout(self):
        cmake_layout(self, src_folder="src")

    def _write_file(self, filename, content):
        with open(filename, "w", encoding="utf-8", newline="\n") as f:
            f.write(content)


    def _get_python_exe(self):
        """Get the Python executable path from the cpython build dependency."""
        cpython_dep = self.dependencies.build["cpython"]
        cpython_root = cpython_dep.package_folder

        if self.settings.os == "Windows":
            python_exe = os.path.join(cpython_root, "bin", "python.exe")
        else:
            python_exe = os.path.join(cpython_root, "bin", "python3")

        if not os.path.isfile(python_exe):
            self.output.warn(f"Python executable not found at {python_exe}, falling back to system python3")
            python_exe = "python3"

        return python_exe


    def _get_qmake2cmake_exe(self):
        """Get the qmake2cmake executable path from the cpython build dependency."""
        cpython_root = self.dependencies.build["cpython"].package_folder

        if self.settings.os == "Windows":
            qmake2cmake_exe = os.path.join(cpython_root, "bin", "Scripts", "qmake2cmake.exe")
        else:
            qmake2cmake_exe = os.path.join(cpython_root, "bin", "qmake2cmake")

        return qmake2cmake_exe

    def _get_cpython_env(self):
        """Create an Environment with cpython's shared library path."""
        cpython_lib = os.path.join(self.dependencies.build["cpython"].package_folder, "lib")
        env = Environment()
        env.prepend_path("LD_LIBRARY_PATH", cpython_lib)
        return env.vars(self)

    def _call_widget_gen(self):
        # Run the WidgetGen.py script to generate the Editor API in a cross-platform way
        script = os.path.join(self.source_folder, "qt", "ScintillaEdit", "WidgetGen.py")

        if not os.path.isfile(script):
            raise ConanInvalidConfiguration(f"Could not locate WidgetGen.py in {script}")

        python_exe = self._get_python_exe()
        self.output.info(f"Generating Scintilla Qt editor API using {python_exe}")
        with self._get_cpython_env().apply():
            self.run(f'"{python_exe}" "{os.path.basename(script)}"', cwd=os.path.dirname(script))


    def _generate_scintilla_cmakelists_widgetgen(self):
        python_exe = self._get_python_exe()

        # Run the WidgetGen.py script to generate the Editor API
        script = os.path.join(self.source_folder, "qt", "ScintillaEdit", "WidgetGen.py")
        with self._get_cpython_env().apply():
            self.run(f'"{python_exe}" "{os.path.basename(script)}"', cwd=os.path.dirname(script))

            # Install qmake2cmake into the cpython dependency's environment
            self.run(f'"{python_exe}" -m pip install qmake2cmake', cwd=os.path.dirname(script))

            # run qmake2cmake
            qmake2cmake_exe = self._get_qmake2cmake_exe()
            self.run(f'"{qmake2cmake_exe}" --min-qt-version 6.8 {self.source_folder}/qt/ScintillaEdit/ScintillaEdit.pro')

        scintilla_edit_cmakelists = os.path.join(self.source_folder, "qt", "ScintillaEdit", "CMakeLists.txt")
        # replace_in_file(self, scintilla_edit_cmakelists, "Qt::", "Qt${QT_VERSION_MAJOR}::")
        replace_in_file(self, scintilla_edit_cmakelists, "Qt::", "Qt6::")

    def _generate_scintilla_cmakelists(self):
        python_exe = self._get_python_exe()
        qmake2cmake_exe = self._get_qmake2cmake_exe()

        with self._get_cpython_env().apply():
            # Install qmake2cmake into the cpython dependency's environment
            self.run(f'"{python_exe}" -m pip install qmake2cmake')

            for module in scintilla_qt_modules:
                pro_file = os.path.join(self.source_folder, "qt", module, f"{module}.pro")
                module_dir = os.path.join(self.source_folder, "qt", module)
                if not os.path.isfile(pro_file):
                    self.output.warn(f".pro file not found for module {module}: {pro_file}")
                    continue
                self.output.info(f"Generating CMakeLists.txt for {module} using qmake2cmake")
                self.run(f'"{qmake2cmake_exe}" --min-qt-version 6.8 "{pro_file}"', cwd=module_dir)

    def _patch_generated_cmakelists(self):
        """Replace Qt:: with Qt6:: in the generated CMakeLists.txt files.

        qmake2cmake generates target references using the Qt:: namespace prefix,
        but the Conan-provided Qt package exposes targets under the Qt6:: namespace.
        """
        for module in scintilla_qt_modules:
            cmakelists_path = os.path.join(self.source_folder, "qt", module, "CMakeLists.txt")
            if not os.path.isfile(cmakelists_path):
                self.output.warn(f"CMakeLists.txt not found for module {module}: {cmakelists_path}")
                continue
            self.output.info(f"Patching CMakeLists.txt for {module}: replacing Qt:: with Qt6::")
            replace_in_file(self, cmakelists_path, "Qt::", "Qt6::")

    # The purpose of generate() is to prepare the build, generating the necessary files, such as
    # Files containing information to locate the dependencies, environment activation scripts,
    # and specific build system files among others
    def generate(self):
        self._call_widget_gen()
        self._generate_scintilla_cmakelists()
        self._patch_generated_cmakelists()

        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.generate()

    # This method is used to build the source code of the recipe using the desired commands.
    def build(self):
        print(f'*** build(): source_folder= {self.source_folder} ***')
        print(f'*** build(): build_folder= {self.build_folder} ***')

        for module in scintilla_qt_modules:
            module_folder = os.path.join(self.source_folder, "qt", module)

            print(f"Building module {module} in {module_folder}")

            cmake = CMake(self)
            cmake.configure(build_script_folder=module_folder)
            cmake.build()

    # The actual creation of the package, once it's built, is done in the package() method.
    # Using the copy() method from tools.files, artifacts are copied
    # from the build folder to the package folder
    def package(self):
        print(f'*** package(): source_folder= {self.source_folder} ***')
        print(f'*** package(): build_folder= {self.build_folder} ***')

        cmake = CMake(self)
        cmake.install()

        # Copy headers
        copy(self, "*.h", join(self.source_folder, "include"), join(self.package_folder, "include"))
        copy(self, "*.h", join(self.source_folder, "src"), join(self.package_folder, "include"))
        copy(self, "*.h", join(self.source_folder, "qt", "ScintillaEditBase"), join(self.package_folder, "include"))
        copy(self, "*.h", join(self.source_folder, "qt", "ScintillaEdit"), join(self.package_folder, "include"))

        # Copy built libraries as fallback (the generated CMakeLists may lack install rules)
        copy(self, "*.lib", self.build_folder, join(self.package_folder, "lib"), keep_path=False)
        copy(self, "*.dll", self.build_folder, join(self.package_folder, "bin"), keep_path=False)
        copy(self, "*.a", self.build_folder, join(self.package_folder, "lib"), keep_path=False)
        copy(self, "*.so*", self.build_folder, join(self.package_folder, "lib"), keep_path=False)
        copy(self, "*.dylib", self.build_folder, join(self.package_folder, "lib"), keep_path=False)

    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.bindirs = ["bin"]
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.libs = scintilla_qt_modules
        self.cpp_info.set_property("cmake_target_name", "scintilla")

        # not default in conan2
        self.runenv_info.prepend_path("PATH", os.path.join(self.package_folder, "bin"))
