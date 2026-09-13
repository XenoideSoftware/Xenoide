# libxe-graphics

The XE engine's graphics rendering library: a portable graphics abstraction layer and a reference OpenGL 3 Core Profile implementation.

## Purpose

This target consolidates three previously separate graphics subtrees:

- `xe/graphics/` — the device/context/texture/shader/material abstraction layer (originally `src/libxe-core/src/xe/graphics/`).
- `xe/gl/` — the OpenGL 3 Core Profile reference backend (originally the standalone `src/libxe-gl/` library).
- `xe/render/` — a newer backend-abstraction prototype built around a flat vtable, typed handles, and a GL Core 3 reference backend (originally `src/xe-glaze-test/src/xe/render/`).

All three represent co-existing iterations of the XE rendering abstraction. Consolidating them under a single target is the first step toward merging them into a single coherent API.

## Design Spec

The authoritative design document for the rendering subsystem is [`docs/rendering-subsystem-spec.md`](docs/rendering-subsystem-spec.md) (Revision 0.7). It describes the target API surface (`RenderDevice`, `CommandBuffer`, `BackendVTable`, handle encoding, backend registry, tier model, asset pipeline). The C4 architecture models referenced by the spec live under the repository-level [`docs/architecture/`](../../docs/architecture/) workspace.

Current source code does not yet fully implement the spec — treat the spec as the direction for the ongoing consolidation/refactor work, not as documentation of the present state.

## Directory Map

```
libxe-graphics/
  CMakeLists.txt            # library target libxe-graphics (alias xe::graphics)
  README.md
  docs/
    rendering-subsystem-spec.md
  src/
    xe/
      GL.h                  # top-level GL include
      graphics/             # portable abstraction (Device, Context, Texture*, Shader, Program, Material, …)
      gl/                   # GL 3/4 reference backend (Glaze-based)
      render/               # vtable+handle backend prototype
        backend/
          glcore3-api.*     # GL Core 3 backend for the prototype
  unit-test/                # Catch2 suite for the GL reference backend
```

## Building and Testing

`libxe-graphics` builds as part of the top-level XE build (see root [`CLAUDE.md`](../../CLAUDE.md) for the Conan + CMake workflow). It is enabled through `src/CMakeLists.txt` and inherits the engine-wide CMake options (`XE_DEV_TESTING_ENABLE`, `XE_DEV_PCH_ENABLE`, etc.).

Public dependencies: `xe::core`, `xe::imageloader`, `glaze::gl`, `fmt`, `glm`, `glfw`, `Backport`, `Microsoft.GSL`.

Quick cycle:

```bash
# configure + build (Debug)
conan install . --build=missing --profile=conan/profiles/<your-profile> --settings=build_type=Debug
cmake --preset conan-debug
cmake --build --preset conan-debug

# run the GL unit tests
ctest --test-dir build/Debug -R libxe-graphics-test --output-on-failure
```

## Status

This library is in active redesign. Downstream consumers (`xe-gltf-view`, `capybaria`, `libxe-core-platform-glfw`, `xe-glaze-test`) still reference the pre-consolidation include paths and library names; their migrations to `xe::graphics` are being done in follow-up tasks.
