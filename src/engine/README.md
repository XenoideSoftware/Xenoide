
# XE

## Building with Docker
- Clone a separate repository for building with Docker.

## Quick thoughts
- Barebones for a future multiplatform game engine.
- Contains code for libraries, tools, and simple game and demos

## Documentation

The XE engine is documented in-source with Doxygen blocks on every public
class, struct, free function, and method, following the project's
[Code Conventions](CLAUDE.md#code-conventions). The math library
(`libxe-math`) is fully documented; the rest of the libraries are being
brought up to the same standard incrementally.

A single project-wide Doxygen run produces the HTML output. Opt in via
the Conan `with_docs` option — Conan will then pull `doxygen` as a
build-time tool dependency and CMake will expose a `docs` target driven
by the root [`Doxyfile.in`](Doxyfile.in).

```bash
# 1. Configure with documentation generation enabled.
conan install . --build=missing -o with_docs=True \
    --profile=conan/profiles/<your-profile> --settings=build_type=Release
cmake --preset conan-release

# 2. Build the documentation.
cmake --build --preset conan-release --target docs
```

The build emits two trees under `build/Release/docs/`:

| Path | Purpose |
|---|---|
| `html/index.html` | Browsable HTML reference, MathJax-rendered formulas, source cross-links |
| `xml/`            | XML output for downstream tooling (Sphinx/Breathe, doxygen2html, etc.) |

If `with_docs` is left at its default (`False`), Conan does not pull
Doxygen and CMake leaves `XE_DEV_DOCS` off, so existing build flows are
unaffected. The toggle can also be flipped directly at the CMake level
via `-DXE_DEV_DOCS=ON` for developers who already have Doxygen installed
system-wide.

## Vulkan 
- Needs VulkanSDK to be downloaded manually, because the Hunter package manager provided an old version that didn't worked with the current implementation

## GLTF loader 

Implement 3d model gltf loading for testing the renderer methods

- Don't allocate any memory for the loader (like the current implementation)
- Return a 3d model structure that allows for fast rendering
- Consider that 3d model can contains sub objects that can form a hierarchy
