# Implementation Plan: CMake Checker FixIt Testing

## Goal Description

Validate the automated `Fix` (fixit) capability of the `xe-cmake-checker` v2.2 stack in a **completely isolated sandbox** before any live fixit is ever applied to repository CMake projects. Per the locked decisions in `docs/plans/CMAKE_STYLE_CHECKER_V2.2.md`:

> "Fixits are tested exclusively within an isolated synthetic sandbox project via a separate plan artifact to avoid corrupting repository CMake projects."

This plan is that artifact. It defines the isolated sandbox fixture, the DSL/ChaiScript fixit verification matrix, the idempotency and build-integrity checks, and the production gate.

## 1. Isolated Sandbox Test Project

A dedicated synthetic CMake fixture project is maintained at **`tests/fixtures/cmake-fixit-sandbox/`**. This sandbox is completely decoupled from repository build targets and intentionally incorporates a violation of **every** guideline in `docs/CMAKE.md`:

- **Multiple targets declared within a single directory** (`structure.single-target-per-folder`).
- **Target names conflicting with folder names** (`naming.target-matches-folder`).
- **Unquoted source files in `set (sources ...)`** (`formatting.quote-source-paths`).
- **Multi-argument `target_link_libraries` calls** (`formatting.target-link-single-dependency`).
- **Missing library ALIAS and missing `target_include_directories`** (`library.alias-specification`, `library.include-directories-src`).
- **Incomplete Catch2 test setups** (`testing.catch2-specification`).

Example fixture layout:

```
tests/fixtures/cmake-fixit-sandbox/
├── CMakeLists.txt
├── libxe-alpha/
│   └── CMakeLists.txt          # unquoted sources, no ALIAS, no include dirs
├── xe-app/
│   └── CMakeLists.txt          # multi-dependency target_link_libraries
└── libxe-alpha-test/
    └── CMakeLists.txt          # incomplete Catch2 setup
```

The sandbox is excluded from the main repository build (it is never added to `src/cmake/CMakeLists.txt`).

## 2. FixIt Verification Matrix

### 2.1 DSL Fix Template Validation in Sandbox

Executed against sandbox listfiles using `FixTemplateEngine` + `SyncWriter` over `InMemoryFileSystem`:

| Template | Sandbox Trigger | Expected Repair |
| :--- | :--- | :--- |
| `split_target_link_libraries_per_line` | multi-argument `target_link_libraries` | one command per dependency with per-dependency scope |
| `quote_argument` | unquoted source path in `set (sources ...)` | argument wrapped in `"..."` |

Invariants enforced:
- `requireWorkspaceEditNonOverlapping(workspace_edit)`
- `requireWorkspaceEditSplicingValid(workspace_edit, source_text)`
- Post-splice reparse with zero errors and valid spans.

### 2.2 ChaiScript Procedural Fixit Validation in Sandbox

Executed via `ScriptRuleLoader`:

| Rule | Sandbox Trigger | Expected Repair |
| :--- | :--- | :--- |
| `naming.target-matches-folder` | folder `libxe-alpha`, declared `set (target "alpha_legacy_internal")` | `TextEdit.replace` of the declared name; graph-driven multi-file edits to update consumers |
| `library.include-directories-src` | static library missing `target_include_directories` | insert `target_include_directories(${target} PUBLIC "src")` |

Validates that reverse-offset text splicing preserves byte-exact formatting trivia and produces clean `git diff`s.

## 3. Idempotency & Build Integrity Verification

1. **Pass 1**: Applying fixits resolves the target diagnostics (`recheck` findings for repaired rule ids are empty).
2. **Pass 2**: A subsequent run produces **zero diffs** (fixit idempotency).
3. **Build Validation**: Invoking `cmake -B <build_dir>` on the repaired sandbox project executes without configuration errors (syntax-level validity of the repaired listfiles).

## 4. Production Gate

Under no circumstances will `--fix` be executed on `src/engine` or `src/ide` until the entire verification matrix above has been successfully executed and approved. The safe engine verification track remains strictly `--check` only:

```bash
mise run cmake-check:release
```

## 5. Relation to the Quality Gates

The fixit testing here complements the four mandatory post-implementation phases of `CMAKE_STYLE_CHECKER_V2.2.md`:

| Gate | Coverage |
| :--- | :--- |
| Phase 1: Unit tests | `FixTemplateEngine`, `SyncWriter`, `FixConflictResolver` unit tests |
| Phase 2: Coverage | > 95% line coverage across all `libxe-cmake-checker-*` libraries |
| Phase 3: Mutation | Mull gates for core / dsl / script / rule-engine |
| Phase 4: E2E | `xe-cmake-checker-e2e-test` in-memory full-stack DSL + ChaiScript fixit tests |
| This plan | Isolated on-disk sandbox + idempotency + build-integrity validation |