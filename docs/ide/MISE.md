# Mise Task Runner Guide

[Mise](https://mise.jdx.dev/) is used as the unified task runner and development environment manager for Xenoide. It standardizes build, test, setup, and linting workflows across Linux, macOS, and Windows.

---

## Prerequisites

- **Mise**: Install via instructions at [mise.jdx.dev](https://mise.jdx.dev/) (e.g., `curl https://mise.run | sh` or `brew install mise` or `winget install jdx.mise`).
- **CMake**: 3.25+
- **Conan**: 2.x
- **C++20 Compiler**: GCC 13+, Clang 17+ / Apple Clang 17+, or MSVC 19.4+
- **Clang Tools**: `clang-format` and `clang-tidy` (for formatting and linting tasks)

---

## Quick Reference

| Command | Description | Dependencies / Subtasks |
|---|---|---|
| `mise run setup` | Install Conan dependencies (Release) | `install:release` -> `export-recipes` |
| `mise run install:debug` | Install Conan dependencies (Debug) | `install:debug` -> `export-recipes` |
| `mise run export-recipes` | Export local recipes in `conan/packages/*` to cache | None |
| `mise run configure` | Configure CMake (Release) | `configure:release` -> `install:release` |
| `mise run configure:debug` | Configure CMake (Debug) | `configure:debug` -> `install:debug` |
| `mise run build` | Build Release binaries | `build:release` -> `configure:release` |
| `mise run build:debug` | Build Debug binaries | `build:debug` -> `configure:debug` |
| `mise run test` | Run test suite (Release) | `test:release` -> `build:release` |
| `mise run test:debug` | Run test suite (Debug) | `test:debug` -> `build:debug` |
| `mise run format` | Format source code in `src/` with `clang-format` | None |
| `mise run tidy` | Run `clang-tidy` on modified or specified files | None (requires build directory) |
| `mise run clean` | Remove `build` and `build-*` directories | None |

---

## Task Dependency Pipeline

Mise automatically resolves task dependencies. When you run a downstream task like `mise run build`, Mise runs all prerequisite tasks if needed:

```
export-recipes
        │
        ▼
   install:release  (or install:debug)
        │
        ▼
 configure:release (or configure:debug)
        │
        ▼
   build:release  (or build:debug)
        │
        ▼
    test:release   (or test:debug)
```

---

## Detailed Task Documentation

### 1. Setup & Package Management

#### `export-recipes`
- **Description**: Scans `conan/packages/` for local Conan recipes and exports them into the local Conan cache.
- **Details**: Detects version information in `conanfile.py` or parses version matrices in `conandata.yml`.
- **Usage**:
  ```bash
  mise run export-recipes
  ```

#### `setup` / `install:release`
- **Description**: Installs Conan dependencies using the Release configuration.
- **Details**: Automatically detects host OS and selects `conan/profiles/unix` on Linux/macOS or `conan/profiles/windows` on Windows/MSYS.
- **Usage**:
  ```bash
  mise run setup
  # or explicitly
  mise run install:release
  ```

#### `install:debug`
- **Description**: Installs Conan dependencies using the Debug configuration (`-s build_type=Debug`).
- **Usage**:
  ```bash
  mise run install:debug
  ```

---

### 2. CMake Configuration

#### `configure` / `configure:release`
- **Description**: Configures CMake with the Release preset (`conan-release` on Unix/MSYS, `conan-default` on Windows multi-config).
- **Usage**:
  ```bash
  mise run configure
  ```

#### `configure:debug`
- **Description**: Configures CMake with the Debug preset (`conan-debug` on Unix/MSYS, `conan-default` on Windows multi-config).
- **Usage**:
  ```bash
  mise run configure:debug
  ```

---

### 3. Compilation & Building

#### `build` / `build:release`
- **Description**: Builds Release binaries in parallel.
- **Details**: Uses `cmake --build --preset conan-release --parallel` (or `--preset conan-default --config Release --parallel`).
- **Output Executable**: `build/Release/xenoide` (or `build/xenoide` depending on preset).
- **Usage**:
  ```bash
  mise run build
  ```

#### `build:debug`
- **Description**: Builds Debug binaries in parallel.
- **Usage**:
  ```bash
  mise run build:debug
  ```

---

### 4. Testing

#### `test` / `test:release`
- **Description**: Runs test suite using CTest with `--output-on-failure`.
- **Usage**:
  ```bash
  mise run test
  ```

#### `test:debug`
- **Description**: Runs test suite for Debug build.
- **Usage**:
  ```bash
  mise run test:debug
  ```

---

### 5. Code Quality & Maintenance

#### `format`
- **Description**: Recursively formats all C/C++ source and header files (`.cpp`, `.h`, `.hpp`, `.c`, `.cc`, `.cxx`) in `src/` using `clang-format -i`.
- **Usage**:
  ```bash
  mise run format
  ```

#### `tidy`
- **Description**: Runs `clang-tidy` against `compile_commands.json` found in build directories.
- **Details**:
  - If files are passed as arguments, runs `clang-tidy` on those files: `mise run tidy src/xenoide/main.cpp`.
  - If no arguments are provided, inspects `git diff` for modified C++ files in `src/`.
  - If no modified files are detected in git diff, checks all C++ source files in `src/`.
- **Usage**:
  ```bash
  # Tidy git-modified files
  mise run tidy

  # Tidy specific file
  mise run tidy src/xenoide/MainWindow.cpp
  ```

#### `clean`
- **Description**: Cleans build artifacts by removing `build` and `build-*` directories.
- **Usage**:
  ```bash
  mise run clean
  ```

---

## Configuration File

All tasks are defined in the repository root [`mise.toml`](../mise.toml).
