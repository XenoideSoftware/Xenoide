# AGENTS.md

## 1. Project Vision & Context

**Xenoide** is a high-performance software and game creation platform comprising several components. Currently, there are three main areas of work, for this **monorepo**:

1. **A Custom Engine (`src/engine/`)**.
2. **A Native Integrated IDE (`src/ide/`)**.
3. **Shared Build Options Infrastructure (`src/base-target/`)**.

Eventually, Xenoide will provide native UIs implemented, initially for 

- **Windows**: Windows API 
- **Linux**: GTK4 
- **macOS**. TDB

For development speed reasons, we will start with **Qt6**.

## 2. Core Technical Stack & Specifications
- **Language**: C++17, without extensions.
- **Build System**: CMake 3.25+.
- **Package Manager**: Conan 2.X.
- **Dev Tasks Orchestration**: Mise
- **Testing Framework**: Catch2 v3.
- **Code Formatting**: clang-format.
- **Static Analysis**: clang-tidy.


## Agent Instructions & Verification Checklist

### General instruction guidelines
- Ignore any file or folder referenced in `.gitignore` file: Usually 

### Specific language changes
- **C/C++**: Refer to @docs/CPP.md
- **CMake**: Refer to @docs/CMAKE.md
- **Conan**: Refer to @docs/CONAN.md

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
