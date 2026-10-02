import os
import textwrap

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy, get, replace_in_file


class ZeroMQConan(ConanFile):
    name = "zeromq"
    license = "LGPL-3.0"
    homepage = "https://zeromq.org/"
    url = "https://github.com/zeromq/libzmq"
    description = "ZeroMQ messaging library, curated for the Xenoide build host (incl. old MinGW)."
    topics = ("zeromq", "messaging", "networking", "ipc")

    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "encryption": [False, "tweetnacl"],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        # libsodium intentionally not offered: its autotools build fails
        # on the Windows-x86 / TDM-GCC profile (see root conanfile.py).
        "encryption": "tweetnacl",
    }

    @property
    def _is_mingw(self):
        return self.settings.os == "Windows" and self.settings.compiler == "gcc"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self, src_folder="src")

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.cache_variables["BUILD_SHARED"] = "ON" if self.options.shared else "OFF"
        tc.cache_variables["BUILD_STATIC"] = "OFF" if self.options.shared else "ON"
        tc.cache_variables["BUILD_TESTS"] = "OFF"
        tc.cache_variables["WITH_PERF_TOOL"] = "OFF"
        tc.cache_variables["WITH_DOC"] = "OFF"
        tc.cache_variables["ENABLE_CURVE"] = "ON"
        tc.cache_variables["WITH_TWEETNACL"] = (
            "ON" if self.options.encryption == "tweetnacl" else "OFF"
        )
        tc.cache_variables["WITH_LIBSODIUM"] = "OFF"
        tc.cache_variables["ENABLE_DRAFTS"] = "OFF"
        # Older zeromq CMakeLists hits policy floors removed in CMake 4.
        tc.cache_variables["CMAKE_POLICY_VERSION_MINIMUM"] = "3.5"

        # On MinGW, pre-seed the HAVE_* cache vars so zeromq's broken
        # check_cxx_symbol_exists probes (which pass `*.lib` filenames to
        # the MinGW linker) are skipped — see _patch_sources() for the
        # full rationale.
        if self._is_mingw:
            # Pre-seed the HAVE_* probes that test for *libraries* — the
            # libs themselves all ship with MinGW, but zeromq's CMake
            # passes MSVC-style `*.lib` filenames to the link probe and
            # the MinGW linker rejects them. We assert the libs are
            # present and let _patch_sources() rewrite the filenames.
            #
            # We deliberately do NOT pre-seed HAVE_IF_NAMETOINDEX. That
            # probe tests for a *symbol* (`if_nametoindex` declared in
            # `<iphlpapi.h>`), not just the library — and old MinGW.org
            # 5.x ships an iphlpapi.h that doesn't declare it. Letting
            # the natural probe run means it'll fail on old toolchains
            # (so zeromq's `#ifdef HAVE_IF_NAMETOINDEX` skips the call,
            # losing only IPv6 multicast bind-by-name) and pass on
            # MinGW-w64 where the symbol is declared.
            for v in (
                "HAVE_WS2_32",
                "HAVE_RPCRT4",
                "HAVE_IPHLAPI",
                "HAVE_WS2",
            ):
                tc.cache_variables[v] = "1"

        tc.generate()

    def _patch_sources(self):
        # === Patch: skip MSVC-flavoured library probes on MinGW ===========
        # Inside `if(ZMQ_HAVE_WINDOWS)` zeromq runs:
        #     set(CMAKE_REQUIRED_LIBRARIES "rpcrt4.lib")
        #     check_cxx_symbol_exists(UuidCreateSequential "rpc.h" HAVE_RPCRT4)
        # The `*.lib` filename is the MSVC link convention; the MinGW
        # linker treats it as a literal filename and doesn't find it
        # (MinGW ships `librpcrt4.a` and links via `-lrpcrt4`). The probe
        # then fails and zeromq fires a fatal `Cannot link to rpcrt4`.
        #
        # We pre-seed every HAVE_* probed in this block via cache_variables
        # in generate(), and additionally rewrite the `*.lib` filenames in
        # the source so any later `target_link_libraries(...)` based on
        # CMAKE_REQUIRED_LIBRARIES doesn't drag a non-existent file into
        # the link line. The libs themselves ship with every MinGW distro
        # (ws2_32, rpcrt4, iphlpapi) so linking succeeds normally.
        if not self._is_mingw:
            return

        cmakelists = os.path.join(self.source_folder, "CMakeLists.txt")
        for msvc_name, mingw_name in (
            ("ws2_32.lib", "ws2_32"),
            ("rpcrt4.lib", "rpcrt4"),
            ("iphlpapi.lib", "iphlpapi"),
        ):
            replace_in_file(
                self,
                cmakelists,
                f'"{msvc_name}"',
                f'"{mingw_name}"',
                strict=False,
            )

        # === Patch: gate MSVC SEH thread-naming on _MSC_VER ===============
        # zeromq's `applyThreadName()` raises an MSVC-specific exception
        # (`MS_VC_EXCEPTION = 0x406D1388`) wrapped in a hand-rolled SEH
        # registration record. MinGW.org / TDM-GCC's preprocessor expands
        # NTAPI to `__attribute__((stdcall))` in a way that rejects the
        # `typedef EXCEPTION_DISPOSITION (NTAPI *Handler)(...)` syntax,
        # and even if it compiled the SEH machinery would not work.
        # Upstream gates the entire block on `ZMQ_HAVE_WINDOWS`, which is
        # too coarse — it also catches MinGW. We tighten the gate to
        # `_MSC_VER` and provide an empty stub for the MinGW path
        # (debugger thread-naming is the only feature lost).
        thread_cpp = os.path.join(self.source_folder, "src", "thread.cpp")

        seh_open = "struct MY_EXCEPTION_REGISTRATION_RECORD"
        replace_in_file(
            self,
            thread_cpp,
            seh_open,
            "#ifdef _MSC_VER\n" + seh_open,
            strict=False,
        )

        # Anchor on the unique tail of `applyThreadName()` and append the
        # `#else` stub + `#endif`. Indentation must match the source file
        # verbatim (4-space body, column-0 closing brace) — earlier we
        # used textwrap.dedent and the common-prefix dedent stripped the
        # 4-space indent from the body lines, so the anchor never matched
        # and only the opening `#ifdef _MSC_VER` made it in. That left the
        # inner conditional unterminated; `#elif defined ZMQ_HAVE_VXWORKS`
        # then bound to it instead of the outer `#ifdef ZMQ_HAVE_WINDOWS`,
        # and on MinGW the preprocessor fell through to the POSIX `#else`
        # branch and tried to `#include <sys/resource.h>`.
        seh_close = (
            "    tib->ExceptionList =\n"
            "      (_EXCEPTION_REGISTRATION_RECORD\n"
            "         *) (((MY_EXCEPTION_REGISTRATION_RECORD *) tib->ExceptionList)->Next);\n"
            "}"
        )
        seh_close_replacement = (
            seh_close
            + "\n\n#else\n"
            + "// MinGW: MSVC SEH-based thread naming is unavailable; stub it out.\n"
            + "void zmq::thread_t::applyThreadName () {}\n"
            + "#endif"
        )
        replace_in_file(
            self,
            thread_cpp,
            seh_close,
            seh_close_replacement,
            strict=True,
        )

        # === Patch: ip_resolver.cpp <netioapi.h> on old MinGW =============
        # zeromq's `get_interface_name` includes <netioapi.h> directly.
        # That header ships with the modern Windows SDK and MinGW-w64,
        # but not legacy MinGW.org 5.x. The two things referenced from
        # it — `IF_MAX_STRING_SIZE` and `if_indextoname` — are also
        # reachable via <iphlpapi.h>, with the function exported by
        # iphlpapi.dll on every Windows target.
        #
        # Use `__has_include` (GCC 5+, which TDM-GCC 5.1 supports) to
        # fall back to <iphlpapi.h> + a minimal `IF_MAX_STRING_SIZE`
        # define + a forward declaration of `if_indextoname` when the
        # header is unavailable. No-op on MinGW-w64 / MSVC.
        ip_resolver_cpp = os.path.join(
            self.source_folder, "src", "ip_resolver.cpp",
        )
        netioapi_replacement = textwrap.dedent("""\
            #if defined(__has_include) && __has_include(<netioapi.h>)
            #include <netioapi.h>
            #else
            // Old MinGW.org doesn't ship <netioapi.h>; pull the symbols
            // we need from <iphlpapi.h> (linked against iphlpapi.dll).
            // It also predates <ws2def.h>'s ADDRESS_FAMILY typedef, which
            // ip_resolver.cpp references when walking GetAdaptersAddresses
            // results. The sockaddr sa_family field is USHORT on Windows,
            // so the upstream typedef is just `typedef USHORT ADDRESS_FAMILY;`.
            #include <windows.h>
            #include <iphlpapi.h>
            #ifndef IF_MAX_STRING_SIZE
            #define IF_MAX_STRING_SIZE 256
            #endif
            typedef USHORT ADDRESS_FAMILY;
            // if_indextoname was added in Vista. Old MinGW.org's <iphlpapi.h>
            // doesn't declare it AND its libiphlpapi.a doesn't export it, so
            // a plain forward declaration would link-fail. Resolve it from
            // iphlpapi.dll at runtime instead; on legacy hosts where the
            // export is missing, return NULL — the caller (zeromq's
            // get_interface_name) treats that as "couldn't resolve" and the
            // resolver falls back to the friendly-name path.
            static inline char *if_indextoname(unsigned long InterfaceIndex,
                                               char *InterfaceName)
            {
                typedef char *(WINAPI *if_indextoname_fn)(unsigned long, char *);
                static if_indextoname_fn fn = NULL;
                static int resolved = 0;
                if (!resolved) {
                    HMODULE h = LoadLibraryA("iphlpapi.dll");
                    if (h)
                        fn = (if_indextoname_fn) GetProcAddress(h, "if_indextoname");
                    resolved = 1;
                }
                return fn ? fn(InterfaceIndex, InterfaceName) : NULL;
            }
            #endif""")
        replace_in_file(
            self,
            ip_resolver_cpp,
            "#include <netioapi.h>",
            netioapi_replacement,
            strict=True,
        )

    def build(self):
        self._patch_sources()
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(
            self,
            pattern="LICENSE*",
            src=self.source_folder,
            dst=os.path.join(self.package_folder, "licenses"),
        )
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "ZeroMQ")
        self.cpp_info.set_property("cmake_target_name", "libzmq")

        if self.settings.os == "Windows":
            # zeromq names its static lib libzmq-mt-s-X_Y_Z.lib on MSVC
            # and libzmq.a on MinGW. We let it pick whatever it built and
            # expose system libs the upstream CMake hides behind probes.
            self.cpp_info.system_libs = ["ws2_32", "rpcrt4", "iphlpapi"]
            if not self.options.shared:
                self.cpp_info.defines = ["ZMQ_STATIC"]
        elif self.settings.os in ("Linux", "FreeBSD"):
            self.cpp_info.system_libs = ["pthread", "rt", "m"]

        # Library name discovery: zeromq's install drops a single .a/.lib
        # under lib/, plus a versioned variant on Windows. We list both
        # plain `zmq` and `libzmq` so consumers can link either way.
        self.cpp_info.libs = ["zmq"] if self._is_mingw else ["libzmq"]
