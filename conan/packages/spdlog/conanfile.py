import os
import textwrap

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy, get, replace_in_file


class SpdlogConan(ConanFile):
    name = "spdlog"
    license = "MIT"
    homepage = "https://github.com/gabime/spdlog"
    url = "https://github.com/gabime/spdlog"
    description = "Fast C++ logging library, curated for the Xenoide build host (incl. old MinGW)."
    topics = ("logging", "spdlog", "header-only", "logger")

    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "header_only": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "header_only": False,
    }

    @property
    def _is_msvc(self):
        return self.settings.compiler == "msvc"

    @property
    def _is_mingw(self):
        # Conan reports MinGW as compiler=gcc on os=Windows.
        return self.settings.os == "Windows" and self.settings.compiler == "gcc"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.header_only:
            self.options.rm_safe("shared")
            self.options.rm_safe("fPIC")
            return
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        # Mirror the protobuf recipe: keep extracted sources isolated under
        # `src/` so conan's `build/<cfg>` folders never collide with files
        # the spdlog tarball lays down at the recipe root.
        cmake_layout(self, src_folder="src")

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        # Use the bundled fmt to keep the dependency surface minimal — old
        # MinGW already gives us enough trouble.
        tc.cache_variables["SPDLOG_FMT_EXTERNAL"] = "OFF"
        tc.cache_variables["SPDLOG_FMT_EXTERNAL_HO"] = "OFF"
        tc.cache_variables["SPDLOG_BUILD_EXAMPLE"] = "OFF"
        tc.cache_variables["SPDLOG_BUILD_EXAMPLE_HO"] = "OFF"
        tc.cache_variables["SPDLOG_BUILD_TESTS"] = "OFF"
        tc.cache_variables["SPDLOG_BUILD_TESTS_HO"] = "OFF"
        tc.cache_variables["SPDLOG_BUILD_BENCH"] = "OFF"
        tc.cache_variables["SPDLOG_INSTALL"] = "ON"

        if self.options.header_only:
            tc.cache_variables["SPDLOG_BUILD_SHARED"] = "OFF"
        else:
            tc.cache_variables["SPDLOG_BUILD_SHARED"] = (
                "ON" if self.options.shared else "OFF"
            )

        # Older spdlog CMakeLists hit policy floors removed in CMake 4 —
        # mirror the protobuf recipe's pin so configure does not fail.
        tc.cache_variables["CMAKE_POLICY_VERSION_MINIMUM"] = "3.5"
        tc.generate()

    def _patch_sources(self):
        # spdlog uses two preprocessor indentation styles across the
        # versions we ship: 1.12 indents directives with four spaces
        # (`#    include`), 1.17 keeps them flush-left (`#include`). We
        # apply each anchor in both forms so the patch works on either.
        def _replace_both_styles(path, anchor_tail, replacement_tail):
            replace_in_file(
                self, path,
                "#include " + anchor_tail,
                "#include " + replacement_tail,
                strict=False,
            )
            replace_in_file(
                self, path,
                "#    include " + anchor_tail,
                "#    include " + replacement_tail,
                strict=False,
            )

        # === Patch 1: drop the bare <fileapi.h> include on MinGW =========
        # spdlog includes `<fileapi.h>` directly in both stdout_sinks-inl.h
        # (1.12+) and os-inl.h (1.12 only). That header ships with the
        # modern Windows SDK and MinGW-w64, but the legacy MinGW.org 5.x
        # runtime targeting old MSVCRT only exposes `FlushFileBuffers`,
        # `WriteFile`, etc. via the umbrella `<windows.h>`. Rewriting the
        # include to `<windows.h>` keeps the same declarations available on
        # every toolchain (and is a harmless re-include where windows.h was
        # already pulled in via spdlog's own `windows_include.h` wrapper).
        for relpath in (
            ("include", "spdlog", "sinks", "stdout_sinks-inl.h"),
            ("include", "spdlog", "details", "os-inl.h"),
        ):
            target = os.path.join(self.source_folder, *relpath)
            if os.path.exists(target):
                _replace_both_styles(target, "<fileapi.h>", "<windows.h>")

        # === Patch 2: MSVC-only CRT entry points on old MinGW ============
        # spdlog calls MSVC secure-CRT and non-standard CRT helpers
        # (`localtime_s`, `gmtime_s`, `_fsopen`, `_fileno`, `_mkgmtime`)
        # unconditionally on `_WIN32`, and bundled fmt does the same in
        # `print()`. MinGW-w64 declares all of these globally; the older
        # MinGW.org 5.x runtime targeting the legacy MSVCRT does not, and
        # also doesn't typedef `errno_t`. We:
        #   1. Append the shim to <spdlog/details/windows_include.h>,
        #      which is included by both os-inl.h and stdout_sinks-inl.h
        #      (the two spdlog TUs that use these symbols), so a single
        #      patch covers both.
        #   2. Inject the same shim into bundled fmt's format-inl.h,
        #      which is independent of spdlog's headers.
        # The shim is gated on `__MINGW32__` / `__MINGW64__` so it is a
        # no-op on MSVC. On modern MinGW-w64 the redeclarations match the
        # CRT's own (compatible signatures), so the build is unaffected.
        mingw_shim = textwrap.dedent("""\

            // === Xenoide local patch for older MinGW toolchains ===
            // Two flavours of fix coexist here:
            //   * `_fileno`, `_fsopen` are exported by msvcrt.dll on every
            //     MinGW variant — a forward declaration is enough.
            //   * `localtime_s`, `gmtime_s`, `_mkgmtime` are secure-CRT
            //     additions only present in msvcr80.dll+ (i.e. NOT in the
            //     legacy msvcrt.dll that TDM-GCC 5.x / 32-bit targets).
            //     Forward-declaring them compiles but fails to link, so
            //     we ship `static inline` definitions backed by POSIX
            //     `localtime` / `gmtime` / `mktime`. Each TU gets its own
            //     internal-linkage copy — no ODR concerns.
            #if defined(_WIN32) && (defined(__MINGW32__) || defined(__MINGW64__))
            #include <stdio.h>
            #include <time.h>
            #include <io.h>
            #include <share.h>
            #ifndef _ERRNO_T_DEFINED
            typedef int errno_t;
            #define _ERRNO_T_DEFINED
            #endif

            #ifdef __cplusplus
            extern "C" {
            #endif
              FILE* _fsopen(const char* filename, const char* mode, int shflag);
              int   _fileno(FILE* stream);
            #ifdef __cplusplus
            }
            #endif

            static inline errno_t localtime_s(struct tm* _tm, const time_t* _time) {
                struct tm* _r = localtime(_time);
                if (!_r) return 1;
                *_tm = *_r;
                return 0;
            }
            static inline errno_t gmtime_s(struct tm* _tm, const time_t* _time) {
                struct tm* _r = gmtime(_time);
                if (!_r) return 1;
                *_tm = *_r;
                return 0;
            }
            static inline time_t _mkgmtime(struct tm* _tm) {
                /* Treat _tm as UTC. Round-trip via mktime to compute the
                 * local-vs-UTC delta and apply it. */
                time_t _local = mktime(_tm);
                if (_local == (time_t)-1) return (time_t)-1;
                struct tm _gm = *gmtime(&_local);
                _gm.tm_isdst = 0;
                return _local + (_local - mktime(&_gm));
            }
            #endif
            // === end Xenoide patch ===
            """)

        # Patch the spdlog wrapper that pulls in <windows.h>.
        windows_include = os.path.join(
            self.source_folder,
            "include", "spdlog", "details", "windows_include.h",
        )
        replace_in_file(
            self,
            windows_include,
            "#include <windows.h>",
            "#include <windows.h>\n" + mingw_shim,
            strict=False,
        )

        # Patch bundled fmt — independent header, doesn't pull
        # windows_include.h. The offending `_fileno(f)` call sits inside
        # fmt's `print()` function body, so a file-scope `extern "C"` shim
        # cannot go *at* the call site. We inject a one-line forward
        # declaration immediately before the function (file scope, legal
        # for extern "C"). Linkage is satisfied by MSVCRT.DLL, which
        # exports `_fileno` on every Windows toolchain we target.
        # Anchored on the function signature, which is unique in the file.
        format_inl = os.path.join(
            self.source_folder,
            "include", "spdlog", "fmt", "bundled", "format-inl.h",
        )
        if os.path.exists(format_inl):
            anchor = "FMT_FUNC void print(std::FILE* f, string_view text) {"
            fmt_shim = textwrap.dedent("""\
                // === Xenoide local patch for older MinGW toolchains ===
                #if defined(_WIN32) && (defined(__MINGW32__) || defined(__MINGW64__))
                extern "C" int _fileno(FILE*);
                #endif
                // === end Xenoide patch ===
                """)
            replace_in_file(
                self,
                format_inl,
                anchor,
                fmt_shim + anchor,
                strict=False,
            )

    def build(self):
        self._patch_sources()
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(
            self,
            pattern="LICENSE",
            src=self.source_folder,
            dst=os.path.join(self.package_folder, "licenses"),
        )
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "spdlog")

        target = "spdlog_header_only" if self.options.header_only else "spdlog"
        self.cpp_info.set_property("cmake_target_name", f"spdlog::{target}")

        if self.options.header_only:
            self.cpp_info.libs = []
            self.cpp_info.includedirs = ["include"]
            self.cpp_info.bindirs = []
            self.cpp_info.libdirs = []
        else:
            suffix = "d" if self.settings.build_type == "Debug" else ""
            self.cpp_info.libs = [f"spdlog{suffix}"]
            self.cpp_info.defines = ["SPDLOG_COMPILED_LIB"]

        # spdlog uses fmt internally; with the bundled copy and a compiled
        # library, consumers see the public `spdlog/fmt/...` headers but
        # do not need to link fmt separately.
        if self.settings.os in ("Linux", "FreeBSD"):
            self.cpp_info.system_libs = ["pthread"]
