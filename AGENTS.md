# AGENTS.md

> **Authoritative Operational Guide and Source of Truth for Autonomous AI Agents and Contributors in Xenoide**
> Contains general guidelines across the project. Specific components / folder might have their own AGENTS.md with specific instructions.

---
## 0. AI Usage Guidelines

### Allowed AI-generated code:
- To create support utilities: code generators, checkers, scaffolding, etc.
- To support designing and creating unit tests.
- To perform code review.
- To create native UIs for each platform.
- To discuss different approaches via Q/A sessions.

### Not Allowed Usage
AI might assist here suggesting algorithms, approaches, testing strategies, etc, but rendering, algorithmic code **will remain implemented by humans**.

## 1. Project Vision & Context

**Xenoide** is a high-performance software and game creation platform comprising several components. Currently, there are two main areas of work, for this **monorepo**:

1. **A Custom Engine (`src/engine/`)**.
2. **A Native Integrated IDE (`src/ide/`)**.
3. **Shared Build Options Infrastructure (`src/base-target/`)**.

Eventually, Xenoide will provide native UIs implemented, initially for 

- **Windows**: Windows API 
- **Linux**: GTK4 
- **macOS**. TDB

## 2. Core Technical Stack & Specifications
- **Language**: C++17, without extensions.
- **Build System**: CMake 3.25+.
- **Package Manager**: Conan 2.X.
- **Dev Tasks Orchestration**: Mise
- **Testing Framework**: Catch2 v3.
- **Code Formatting**: clang-format.
- **Static Analysis**: clang-tidy.

## 3. Custom Conan Packages (`conan/packages/`)

Xenoide maintains custom Conan recipes inside `conan/packages/`. Several of these packages contain custom build logic, patches, or source code.

### Exporting Local Recipes
Whenever changes are made to any local recipe or in-tree package source (such as `glazer` or `glazed`), the recipes must be re-exported to the local Conan cache before installing dependencies:
```bash
mise run setup:export-recipes
```
This script iterates through each directory in `conan/packages/` and executes `conan export`.

## 4. C++ Coding Standards & Best Practices

### Standard & Language Features
- **Strict C++17**: Code must strictly conform to C++17. Do not use C++20 features (e.g., concepts, ranges library, `std::span` unless provided by `ms-gsl`/`gsl-lite` and the cpp-backport library, coroutines).
- **Vendor Extensions Disabled**: Do not rely on compiler-specific non-standard extensions.

### Zero-Warning Tolerance
- All code is compiled All Warnings and Warnings as Errors enabled.

### Design Principles & Idioms
- **RAII & Memory Safety**: Never leak raw pointers. Use `std::unique_ptr` for exclusive ownership and `std::shared_ptr` only when ownership is genuinely shared.
- **Value Semantics & Views**: Prefer `std::string_view` for read-only string parameters. Pass complex types by const-reference unless passing by value for sink parameters.
- **Error Handling**:
  - In the engine core and performance-critical loops, avoid throwing exceptions. Prefer monadic types (`tl::expected`, `std::optional`) or explicit result codes.
  - Ensure assertions (`XE_ASSERT` or equivalent) are used to enforce invariants in debug builds.
- **Namespaces**:
  - Engine code belongs in `namespace xe { ... }` or specific sub-namespaces (`xe::graphics`, `xe::math`, etc.).
  - IDE code belongs in `namespace xenoide { ... }`.
- **Include Order**:
  1. Main module header (e.g., `#include "MyClass.h"`).
  2. Subsystem internal headers.
  3. Third-party library headers (e.g., `<fmt/format.h>`, `<tl/expected.hpp>`).
  4. Standard library headers (e.g., `<vector>`, `<string>`, `<memory>`).

## 5. Agent Instructions & Verification Checklist

### General instruction guidelines
- Ignore any file or folder referenced in `.gitignore` file: Usually 

### Implementing new features 

When assigned a task in this repository, follow this systematic checklist:

1. **Check Local Custom Recipes**:
   - If any file in `conan/packages/` (especially `glazer`, `glazed`, or `winlamb`) is created or modified, immediately execute:
     ```bash
     mise run setup:export-recipes
     ```
2. **Setup Dependencies**:
   - Ensure the Conan cache and build layout are up-to-date:
     ```bash
     mise run setup:release
     # or for debug workflows
     mise run setup:debug
     ```
3. **CMake Configuration**:
   - Generate build system files:
     ```bash
     mise run configure:release
     ```
4. **Implement Code Changes**:
   - Adhere strictly to **C++17**.
   - Respect target grouping and module boundaries (e.g., do not introduce circular dependencies between `engine` and `ide`).
5. **Run Static Code Analysis**:
    - Run tidy to currently modified, with auto-fixes.
    - For those changes that tidy can't fix, use a conservative approach to fix them.
    ```bash
     mise run tidy:release --fix
     ```
6. **Compile & Verify (Zero Warnings)**:
   - Compile using Mise:
     ```bash
     mise run build:release
     ```
   - Resolve any warnings immediately (warnings are treated as errors).
7. **Run Automated Tests**:
   - Execute the test suite to ensure regressions were not introduced:
     ```bash
     mise run test:release
     ```
8. **Format Modified Code**:
   - Format currently modified source files before building:
     ```bash
     mise run format
     ```
   - Build in both debug and release (mise run build)
   - Run the unit tests to discard any regressions (mise run test)
