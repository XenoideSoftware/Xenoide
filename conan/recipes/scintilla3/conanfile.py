import os
from os.path import join

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.files import copy, get, patch, replace_in_file
from conan.tools.microsoft import VCVars


class Scintilla3Conanfile(ConanFile):
    name = "scintilla3"
    version = "3.7.6"
    description = "Scintilla - A free source code editing component"
    license = "HPND"
    homepage = "https://www.scintilla.org/"
    url = "https://www.scintilla.org/"

    settings = "os", "compiler", "build_type", "arch"

    exports_sources = "conandata.yml", "win98.patch"

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def layout(self):
        self.folders.source = "src"
        self.folders.build = "build"

        # Editable mode: headers and built DLLs live inside the extracted source
        # tree. Without these, Conan defaults to <recipe>/include which never
        # exists and breaks consumers' find_package(scintilla3).
        self.cpp.source.includedirs = ["include", "src"]
        self.cpp.source.bindirs = ["bin"]
        self.cpp.source.libdirs = []

    def generate(self):
        # Activate the matching VS environment so nmake's spawned `link.exe`
        # resolves to MSVC's linker rather than /usr/bin/link from MSYS
        # coreutils, which shadows it on bash-on-Windows shells.
        if self.settings.compiler == "msvc":
            VCVars(self).generate()

    def validate(self):
        if self.settings.os != "Windows":
            raise ConanInvalidConfiguration(
                "scintilla3 only ships Win32 build files (win32/makefile, "
                "win32/scintilla.mak); other platforms are not supported."
            )
        if self.settings.compiler not in ("gcc", "msvc"):
            raise ConanInvalidConfiguration(
                f"scintilla3: unsupported compiler '{self.settings.compiler}'. "
                "Only msvc (nmake) and gcc/mingw (mingw32-make) are supported."
            )

    def build(self):
        win32 = join(self.source_folder, "win32")

        # UniConversion.h declares std::string FixInvalidUTF8(...) without
        # including <string>. Modern MSVC's STL no longer pulls <string> in
        # transitively, so the win32 sources fail to compile. Inject the
        # missing include at the top of the header. strict=False keeps it
        # idempotent across rebuilds.
        replace_in_file(
            self, join(self.source_folder, "src", "UniConversion.h"),
            "#define UNICONVERSION_H",
            "#define UNICONVERSION_H\n#include <string>",
            strict=False,
        )

        if self.settings.compiler == "gcc":
            # MinGW / TDM-GCC. The Win98 SDK patch lowers _WIN32_WINNT to 0x0410
            # so old SDK headers compile; it would break the modern MSVC SDK,
            # which is why it is applied only on the GCC path.
            patch(self, base_path=self.source_folder,
                  patch_file=join(self.recipe_folder, "win98.patch"))
            self.run("mingw32-make -j4", cwd=win32)
        else:
            # Force LD=link.exe — when invoked from a shell where /usr/bin/link
            # (GNU coreutils) is on PATH ahead of MSVC, bare `link` resolves to
            # the wrong binary and breaks the final DLL link step.
            args = ["nmake", "-f", "scintilla.mak", "LD=link.exe"]
            if self.settings.build_type == "Debug":
                args.append("DEBUG=1")
            # Clean any stale .obj from a prior build whose architecture might
            # not match the currently-activated VCVars (e.g. switching between
            # an x86 dev shell and an x64 Conan profile).
            self.run("nmake -f scintilla.mak clean", cwd=win32)
            self.run(" ".join(args), cwd=win32)

    def package(self):
        copy(self, "*.h",
             join(self.source_folder, "include"),
             join(self.package_folder, "include"))
        copy(self, "*.h",
             join(self.source_folder, "src"),
             join(self.package_folder, "include"))
        copy(self, "*.dll",
             join(self.source_folder, "bin"),
             join(self.package_folder, "bin"),
             keep_path=False)

    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.bindirs = ["bin"]
        self.cpp_info.libdirs = []
        self.cpp_info.libs = []
        self.cpp_info.set_property("cmake_target_name", "scintilla3")

        self.runenv_info.prepend_path("PATH", os.path.join(self.package_folder, "bin"))
