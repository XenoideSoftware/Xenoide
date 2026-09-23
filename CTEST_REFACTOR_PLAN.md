# CTest / Unit Test Refactor Plan (CMake)

## 1. Shared test helper module (new)

Create `src/cmake/XenoideTesting.cmake` with:

```cmake
function(xe_add_unit_test target)
  cmake_parse_arguments(ARG "" "" "SOURCES;LIBRARIES;LABELS" ${ARGN})
  add_executable(${target} ${ARG_SOURCES})
  target_link_libraries(${target} PRIVATE
      Catch2::Catch2WithMain ${ARG_LIBRARIES} xe::compile-options)
  include(Catch)
  catch_discover_tests(${target})
  set_tests_properties(${target} PROPERTIES LABELS "unit;${ARG_LABELS}")
endfunction()
```

Wire it into the module path once in `src/CMakeLists.txt`
(`list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")`) so every
subdirectory can `include(XenoideTesting)`.

## 2. Rename the flag-carrier (`xe::interface` → `xe::compile-options`)

In `src/base-target/CMakeLists.txt`, rename target `xe-interface` →
`xe-compile-options` (alias `xe::compile-options`) since it only carries
`-Werror`/coverage/sanitizer flags. Update all ~23 references (mechanical;
libraries and tests). Tests get it automatically via the helper.

## 3. Unify the test flag onto `BUILD_TESTING`

- Root `CMakeLists.txt`: call `include(CTest)` unconditionally (this defines
  `BUILD_TESTING`, default ON); keep `XE_ENABLE_TESTING` as a deprecated alias
  that sets `BUILD_TESTING`, or drop it. Gate `find_package(Catch2 3 CONFIG
  REQUIRED)` on `BUILD_TESTING`.
- `conanfile.py`: emit `BUILD_TESTING` instead of `XE_ENABLE_TESTING`.
- `src/engine/src/CMakeLists.txt`: replace `XE_ENABLE_TESTING` with
  `BUILD_TESTING` for `libxe-core-test`/`libxe-geometry-test`.
- `src/engine/src/libxe-math/CMakeLists.txt` + `libxe-graphics-gl/CMakeLists.txt`:
  same swap.
- `src/ide`: fold `XENOIDE_TESTS` into `BUILD_TESTING` (see item 5).

## 4. Fix inconsistent discovery

`src/engine/src/libxe-math/test/CMakeLists.txt`: replace
`add_test(NAME … COMMAND …)` with the `xe_add_unit_test` helper (per-`TEST_CASE`
discovery).

## 5. Rename GL programs to `-demo` + gate behind `XE_ENABLE_DEMOS`

- Add `option(XE_ENABLE_DEMOS "Build GL demo applications" OFF)` in root
  `CMakeLists.txt` (was previously always-built).
- `src/engine/src/CMakeLists.txt:42-43`: gate on `XE_ENABLE_DEMOS`, update
  subdir names.
- Rename dirs/targets/sources:
  - `xe-render-core-3d-test/` → `xe-render-core-3d-demo/`,
    target `xe-render-core-3d-demo`, source `xe-render-core-3d-demo.cpp`.
  - `xe-glaze-test/` → `xe-glaze-demo/`, target `xe-glaze-demo`; inner target
    `xe-render-core` → `xe-render-core-demo`, sources `xe-glaze-demo.cpp` /
    `xe-render-core-demo.cpp`.

## 6. Wire the IDE tests

- `src/ide/CMakeLists.txt`: add `include(CTest)` + gated
  `find_package(Catch2 3 CONFIG REQUIRED)` so the standalone IDE recipe can
  enable tests.
- `src/ide/src/CMakeLists.txt:29-31`: uncomment `add_subdirectory("xenoide-test")`
  gated on `BUILD_TESTING`.
- `xenoide-test/CMakeLists.txt`, `xenoide-ui-codeeditor/CMakeLists.txt`,
  `xenoide-ui-folderbrowser/CMakeLists.txt`: replace `XENOIDE_TESTS` with
  `BUILD_TESTING` and use `xe_add_unit_test`.
- **Caveat:** the standalone IDE recipe (`src/ide/conanfile.py`) exports only
  `src/ide/*`; the shared module lives in repo-root `src/cmake/`. Plan: also
  ship a copy/symlink of `XenoideTesting.cmake` under `src/ide/src/cmake/` (or
  keep the IDE's helper inline).

## 7. CTest metadata

- Helper assigns `unit` label by default; allow `LABELS integration` for
  GL/graphics tests (`libxe-graphics-gl/unit-test`).
- For asset-dependent tests, set `WORKING_DIRECTORY`/`ENVIRONMENT`
  (`XE_EXTERNAL_ASSET_ROOT_PATH`) via optional helper args rather than relying on
  global `add_definitions`.

## 8. Align Catch2 + CI + docs

- Bump `src/ide/conanfile.py` catch2 `3.7.1` → `3.14.0` to match root.
- Add `.github/workflows/ci.yml` (Conan → CMake configure → build →
  `ctest --output-on-failure`).
- Add a short "Running tests" section to `README.md`.

## Verification

- Configure/build with `BUILD_TESTING=ON` and `XE_ENABLE_DEMOS=ON`.
- `ctest -N` shows per-test-case entries for math/core/geometry/graphics;
  `ctest --output-on-failure` passes.
- Configure with `BUILD_TESTING=OFF` → no Catch2 lookup, no test targets.
- Confirm demos build only under `XE_ENABLE_DEMOS=ON`.

## Open decisions

1. Exact `-demo` source-file renames (proposed: yes, for full consistency).
2. `XE_ENABLE_DEMOS` default: ON (lean default).

