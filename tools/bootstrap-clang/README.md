# bootstrap-clang

Experimental facility for **bootstrapping a 32-bit clang toolchain on Windows
from TDM-GCC-5**, aimed at the **Win9x / Win98 era** (PE subsystem version
4.x).

This is *not* a path to a modern toolchain. It exists to exercise clang's
ability to produce binaries for genuinely legacy 32-bit Windows targets.

- Target triple: `i686-w64-mingw32`
- PE subsystem floor: `4.0` (Win95/98/Me)
- LLVM: **7.0.1** (last 7.x release; the last branch where the mingw-w64
  driver still emits classic-MSVCRT-friendly output and which compiles
  cleanly with a C++11/14 host like GCC 5).

x86_64 is not in scope for this bootstrap.

## Pipeline

```
TDM-GCC-5 (32-bit)
    └─ stage 1 ─▶ clang/lld 7.0.1, host = i686-w64-mingw32
                     └─ stage 2 ─▶ clang/lld 7.0.1, host = i686-w64-mingw32
                                       └─ smoke test for the Win9x i686 target
                                       └─ emits a Conan profile for the project
```

Stage 2 existing and running == self-host succeeded. Single pass (no ternary
stage-3 bit comparison).

## Prerequisites

Required on `PATH` in a stock `cmd.exe` window:

- **TDM-GCC-5** (32-bit) — auto-detected at `C:\TDM-GCC-32`, override with
  `set TDM_GCC_ROOT=<path>` before calling any script.
- `git`
- `cmake` (≥ 3.13.4 — what LLVM 7 requires)
- `ninja`

Optional:

- `objdump` (ships with TDM-GCC-5) — used by the smoke test for a PE-version
  probe; if absent the smoke test still succeeds, just skips the probe.

No Git Bash, no MSYS, no PowerShell, no downloads.

## Usage

```bat
cd tools\bootstrap-clang
call env\stage0-tdm.cmd
call scripts\00-fetch-sources.cmd
call scripts\10-build-mingw-sysroot.cmd
call scripts\20-build-stage1.cmd
prefix\stage1\bin\clang.exe --version
call scripts\30-build-stage2.cmd
prefix\stage2\bin\clang.exe --version
call scripts\40-smoke-test.cmd
call scripts\99-emit-conan-profile.cmd
```

Stage 1 is the slow step — TDM-GCC-5 ships a 32-bit `ld.bfd`, so links are
serialized (`LLVM_PARALLEL_LINK_JOBS=1`). Expect 30-60 minutes on modern
hardware. Stage 2 uses lld and parallelizes properly.

## Layout

```
tools/bootstrap-clang/
├── README.md             this file
├── versions.json         pinned upstream LLVM tag
├── .gitattributes        CRLF for the .cmd subtree
├── env/
│   └── stage0-tdm.cmd    TDM-GCC-5 detection + PATH/CC/CXX export
├── scripts/
│   ├── 00-fetch-sources.cmd
│   ├── 10-build-mingw-sysroot.cmd    stages i686 sysroot from TDM-GCC-5
│   ├── 20-build-stage1.cmd
│   ├── 30-build-stage2.cmd
│   ├── 40-smoke-test.cmd             Win9x-floor i686 hello world
│   └── 99-emit-conan-profile.cmd
├── src/                  fetched LLVM tree     (gitignored)
└── prefix/               install roots         (gitignored)
    ├── mingw-sysroot/
    │   └── i686-w64-mingw32/
    ├── stage1/
    └── stage2/           the deliverable
```

## Sysroot strategy

The i686 sysroot is staged by mirroring TDM-GCC-5's bundled mingw-w64 runtime
(`<tdm>/include`, `<tdm>/lib`, `<tdm>/i686-w64-mingw32/`, `<tdm>/lib/gcc/`)
into `prefix/mingw-sysroot/i686-w64-mingw32/`. No download, no MSYS, no
network. Run once, idempotent.

## Conan profile output

`99-emit-conan-profile.cmd` writes:

- `conan/profiles/windows-x86-clang-legacy`

The profile bakes the Win9x PE floor into `LDFLAGS` (`--subsystem,console:4.0`
plus matching OS/subsystem version stamps) so anything Conan builds with this
profile inherits Win98-era binary compatibility.

```bat
conan install . --build=missing -pr:h windows-x86-clang-legacy ^
                                  -pr:b windows-x86-clang-legacy
```

Whether the rest of Xenoide actually compiles under LLVM 7's clang is **out
of scope** for this experiment — the toolchain enables that investigation,
it does not promise success.

## Caveats

- **Win9x runtime reality**: clang + lld can stamp a PE with subsystem 4.0,
  but actual Win98 compatibility also depends on the *imported APIs*. mingw-w64
  links against `msvcrt.dll` and `kernel32.dll` symbols — not all of which
  exist on Win9x. Producing a binary that *runs* on Win98 may need source-side
  discipline (avoid Unicode-W APIs, etc.) or a runtime shim like KernelEx.
  This bootstrap gives you the toolchain; the runtime audit is a separate
  exercise.
- **No CI**: this is a manual one-shot developer procedure.
- **Patches**: if LLVM 7 source files trip GCC-5 in your environment, drop
  unified-diff `.patch` files into `tools/bootstrap-clang/patches/` and the
  fetch script will apply them in lexical order. The directory is not created
  up-front.
