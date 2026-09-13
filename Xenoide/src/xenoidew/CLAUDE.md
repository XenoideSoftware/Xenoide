# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Generate Visual Studio solution (run from repo root)
cmake -B build

# Build the project
cmake --build build

# Build a specific configuration
cmake --build build --config Debug
cmake --build build --config Release
```

## Build Commands (MinGW — TDM-GCC-32)

The MinGW toolchain used is **TDM-GCC-32 5.1.0** located at `C:\TDM-GCC-32-5.1.0-3`.
Before running CMake with MinGW, the toolchain must be on PATH:

```powershell
# In PowerShell, prepend TDM-GCC-32 to PATH first
$env:PATH = "C:\TDM-GCC-32-5.1.0-3\bin;" + $env:PATH

# Generate MinGW Makefiles (run from repo root)
cmake -B build-gcc -G "MinGW Makefiles"

# Build the project
cmake --build build-gcc
```

## Win98 Compatibility Build

A dedicated build directory `build-win98` is configured to test Win98 API compatibility
(`WINVER=0x0410`, `_WIN32_WINNT=0x0410`):

```powershell
$env:PATH = "C:\TDM-GCC-32-5.1.0-3\bin;" + $env:PATH
cmake -B build-win98 -G "MinGW Makefiles" -DCMAKE_CXX_FLAGS="-DWINVER=0x0410 -D_WIN32_WINNT=0x0410 -D_WIN32_WINDOWS=0x0410" .
cmake --build build-win98
```

Use `cmake --build build-win98` to verify Win98 compatibility after any winlamb changes.

Unicode support is off by default. To enable:
```bash
cmake -B build -DAPP_UNICODE=ON
```

## Architecture

This is a native Windows GUI application in C++17 used to test a miriad of external Win32 libraries, backed by several git submodules.

### Key libraries (in `submodules/`)

- **winlamb** — Header-only C++ wrapper over Win32 API. Windows inherit from `wl::window_main`, `wl::dialog_main`, etc. Message handlers are registered via lambda callbacks in the constructor. Use `RUN(ClassName)` macro to generate the `WinMain` entry point.
- **mctrl** — Compiled as a shared library (`mCtrl.dll`). Provides modern Win32 controls beyond standard USER32/COMCTL32. The DLL is automatically copied next to the executable at post-build.
- **HexCtrl** — Hex editor/viewer control using C++20 modules (`.ixx` files). Conditionally compiled only when CMake >= 3.28.
- **win32-property-grid** — Static library, compiled as C despite `.cpp` extension.

### WinLamb window pattern

```cpp
class My_Window : public wl::window_main {
public:
    My_Window() {
        on_message(WM_CREATE, [this](wl::params p) -> LRESULT {
            // setup controls here
            return 0;
        });
        on_message(WM_COMMAND, [this](wl::params p) -> LRESULT {
            // handle commands
            return 0;
        });
    }
};
RUN(My_Window)
```

### No test infrastructure in main app

Tests exist inside mCtrl submodule but are disabled (`MCTRL_BUILD_TESTS OFF`). There is no test runner for the main application.

## Win98 Compatibility Conventions

The winlamb headers have been audited and guarded for Win98 compatibility. When adding
new Win32 API calls, follow these patterns:

### GetWindowLongPtr / SetWindowLongPtr

These functions only exist on Win2000+ (`_WIN32_WINNT >= 0x0500`). Use:
```cpp
#if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0500
    GetWindowLongPtrW(hWnd, GWLP_USERDATA);   // GWLP_* constants
    SetWindowLongPtrW(hWnd, GWLP_HINSTANCE, ...);
#else
    GetWindowLongW(hWnd, GWL_USERDATA);        // GWL_* fallback
    SetWindowLongW(hWnd, GWL_HINSTANCE, ...);
#endif
// DWLP_USER (dialogs) falls back to DWL_USER
// GWLP_ID falls back to GWL_ID
```

### GetClassLongPtr

Same threshold. Use `GetClassLongW` + `GCL_*` as fallback for `GCLP_*`.

### VersionHelpers.h / IsWindowsVistaOrGreater()

`<VersionHelpers.h>` is not available on Win9x SDKs. Use the runtime helper in
`winlamb/internals/os_version.h` instead:
```cpp
#include "internals/os_version.h"
if (_wli::IsWindowsVistaOrGreater()) { ... }
```

### ITaskbarList3 (progress_taskbar.h)

Requires Win7 (`_WIN32_WINNT >= 0x0601`). The entire class is guarded:
```cpp
#if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0601
// class progress_taskbar { ... };
#endif
```

### CSIDL_PROGRAM_FILESX86

Not defined on Win9x. Guard usage with:
```cpp
#if defined(CSIDL_PROGRAM_FILESX86)
// ...
#endif
```

### XP-only features (already guarded in winlamb)

- `SetWindowSubclass` / `RemoveWindowSubclass` / `DefSubclassProc` → `_WIN32_WINNT >= 0x0501`
- UxTheme (`IsThemeActive`, `OpenThemeData`, etc.) → `_WIN32_WINNT >= 0x0501`
