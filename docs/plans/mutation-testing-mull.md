# Mutation Testing Workflow & Implementation Plan (Mull)

This document outlines the workflow and implementation plan for integrating **Mull** mutation testing into the **Xenoide** project.

---

## 1. Executive Summary & Goals

Mutation testing evaluates the fault-detection capability of our unit tests by injecting small syntactic changes (mutants) into the source code and verifying whether the test suite fails (killing the mutant) or passes (surviving mutant).

Key objectives:
- **Dedicated build directory**: Isolate Mull-instrumented binaries in `build-mutation-mull/`.
- **Mise task integration**: Expose `mutation:mull` via `mise.toml` with flags for mutant generation (`--generate`) and execution/killing (`--kill`).
- **Mull runner & reporter integration**: Run Mull across multiple test targets (`xe-core-test`, `xe-geometry-test`, etc.), aggregate findings in an SQLite database, and generate actionable IDE diagnostics, interactive HTML reports (Mutation Testing Elements), and mutant patch diffs.
- **Project configuration**: Provide `mull.yml` to filter out non-application code (Catch2, Conan packages, test suites) and select mutation operators.

---

## 2. Prerequisites & Architecture Note

### Compiler & Plugin Compatibility

Mull instruments code at compile time via an LLVM pass plugin:
```
-fpass-plugin=/usr/lib/mull-ir-frontend-<VERSION> -O0 -g -grecord-command-line
```

> **Important**: LLVM pass plugins are tightly bound to the exact LLVM major version ABI.
> - The environment has `mull-19` (`/usr/lib/mull-ir-frontend-19`, `/usr/bin/mull-runner-19`, `/usr/bin/mull-reporter-19`) and `clang-19` (`clang-19`, `clang++-19`) installed.
> - Plugin compatibility between `clang-19` and `/usr/lib/mull-ir-frontend-19` has been tested and verified.
> - The orchestration scripts automatically detect and pair matching versions (`mull-runner-19` + `clang++-19`, or `mull-runner-18` + `clang++-18`), and fail fast if a version mismatch is detected.

---

## 3. Workflow Design

```
   +----------------------------------------------------------------+
   | 1. Configuration & Build (build-mutation-mull/)                |
   |    - conan install (into build-mutation-mull/Debug)            |
   |    - cmake configure with -fpass-plugin=/usr/lib/mull-ir-frontend|
   |    - cmake --build test targets                                |
   +----------------------------------------------------------------+
                                   |
                                   v
   +----------------------------------------------------------------+
   | 2. Execution via mull-runner                                   |
   |    - Target Discovery: ctest --show-only=json-v1               |
   |    - Mode Check:                                               |
   |      * --generate: mull-runner --dry-run                       |
   |      * --kill:     mull-runner (active test execution)         |
   |    - Shared SQLite DB: mutation/mull/xenoide.sqlite            |
   +----------------------------------------------------------------+
                                   |
                                   v
   +----------------------------------------------------------------+
   | 3. Information Gathering & Reporting (mull-reporter)           |
   |    - mull-reporter analyzes aggregated xenoide.sqlite          |
   |    - IDE Reporter: Terminal warnings (file:line:col)           |
   |    - Elements Reporter: Interactive HTML report                |
   |    - Patches Reporter: Source .patch files per mutant          |
   +----------------------------------------------------------------+
```

### Flags Specification

| Flag | Description | Underlying Mull Runner Option |
|---|---|---|
| `--generate` | Discovers and generates mutants without running tests (fast dry run) | `--dry-run` |
| `--kill` | Executes unit tests against each mutant to kill them (default mode) | *(active test execution)* |
| `--config <cfg>` | Build type: `Debug` (default) or `Release` | `-DCMAKE_BUILD_TYPE=<cfg>` |
| `--reporters <list>` | Output formats: `IDE,SQLite,Elements,Patches,Sarif` | `--reporters ...` |
| `--output-dir <dir>` | Directory for reports (default: `mutation/mull`) | `--report-dir ...` |
| `--report-name <name>` | Report database base name (default: `xenoide`) | `--report-name ...` |
| `--clean` | Deletes existing build/mutation artifacts before running | `rm -rf build-mutation-mull mutation/mull` |
| `--target <name>` | Run against a specific test target (e.g. `xe-core-test`) | Select single binary |
| `--timeout <ms>` | Maximum test execution timeout per mutant | `--timeout <ms>` |
| `--threshold <score>` | Minimum required mutation score (0-100) | `--mutation-score-threshold <score>` |

---

## 4. Multi-Target Information Gathering

Xenoide splits its test suite into several focused Catch2 binaries:
- `xe-core-test`
- `xe-geometry-test`
- `xe-glaze-test`
- `libxe-graphics-test`
- `xe-math-test`

Running `mull-runner` target-by-target with independent reports can lead to duplicated mutant counts or inaccurate survival rates when a mutant in an engine library is tested by multiple test binaries.

### Multi-Target Aggregation Strategy:
1. **Accumulate into SQLite**:
   Each test executable is passed to `mull-runner` using:
   ```bash
   mull-runner \
       --reporters SQLite \
       --report-dir mutation/mull \
       --report-name xenoide \
       --allow-surviving \
       "$test_binary"
   ```
   Mull automatically registers all mutants and test results in `mutation/mull/xenoide.sqlite`.
2. **Consolidate with `mull-reporter`**:
   Once all targets finish, `mull-reporter` processes `xenoide.sqlite`:
   ```bash
   mull-reporter \
       --reporters IDE \
       --reporters Elements \
       --reporters Patches \
       --report-dir mutation/mull \
       --report-name xenoide \
       mutation/mull/xenoide.sqlite
   ```
3. **Artifacts Produced**:
   - `mutation/mull/xenoide.sqlite`: Queryable database of all mutants and results.
   - `mutation/mull/xenoide.html`: Mutation Testing Elements HTML report with code viewer and mutation badges.
   - `mutation/mull/patches/`: Directory containing patch files for inspecting individual mutants.

---

## 5. Implementation Steps & File Changes

### Step 1: Create `mull.yml`
Configure active mutators and exclusion patterns:
```yaml
mutators:
  - cxx_add_to_sub
  - cxx_sub_to_add
  - cxx_mul_to_div
  - cxx_div_to_mul
  - cxx_rem_to_div
  - cxx_logical_and_to_or
  - cxx_logical_or_to_and
  - cxx_remove_negation
  - cxx_eq_to_ne
  - cxx_ne_to_eq
  - cxx_le_to_gt
  - cxx_lt_to_ge
  - cxx_ge_to_lt
  - cxx_gt_to_le
  - cxx_assign_const
  - cxx_init_const

excludePaths:
  - .*/Catch2/.*
  - .*/\.conan2/.*
  - .*/tests?/.*
  - .*/unit-test/.*
  - .*/libxe-.*-test/.*
  - .*/include/c\+\+/.*
  - /usr/.*

includePaths:
  - .*/src/.*

parallelization:
  workers: 4
  executionWorkers: 4
```

### Step 2: Implement Orchestration Script (`mise/mutation.sh`)
Create `mise/mutation.sh` with:
- CLI option parsing (`--generate`, `--kill`, `--config`, etc.)
- Mull/Clang compatibility check and binary selection
- Conan setup in `build-mutation-mull/<CONFIG>`
- CMake build of test targets with Mull compiler flags
- Test target discovery and execution loop
- Consolidated report generation with `mull-reporter`

### Step 3: Add Windows Counterpart (`mise/mutation.ps1`)
Provide equivalent PowerShell runner for Windows developers.

### Step 4: Add Task to `mise.toml`
Register `tasks."mutation:mull"` with usage specifications matching other Xenoide tasks.

### Step 5: Update `.gitignore`
Add `mutation/` to `.gitignore` (build folder `build-mutation-mull/` is already ignored under `build-*/`).

---

## 6. Verification & Usage Examples

1. **Discover and generate mutants without running tests**:
   ```bash
   mise run mutation:mull --generate
   ```

2. **Execute and kill mutants on a single target**:
   ```bash
   mise run mutation:mull --kill --target xe-core-test
   ```

3. **Full mutation suite with HTML dashboard**:
   ```bash
   mise run mutation:mull --reporters IDE,SQLite,Elements
   # Open mutation/mull/xenoide.html in a web browser
   ```
