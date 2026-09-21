# CMake Target Grouping: Execution Plan for Xenoide

## Goal
Organize Visual Studio Solution Explorer into four top-level folders using the `FOLDER` target property and `CMAKE_FOLDER` inheritance, with dedicated sub-folders for automated tests and POCs/playgrounds:

| Folder | Contents |
|---|---|
| `CMakePredefinedTargets` | `ALL_BUILD`, `ZERO_CHECK`, `INSTALL`, `RUN_TESTS`, plus CTest dashboard targets (`Continuous`, `Experimental`, `Nightly`, `NightlyMemoryCheck`) |
| `Common` | Shared base interface and utility libraries (`xe-interface`) |
| `Engine` | Core engine production libraries (`xe-core`, `xe-math`, `xe-geometry`, `libxe-graphics`, `libxe-graphics-gl`, `libxe-graphics-gl3`, `xe-imageloader`, `xe-mesh`, `xe-scene`) |
| `Engine/Tests` | Automated engine unit test suites (`xe-core-test`, `xe-geometry-test`, `xe-math-test`, `libxe-graphics-test`) |
| `Engine/Pocs` | Engine proof-of-concept projects, library experiments, and playgrounds (`xe-glaze-test`, `xe-render-core`, `xe-render-core-3d`, `d3d11-app-poc`, `vulkan-app-poc`, `xe-gltf-view`) |
| `IDE` | IDE production libraries and application (`xenoidew`, `winlambxe`, `xenoide-core`, `xenoide-base`, `xenoide-sanitize`) |
| `IDE/Tests` | Optional IDE unit test suite (`xenoide-test`) |
| `IDE/Pocs` | IDE proof-of-concept projects and tooling experiments (`clang-test`, `poc-lsp-*`, `poc-zeromq-*`, `poc-protobuf-*`, `poc-wxminimal`) |

---

## Phase 1: Inventory

### Active Targets in `build/xenoide.sln` (27 targets)

| Target | Type | Group | Source Directory | Notes |
|---|---|---|---|---|
| `ALL_BUILD` | Utility | `CMakePredefinedTargets` | N/A | CMake built-in |
| `ZERO_CHECK` | Utility | `CMakePredefinedTargets` | N/A | CMake built-in |
| `INSTALL` | Utility | `CMakePredefinedTargets` | N/A | CMake built-in |
| `RUN_TESTS` | Utility | `CMakePredefinedTargets` | N/A | CMake built-in |
| `Continuous` | Custom | `CMakePredefinedTargets` | Root (`CTest`) | CTest dashboard target |
| `Experimental` | Custom | `CMakePredefinedTargets` | Root (`CTest`) | CTest dashboard target |
| `Nightly` | Custom | `CMakePredefinedTargets` | Root (`CTest`) | CTest dashboard target |
| `NightlyMemoryCheck` | Custom | `CMakePredefinedTargets` | Root (`CTest`) | CTest dashboard target |
| `xe-interface` | INTERFACE | `Common` | `src/base-target` | Interface library for compiler flags and specifications |
| `xe-core` | STATIC | `Engine` | `src/engine/src/libxe-core` | Core engine utilities |
| `xe-math` | STATIC | `Engine` | `src/engine/src/libxe-math` | Engine math library |
| `xe-geometry` | STATIC | `Engine` | `src/engine/src/libxe-geometry` | Procedural geometry generation |
| `libxe-graphics` | STATIC | `Engine` | `src/engine/src/libxe-graphics` | Graphics device abstractions |
| `libxe-graphics-gl` | STATIC | `Engine` | `src/engine/src/libxe-graphics-gl` | OpenGL renderer implementation |
| `libxe-graphics-gl3` | STATIC | `Engine` | `src/engine/src/libxe-graphics-gl3` | GL3 renderer backend |
| `xe-imageloader` | STATIC | `Engine` | `src/engine/src/libxe-imageloader` | Image loading library |
| `xe-mesh` | STATIC | `Engine` | `src/engine/src/libxe-mesh` | Mesh data structures |
| `xe-scene` | STATIC | `Engine` | `src/engine/src/libxe-scene` | Scene graph and node hierarchy |
| `xe-core-test` | EXECUTABLE | `Engine/Tests` | `src/engine/src/libxe-core-test` | Automated Catch2 unit test |
| `xe-geometry-test` | EXECUTABLE | `Engine/Tests` | `src/engine/src/libxe-geometry-test` | Automated Catch2 unit test |
| `xe-math-test` | EXECUTABLE | `Engine/Tests` | `src/engine/src/libxe-math/test` | Automated Catch2 unit test |
| `libxe-graphics-test` | EXECUTABLE | `Engine/Tests` | `src/engine/src/libxe-graphics-gl/unit-test` | Automated Catch2 unit test |
| `xe-glaze-test` | EXECUTABLE | `Engine/Pocs` | `src/engine/src/xe-glaze-test` | Glaze & OpenGL rendering experiment |
| `xe-render-core` | EXECUTABLE | `Engine/Pocs` | `src/engine/src/xe-glaze-test` | Renderer core playground executable |
| `xe-render-core-3d` | EXECUTABLE | `Engine/Pocs` | `src/engine/src/xe-render-core-3d-test` | 3D rendering experiment executable |
| `xenoide-core` | STATIC | `IDE` | `src/ide/src/xenoide-core` | Core IDE services (FileService, StringUtil) |
| `winlambxe` | STATIC | `IDE` | `src/ide/src/winlambxe` | WinLamb Win32 UI framework library |
| `xenoidew` | EXECUTABLE | `IDE` | `src/ide/src/xenoidew` | Main Windows IDE executable (`VS_STARTUP_PROJECT`) |

### Inactive / Conditionally Enabled Targets (including POCs)

| Target | Type | Group | Source Directory | Notes |
|---|---|---|---|---|
| `xe-app` | STATIC | `Engine` | `src/engine/src/libxe-app` | Application scaffold (currently commented) |
| `xe-graphics-png` | STATIC | `Engine` | `src/engine/src/libxe-core-imageloader-lodepng` | LodePNG plugin (`XE_ENABLE_ENGINE_PLUGIN_PNG`) |
| `xe-platform-glfw` | STATIC | `Engine` | `src/engine/src/libxe-core-platform-glfw` | GLFW platform plugin (`XE_ENABLE_ENGINE_PLUGIN_GL_GLFW`) |
| `xe-ktxc` | EXECUTABLE | `Engine` | `src/engine/src/xe-ktxc` | KTX tool (`XE_ENABLE_ENGINE_PLUGIN_KTX`) |
| `xe-gltf-view` | EXECUTABLE | `Engine/Pocs` | `src/engine/src/xe-gltf-view` | GLTF viewer experiment / playground |
| `xe-gltfc` | EXECUTABLE | `Engine` | `src/engine/src/xe-gltfc` | GLTF compiler tool |
| `xe-d3d11-app-poc` | EXECUTABLE | `Engine/Pocs` | `src/engine/src/d3d11-app-poc` | Direct3D 11 proof-of-concept |
| `xe-vulkan-app-poc` | EXECUTABLE | `Engine/Pocs` | `src/engine/src/vulkan-app-poc` | Vulkan proof-of-concept |
| `xenoide-base` | INTERFACE | `IDE` | `src/ide/src/xenoide-base` | Compiler warnings interface |
| `xenoide-sanitize` | INTERFACE | `IDE` | `src/ide/src/xenoide-sanitize` | Sanitizer flags interface |
| `xenoide-win32xx` | EXECUTABLE | `IDE` | `src/ide/src/xenoide-win32xx` | Alternate Win32++ UI frontend |
| `xenoide-qt6` | EXECUTABLE | `IDE` | `src/ide/src/xenoide-qt6` | Alternate Qt6 UI frontend |
| `xenoide-wx3` | EXECUTABLE | `IDE` | `src/ide/src/xenoide-wx3` | Alternate wxWidgets UI frontend |
| `poc-wxminimal` | EXECUTABLE | `IDE/Pocs` | `src/ide/src/poc-wxminimal` | wxWidgets minimal POC (`XENOIDE_UI_WX`) |
| `xenoide-test` | EXECUTABLE | `IDE/Tests` | `src/ide/src/xenoide-test` | Catch2 IDE test suite (`XENOIDE_TESTS`) |
| `clang-test` | EXECUTABLE | `IDE/Pocs` | `src/ide/src/clang-test` | Clang libtooling experiment (`XENOIDE_POCS`) |
| `poc-lsp-client` | EXECUTABLE | `IDE/Pocs` | `src/ide/src/poc-lsp-client` | Language Server Protocol client POC (`XENOIDE_POCS`) |
| `poc-lsp-server` | EXECUTABLE | `IDE/Pocs` | `src/ide/src/poc-lsp-server` | Language Server Protocol server POC (`XENOIDE_POCS`) |
| `poc-zeromq-client` | EXECUTABLE | `IDE/Pocs` | `src/ide/src/poc-zeromq-client` | ZeroMQ messaging client POC (`XENOIDE_POCS`) |
| `poc-zeromq-server` | EXECUTABLE | `IDE/Pocs` | `src/ide/src/poc-zeromq-server` | ZeroMQ messaging server POC (`XENOIDE_POCS`) |
| `protobuf-protocol` | STATIC | `IDE/Pocs` | `src/ide/src/poc-protobuf-protocol` | Protobuf protocol library POC (`XENOIDE_POCS`) |
| `person-proto` | STATIC | `IDE/Pocs` | `src/ide/src/poc-protobuf-test` | Protobuf test schema library (`XENOIDE_POCS`) |
| `protobuf-test` | EXECUTABLE | `IDE/Pocs` | `src/ide/src/poc-protobuf-test` | Protobuf serialization POC (`XENOIDE_POCS`) |
| `xenoide-poc` | EXECUTABLE | `IDE/Pocs` | `src/ide/src-deprecated/xenoide-poc` | Deprecated UI POC |

---

## Phase 2: Root Configuration

In top-level `CMakeLists.txt`:
1. Enable `USE_FOLDERS` and set `PREDEFINED_TARGETS_FOLDER`.
2. Wrap `include(CTest)` with `set(CMAKE_FOLDER "CMakePredefinedTargets")` to capture `Continuous`, `Experimental`, `Nightly`, and `NightlyMemoryCheck`.
3. Include the helper module `cmake/TargetFolders.cmake`.
4. Configure `xenoidew` as the default Visual Studio startup project.

```cmake
# Top-level CMakeLists.txt (after project(...))
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakePredefinedTargets")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(TargetFolders)
```

```cmake
# Top-level CMakeLists.txt (wrapping CTest)
if (XE_ENABLE_TESTING)
    set(CMAKE_FOLDER "CMakePredefinedTargets")
    include(CTest)
    enable_testing()
    find_package(Catch2 3 CONFIG REQUIRED)
    unset(CMAKE_FOLDER)
endif()
```

```cmake
# Top-level CMakeLists.txt (at end of file)
if (TARGET xenoidew)
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT xenoidew)
endif()
```

---

## Phase 3: Helper Function

Put in `cmake/TargetFolders.cmake` and include from root:

```cmake
# cmake/TargetFolders.cmake
function(set_target_folder folder)
    foreach(target IN LISTS ARGN)
        if(TARGET ${target})
            set_target_properties(${target} PROPERTIES FOLDER "${folder}")
        else()
            message(WARNING "set_target_folder: '${target}' is not a target")
        endif()
    endforeach()
endfunction()
```

---

## Phase 4: Apply Grouping (Strategy A + B)

We adopt **Strategy A + B**:
- **Strategy A (directory defaults)**: Set `CMAKE_FOLDER` before entering `src/base-target`, `src/engine`, and `src/ide`. Production libraries and executables inherit their groups automatically.
- **Strategy B (scoped overrides)**: Scoping `CMAKE_FOLDER` around test subdirectories (`Engine/Tests`, `IDE/Tests`) and proof-of-concept subdirectories (`Engine/Pocs`, `IDE/Pocs`).

### 1. `src/CMakeLists.txt`
```cmake
set(CMAKE_FOLDER "Common")
add_subdirectory("base-target")

if (XE_ENABLE_ENGINE)
    set(CMAKE_FOLDER "Engine")
    add_subdirectory("engine")
endif()

if (XE_ENABLE_IDE)
    set(CMAKE_FOLDER "IDE")
    add_subdirectory("ide")
endif()

unset(CMAKE_FOLDER)
```

### 2. `src/engine/src/CMakeLists.txt`
```cmake
# When adding PoC targets:
# set(CMAKE_FOLDER "Engine/Pocs")
# add_subdirectory("vulkan-app-poc")
# add_subdirectory("d3d11-app-poc")
# set(CMAKE_FOLDER "Engine")

add_subdirectory ("libxe-core")
if (XE_ENABLE_TESTING)
    set(CMAKE_FOLDER "Engine/Tests")
    add_subdirectory ("libxe-core-test")
    set(CMAKE_FOLDER "Engine")
endif()

add_subdirectory ("libxe-math")
add_subdirectory ("libxe-scene")
add_subdirectory ("libxe-geometry")
if (XE_ENABLE_TESTING)
    set(CMAKE_FOLDER "Engine/Tests")
    add_subdirectory ("libxe-geometry-test")
    set(CMAKE_FOLDER "Engine")
endif()

add_subdirectory ("libxe-imageloader")
add_subdirectory ("libxe-graphics")
add_subdirectory ("libxe-graphics-gl")
add_subdirectory ("libxe-graphics-gl3")
add_subdirectory ("libxe-mesh")

# Glaze & 3D renderer experiments / playgrounds
set(CMAKE_FOLDER "Engine/Pocs")
add_subdirectory("xe-glaze-test")
add_subdirectory("xe-render-core-3d-test")
set(CMAKE_FOLDER "Engine")
```

### 3. `src/engine/src/libxe-math/CMakeLists.txt`
```cmake
if (XE_ENABLE_TESTING)
    set(CMAKE_FOLDER "Engine/Tests")
    add_subdirectory("test")
    set(CMAKE_FOLDER "Engine")
endif()
```

### 4. `src/engine/src/libxe-graphics-gl/CMakeLists.txt`
```cmake
if (XE_ENABLE_TESTING)
    set(CMAKE_FOLDER "Engine/Tests")
    add_subdirectory("unit-test")
    set(CMAKE_FOLDER "Engine")
endif()
```

### 5. `src/ide/src/CMakeLists.txt`
```cmake
# Targets in xenoide-base, xenoide-sanitize, xenoide-core, winlambxe, xenoidew
# automatically inherit CMAKE_FOLDER = "IDE".

if (XENOIDE_UI_WX)
    set(CMAKE_FOLDER "IDE/Pocs")
    add_subdirectory("poc-wxminimal")
    set(CMAKE_FOLDER "IDE")
    add_subdirectory("xenoide-wx3")
endif()

if (XENOIDE_TESTS)
    set(CMAKE_FOLDER "IDE/Tests")
    add_subdirectory("xenoide-test")
    set(CMAKE_FOLDER "IDE")
endif()

if (XENOIDE_POCS)
    set(CMAKE_FOLDER "IDE/Pocs")
    add_subdirectory("clang-test")
    add_subdirectory("poc-zeromq-server")
    add_subdirectory("poc-zeromq-client")
    add_subdirectory("poc-protobuf-test")
    add_subdirectory("poc-protobuf-protocol")
    add_subdirectory("poc-lsp-server")
    add_subdirectory("poc-lsp-client")
    set(CMAKE_FOLDER "IDE")
endif ()
```

---

## Phase 5: Third-Party and Dependencies

- **Conan Packages**: Dependencies (`Catch2`, `winlamb`, `scintilla3`, `fmt`, `glfw`, `glaze`, `gsl-lite`, etc.) are imported as `find_package(...)` `IMPORTED` targets. They do not generate `.vcxproj` project files in the Visual Studio solution and do not require folder grouping.
- **Future In-Tree / FetchContent Dependencies**: If dependencies are built from source via `add_subdirectory` or `FetchContent`, set `CMAKE_FOLDER "ThirdParty"` around them to keep them in an isolated top-level folder.

---

## Phase 6: Optional Extras & IDE Configuration

- [x] **Sub-folders for tests**: Engine unit tests organized under `Engine/Tests`, IDE unit tests under `IDE/Tests`.
- [x] **Sub-folders for POCs & experiments**: Dedicated `Engine/Pocs` and `IDE/Pocs` sub-folders separating experimental/playground code from production libraries.
- [x] **IDE Startup Project**: Configured `xenoidew` as `VS_STARTUP_PROJECT` on the root directory.
- [ ] **File filters (`source_group`)**: File tree structure within individual `.vcxproj` projects can be mirrored using `source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})` as a future refinement.

---

## Phase 7: Verification

1. **Configure via Conan Preset**:
   ```powershell
   powershell -ExecutionPolicy Bypass -File mise/configure.ps1 -Configuration Debug
   # or
   cmake --preset conan-default
   ```
2. **Confirm Solution Hierarchy in `build/xenoide.sln`**:
   - Inspect `build/xenoide.sln` for `GlobalSection(NestedProjects) = preSolution`.
   - Confirm all active targets are mapped to `CMakePredefinedTargets`, `Common`, `Engine`, `Engine/Tests`, `Engine/Pocs`, or `IDE`.
   - Confirm no projects remain at the root level.
3. **Confirm Visual Studio Behavior**:
   - Open `build/xenoide.sln` in Visual Studio 2022.
   - Verify top-level folders: `CMakePredefinedTargets`, `Common`, `Engine`, and `IDE`.
   - Verify subfolders under Engine (`Tests`, `Pocs`) and IDE (`Tests`, `Pocs` when enabled).
   - Verify `xenoidew` is bolded as the default startup project.
   - Press F5 or build to verify no build regressions.
4. **Run Test Suite**:
   ```powershell
   powershell -ExecutionPolicy Bypass -File mise/test.ps1 -Configuration Debug
   ```

---

## Open Questions & Resolutions

1. **Where do tests, samples, and POC targets live?**
   - **Resolved**: 
     - Automated unit tests live under `Engine/Tests` and `IDE/Tests`.
     - Proof-of-concept projects, library experiments, and playgrounds live under `Engine/Pocs` and `IDE/Pocs`.
2. **Should Common be split (e.g. `Common/Core`, `Common/Platform`)?**
   - **Resolved**: No. Currently only `xe-interface` exists under `Common`. A single top-level `Common` folder is sufficient and avoids excessive hierarchy.
3. **Is a `ThirdParty` folder wanted, and where?**
   - **Resolved**: Third-party packages are Conan `IMPORTED` targets and do not generate `.vcxproj` projects in the solution. If in-tree or `FetchContent` dependencies are added later, a top-level `ThirdParty` folder will be used.
4. **Any targets shared between Engine and IDE that blur the Common boundary?**
   - **Resolved**: `xe-interface` is the shared base. `xenoide-base` and `xenoide-sanitize` remain under `IDE` since they are currently used exclusively within the IDE tree.

---

## Notes
- Target folders (`FOLDER` property) only affect IDE-generated solutions (Visual Studio, Xcode). They have no effect on Ninja or Makefile builds.
- In Visual Studio "Open Folder" mode, use the CMake Targets view to see the folder hierarchy.
