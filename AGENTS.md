# AGENTS.md — xenoide (IDE)

This repository is the **Xenoide IDE**. Shared infrastructure and conventions
live in the `devkit` repository; link to it instead of copying rules.

## Read first

- `devkit/docs/CONVENTIONS.md` — repositories, naming, pins, configs, versions
- `devkit/docs/CPP.md` — strict C++17 rules
- `devkit/docs/CMAKE.md` — CMake target conventions
- `devkit/docs/CONAN.md` — recipes, base class, channels
- `devkit/docs/TESTING.md` — Catch2 testing rules

They are available at `.devkit/docs/` after `mise run setup`.

## Technical stack

- C++17, no extensions
- CMake 3.25+
- Conan 2.x
- mise for orchestration
- Catch2 v3
- Qt6 is the supported UI (`with_ide_qt6`); native Cocoa is deferred

## Workflow

```bash
mise install
mise run setup                 # .devkit + recipes + in-house tools + doctor
mise run build --config clang-debug
mise run test  --config clang-debug
mise run format
mise run tidy  --config clang-debug
mise run cmake-check --config clang-debug
```

Use `mise run use <config>` to switch the active config; clangd follows
`build/compile_commands.json`.

## Rules

- Do not reintroduce `../cmake` include paths or `XE*` CMake modules; use
  `include(XenoideConfig)` and `xenoide::interface`.
- In-house dependencies are bare `name/version`; never `@xenoide/xenoide`.
- Add `.devkit/`, `.deps/`, `.conan2/`, `.xenoide/` to `.gitignore` (already
  present).
- CI workflows remain disabled until the CI phase; do not re-enable them
  ad hoc.
