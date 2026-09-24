import os
import textwrap

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy, get, replace_in_file
from conan.tools.scm import Version


class ProtobufConan(ConanFile):
    name = "protobuf"
    license = "BSD-3-Clause"
    homepage = "https://github.com/protocolbuffers/protobuf"
    url = "https://github.com/protocolbuffers/protobuf"
    description = "Protocol Buffers (pre-abseil 2.x / 3.x branches), curated for the Xenoide build host."
    topics = ("protobuf", "serialization", "rpc", "protocol-buffers")

    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }

    # Use `exports` (not `exports_sources`) so the recipe's CMake build
    # module is shipped to the cache but is *not* copied into the source
    # folder before source(). Protobuf's own tarball also contains a
    # top-level `cmake/` directory, and we don't want extraction to clobber
    # ours (or vice versa).
    exports = "cmake/*"

    @property
    def _is_v2(self):
        return Version(self.version) < "3.0.0"

    @property
    def _is_msvc(self):
        return self.settings.compiler == "msvc"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def validate(self):
        # The `cmake/` subfolder shipped with protobuf 2.x targets MSVC only.
        # On non-Windows hosts upstream expects the autotools path, which
        # this recipe deliberately does not wire up.
        if self._is_v2 and self.settings.os != "Windows":
            raise ConanInvalidConfiguration(
                "protobuf 2.x is supported only on Windows by this recipe; "
                "upstream relies on autotools for Unix-like systems."
            )

    def layout(self):
        # `src_folder="src"` keeps the extracted protobuf tree fully
        # isolated from the build folder. With the default cmake_layout()
        # the source ends up at the recipe root and conan's `build/<cfg>`
        # subfolders collide with files protobuf's own tarball lays down.
        cmake_layout(self, src_folder="src")

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.cache_variables["protobuf_BUILD_TESTS"] = "OFF"
        tc.cache_variables["protobuf_BUILD_EXAMPLES"] = "OFF"
        tc.cache_variables["protobuf_BUILD_PROTOC_BINARIES"] = "ON"
        tc.cache_variables["protobuf_WITH_ZLIB"] = "OFF"
        tc.cache_variables["protobuf_MSVC_STATIC_RUNTIME"] = "OFF"
        # CMake 4 removed legacy compatibility; pin a floor old protobuf
        # CMakeLists files satisfy.
        tc.cache_variables["CMAKE_POLICY_VERSION_MINIMUM"] = "3.5"
        tc.generate()

    def _patch_sources(self):
        # MinGW toolchains paired with GCC 5.x ship MSVCRT headers that do
        # not expose the wide-character CRT entry points (`_wfopen`,
        # `_wopen`, `_wstat`, ...) to the global namespace under
        # `-std=gnu++NN`. Protobuf's `io_win32.cc` calls them as
        # `::_wfopen(...)`, so the build fails with
        #     error: '::_wfopen' has not been declared
        # The same TU compiles cleanly on MSVC because Microsoft's CRT
        # headers always declare them in `::`.
        #
        # Upstream protobuf restructured this file properly around 3.13;
        # for the curated 3.x versions in this recipe we forward-declare
        # the symbols ourselves under `__MINGW32__`. The patch is a no-op
        # on MSVC and has no effect on protobuf 2.x (which does not ship
        # `io_win32.cc`).
        io_win32 = os.path.join(
            self.source_folder, "src", "google", "protobuf", "io", "io_win32.cc"
        )
        if not os.path.exists(io_win32):
            return

        forward_decls = textwrap.dedent("""\
            // === Xenoide local patch for older MinGW toolchains ===
            #if defined(__MINGW32__) || defined(__MINGW64__)
            #include <sys/stat.h>
            extern "C" {
              FILE*   _wfopen (const wchar_t* filename, const wchar_t* mode);
              int     _wopen  (const wchar_t* filename, int oflag, ...);
              int     _wstat  (const wchar_t* path, struct _stat* buffer);
              int     _waccess(const wchar_t* path, int mode);
              int     _wmkdir (const wchar_t* dirname);
              int     _wchdir (const wchar_t* dirname);
            }
            #endif
            // === end Xenoide patch ===

            """)

        replace_in_file(
            self,
            io_win32,
            "#include <windows.h>",
            "#include <windows.h>\n" + forward_decls,
            strict=False,
        )

    def build(self):
        self._patch_sources()
        cmake = CMake(self)
        # Both 2.x and 3.x keep their CMake glue under <src>/cmake/.
        cmake.configure(build_script_folder="cmake")
        cmake.build()

    def package(self):
        # Headers live under <src>/src/google/...
        copy(
            self,
            pattern="*.h",
            src=os.path.join(self.source_folder, "src"),
            dst=os.path.join(self.package_folder, "include"),
        )
        copy(
            self,
            pattern="*.inc",
            src=os.path.join(self.source_folder, "src"),
            dst=os.path.join(self.package_folder, "include"),
        )

        # Static and import libraries.
        for pattern in ("*.lib", "*.a"):
            copy(
                self,
                pattern=pattern,
                src=self.build_folder,
                dst=os.path.join(self.package_folder, "lib"),
                keep_path=False,
            )

        # Shared libraries.
        for pattern in ("*.so*", "*.dylib"):
            copy(
                self,
                pattern=pattern,
                src=self.build_folder,
                dst=os.path.join(self.package_folder, "lib"),
                keep_path=False,
            )

        # Runtime + protoc on Windows.
        for pattern in ("*.dll", "protoc.exe", "protoc"):
            copy(
                self,
                pattern=pattern,
                src=self.build_folder,
                dst=os.path.join(self.package_folder, "bin"),
                keep_path=False,
            )

        # CMake build module that exposes protobuf_generate_cpp +
        # protobuf_PROTOC_EXECUTABLE to consumers. Sourced from the
        # recipe folder (where `exports = "cmake/*"` placed it) rather
        # than the source folder, which only contains protobuf's own
        # extracted tree.
        copy(
            self,
            pattern="protobuf-generate.cmake",
            src=os.path.join(self.recipe_folder, "cmake"),
            dst=os.path.join(self.package_folder, "lib", "cmake", "protobuf"),
            keep_path=False,
        )

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "protobuf")
        self.cpp_info.set_property("cmake_target_name", "protobuf::protobuf")

        build_module = os.path.join("lib", "cmake", "protobuf", "protobuf-generate.cmake")
        self.cpp_info.set_property("cmake_build_modules", [build_module])

        libprotobuf = self.cpp_info.components["libprotobuf"]
        libprotobuf.set_property("cmake_target_name", "protobuf::libprotobuf")
        libprotobuf.set_property("cmake_build_modules", [build_module])
        libprotobuf.libs = ["libprotobuf"] if self._is_msvc else ["protobuf"]
        libprotobuf.includedirs = ["include"]
        if self.settings.os in ("Linux", "FreeBSD"):
            libprotobuf.system_libs = ["pthread"]

        # Make protoc discoverable on PATH for any consumer that calls it
        # directly (and so VirtualBuildEnv picks it up).
        self.buildenv_info.append_path("PATH", os.path.join(self.package_folder, "bin"))
        self.runenv_info.append_path("PATH", os.path.join(self.package_folder, "bin"))
