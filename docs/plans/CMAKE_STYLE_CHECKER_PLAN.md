# Implementation Plan: `cmake-checker` — Native CMake Style Checker

## Goal Description

Formalize the conventions documented in `docs/CMAKE.md` into an automated **style
checker** for the monorepo. The checker is a native **C++17 CLI tool**
(`cmake-checker`), built as a **monorepo component under `src/`** and wired into
`mise` tasks mirroring the existing `tidy` family.

The tool is a **checker only** — it does not auto-format files. It validates all
in-scope CMake files and reports rule violations with a non-zero exit code on
errors, so it can gate developer workflows and CI.

Parsing is split across two layers:

- **Semantic layer** — drives the installed `cmake` binary via
  `--trace-format=json-v1` and the **CMake File API** (`codemodel` v2 +
  `cmakeFiles`). This provides version-accurate, official parse products (no
  vendored CMake source, no linking against CMake internals).
- **Lexical layer** — a small hand-written CMake listfile lexer over the raw
  files, needed because CMake's trace/file-api discard whitespace, comments and
  exact token spans.

---

## User Review Required

> [!IMPORTANT]
> **Locked decisions** (confirmed during design review):
>
> | Decision        | Choice                                                             |
> | --------------- | ------------------------------------------------------------------ |
> | Tool name       | `cmake-checker` (CLI); core library `cmake-checker-core`           |
> | Capability      | Checker only (no formatter)                                        |
> | Language        | C++17 native (Catch2-tested, zero warnings)                        |
> | Parsing         | Subprocess `cmake` (trace + File API) + own lexer                  |
> | Packaging       | Monorepo component: `src/cmake-checker`, `src/libcmake-checker-core`, `src/cmake-checker-test` |
> | Tool location   | Built in-tree; binary at `src/cmake-checker/build-cmake-check/<cfg>/bin/cmake-checker` |
> | Config format   | YAML → `.cmake-check.yaml` via `rapidyaml` (verified: ConanCenter `rapidyaml/0.7.2`, target `ryml::ryml`) |
> | JSON parsing    | `nlohmann_json/3.12.0` (trace + File API)                          |
> | CLI parsing     | `cxxopts/3.3.1` (consistent with `src/engine`)                     |
> | fmt             | `fmt/[>=11 <12]` (aligned with `src/engine/conanfile.py`)          |
> | Catch2          | `catch2/3.14.0` (aligned with `src/engine/conanfile.py`)           |
> | Tool build type | `Release` for the checker binary itself                            |
> | Scope           | `src/engine` only (review: removed `src/ide` and `src/pocs`)       |

> [!WARNING]
> **`docs/CMAKE.md` reconciliation (review item 6) is tracked by the owner.**
> The rule assumptions below are the confirmed encoding target and stay
> authoritative:
>
> 1. **Target vs folder naming.** Real folders are `libxe-*` / `libxenoide-*`
>    while targets drop the `lib` prefix (e.g. `libxe-imageloader-il` → target
>    `xe-imageloader-il`; `libxe-core` → `xe-core`). The confirmed rule is
>    type-dependent (see **R2**).
> 2. **Space before parenthesis.** The confirmed rule (and the codebase's
>    dominant form) is the no-space form (`set(`) — see **L1**.
> 3. **Link keyword ordering.** Explicit `PUBLIC`/`PRIVATE`/`INTERFACE` are
>    required, in the fixed order `PUBLIC` → `PRIVATE` → `INTERFACE` (see **R4**).

> [!NOTE]
> **Test targets are a special case.** A target is classified as a *test* if any
> of the following hold: its folder name ends with `-test`, it links
> `Catch2::Catch2WithMain`, or it calls `catch_discover_tests`. Test targets are
> validated by **R2c** (strip `lib`, require `-test` suffix), not R2b.

---

## Architecture & Layout Plan

```mermaid
flowchart TB
    subgraph mise ["mise tasks"]
        IC["install:cmake-check:&lt;cfg&gt;<br/>conan install + build the tool<br/>into build-cmake-check/&lt;cfg&gt;"]
        CC["configure:cmake-check:&lt;cfg&gt;<br/>src/engine: cmake --trace-format=json-v1<br/>+ file-api query"]
        CK["cmake-check:&lt;cfg&gt;<br/>invoke the built tool"]
    end

    subgraph tool ["cmake-checker (C++17)"]
        CLI["cmake-checker<br/>src/cmake-checker (cxxopts CLI)"]
        CORE["cmake-checker-core<br/>src/libcmake-checker-core"]
        DRV["CmakeDriver<br/>(subprocess cmake)"]
        TR["TraceParser<br/>(nlohmann_json)"]
        FA["FileApiParser<br/>(nlohmann_json)"]
        LX["Lexer<br/>(listfile tokens)"]
        RE["RuleEngine"]
        CFG["Config (rapidyaml)"]
        REP["Report (text / json)"]
    end

    IC --> BIN["src/cmake-checker/build-cmake-check/&lt;cfg&gt;/bin/cmake-checker"]
    BIN --> CK
    CC --> TREE["src/engine/build-cmake-check/&lt;cfg&gt;/<br/>trace.json + .cmake/api/v1/reply"]
    TREE --> CK
    CK --> CLI
    CLI --> CORE
    CORE --> DRV
    DRV --> TR
    DRV --> FA
    CORE --> LX
    CFG --> RE
    TR --> RE
    FA --> RE
    LX --> RE
    RE --> REP
```

### Parsing pipeline

1. The `configure:cmake-check:<cfg>` task issues a config-independent
   **File API query** (`codemodel` v2 + `cmakeFiles`) by writing
   `src/engine/build-cmake-check/<cfg>/.cmake/api/v1/query/client-xe-cmake-check/query.json`.
2. It configures `src/engine` with trace enabled:
   `cmake --preset conan-<cfg> --trace-format=json-v1 --trace-redirect=<dir>/trace.json`.
3. `cmake-checker` consumes:
   - `trace.json` — every executed command with `file`, `line`, `line_end`,
     `cmd`, `args` (semantic rules R4, R5, R6, R7).
   - File API `codemodel` v2 — folders, targets, source attribution
     (semantic rules R1, R2, R3).
   - File API `cmakeFiles` — authoritative list of every listfile CMake read,
     used to enumerate raw files for the lexical pass (L1–L6).
4. The lexer parses each enumerated raw file; the rule engine evaluates all
   enabled rules; the reporter prints findings (human-readable by default,
   `--json` for machines).

> [!NOTE]
> A dedicated `build-cmake-check/<cfg>/` directory (analogous to `build-tidy/`)
> keeps the normal build tree untouched and guarantees the trace/file-api data
> is fresh.

---

## Proposed Changes

### Component 1: Monorepo Component & Build

The checker is a monorepo component with three sibling, single-target folders
(per `docs/CMAKE.md`: one target per folder, target name == folder name, with
`lib` stripped for libraries).

```
src/
├── cmake-checker/            # CLI logic (project root: conanfile.py + CMakeLists.txt)
│   ├── conanfile.py
│   ├── CMakeLists.txt        # defines executable target `cmake-checker`; add_subdirectory's siblings
│   └── src/                  # CLI sources (cxxopts parsing, main.cpp)
├── libcmake-checker-core/    # core logic
│   ├── CMakeLists.txt        # defines library target `cmake-checker-core`
│   └── src/                  # lexer, driver, parsers, rules, config, report
└── cmake-checker-test/       # unit tests for the core lib
    ├── CMakeLists.txt        # defines test target `cmake-checker-test`
    └── src/                  # Catch2 v3 tests
```

##### `src/cmake-checker/conanfile.py`

```python
class CmakeCheckerConan(ConanFile):
    name = "cmake-checker"
    version = "0.0.0"
    description = "Xenoide CMake style checker"
    license = "MIT"
    package_type = "application"

    settings = "os", "compiler", "build_type", "arch"

    options = {"with_tests": [True, False]}
    default_options = {"with_tests": False}

    exports_sources = "CMakeLists.txt", "src/*"

    def requirements(self):
        self.requires("nlohmann_json/3.12.0")
        self.requires("rapidyaml/0.7.2")   # verified on ConanCenter; CMake target: ryml::ryml
        self.requires("cxxopts/3.3.1")
        self.requires("fmt/[>=11 <12]")    # aligned with src/engine/conanfile.py

    def build_requirements(self):
        if self.options.with_tests:
            self.test_requires("catch2/3.14.0")   # aligned with src/engine/conanfile.py

    def layout(self): ...
    def generate(self): ...   # CMakeDeps + CMakeToolchain; XE_CMAKE_CHECKER_WITH_TESTS
    def build(self): ...      # configure / build / (test)
    def package(self): ...    # copy binary into <pkg>/bin
    def package_info(self):   # buildenv_info.prepend_path("PATH", bindir)
```

##### `src/cmake-checker/CMakeLists.txt`

Standard C++17 project header (see `src/engine/CMakeLists.txt`), with:
- `CMAKE_CXX_STANDARD 17`, `CMAKE_CXX_EXTENSIONS OFF`, `CMAKE_EXPORT_COMPILE_COMMANDS ON`.
- `list(APPEND CMAKE_MODULE_PATH "../cmake")` + `include(XEBaseTarget)` +
  `include(XETesting)` (same wiring as `src/engine`).
- Flattened runtime output (`CMAKE_RUNTIME_OUTPUT_DIRECTORY = ${CMAKE_BINARY_DIR}/bin`).
- Executable target `cmake-checker` linking `cmake-checker-core`.
- `add_subdirectory("../libcmake-checker-core")` and, when
  `XE_CMAKE_CHECKER_WITH_TESTS` is ON, `add_subdirectory("../cmake-checker-test")`.

---

### Component 2: Tool Sources

| Folder                  | Responsibility                                                                               |
| ----------------------- | -------------------------------------------------------------------------------------------- |
| `src/libcmake-checker-core/src/lexer/`    | Tokenize CMake listfiles (unquoted/quoted/bracket args, comments, newlines) preserving exact spans. |
| `src/libcmake-checker-core/src/driver/`   | Locate and invoke `cmake`; resolve build/prepare the File API query; capture trace output.    |
| `src/libcmake-checker-core/src/parsers/`  | `nlohmann_json` readers for `trace.json` (json-v1) and File API `codemodel`/`cmakeFiles` replies. |
| `src/libcmake-checker-core/src/rules/`    | Rule registry keyed by stable IDs; semantic + lexical rule implementations; severity handling. |
| `src/libcmake-checker-core/src/config/`   | `rapidyaml` loader for `.cmake-check.yaml`; defaults; discovery by walking up from the target. |
| `src/libcmake-checker-core/src/report/`   | Findings model; human-readable and `--json` renderers; exit-code policy.                     |
| `src/cmake-checker/src/`                  | `cxxopts`-based CLI (`--project`, `--build-dir`, `--config`, `--rules`, `--report`, `--werror`); wires the core library together in `main.cpp`. |
| `src/cmake-checker-test/src/`             | Catch2 v3 unit tests for `cmake-checker-core`.                                               |

**Exit-code policy:** `0` if no `error`-level findings; `1` if any `error`-level
finding; `2` on tool/usage failure. `warn` findings are printed but do not fail
(unless `--werror`).

---

### Component 3: Rule Catalog

Rule IDs are stable and referenced verbatim in `.cmake-check.yaml`.

#### Semantic rules (File API + trace)

| ID   | Rule                    | Definition                                                                                                             |
| ---- | ----------------------- | ---------------------------------------------------------------------------------------------------------------------- |
| R1   | `one_target_per_folder` | Each folder containing a `CMakeLists.txt` defines **at most one** real target (`add_executable`/`add_library`/`add_custom_target`), excluding pure `ALIAS` targets. |
| R2a  | `library_name`          | For **library** targets: `target == folder_name` with a leading `lib` stripped (`libcmake-checker-core` → `cmake-checker-core`). |
| R2b  | `executable_name`       | For **executable** targets (non-test): `target == folder_name` exactly.                                                 |
| R2c  | `test_name`             | For **test** targets: `target == strip_lib(folder_name)` **and** ends with `-test` (`libxe-math-test` → `xe-math-test`). |
| R3   | `target_location`       | Every target is defined under `src/engine` only.                                                                      |
| R4   | `link_keywords`         | Every `target_link_libraries` dependency carries an explicit `PUBLIC`/`PRIVATE`/`INTERFACE` keyword; **one dependency per line**; keyword groups appear in the order `PUBLIC` → `PRIVATE` → `INTERFACE`. |
| R5   | `include_directories`   | Library targets declare `target_include_directories(${target} <scope> <dir>)`.                                          |
| R6   | `known_commands`        | No unknown/misspelled command names (configure-time errors); explicitly flag the `target_link_libraries` singular typo.   |
| R7   | `catch2_test_pattern`   | Test targets link `Catch2::Catch2WithMain` (`PRIVATE`), and use `include(Catch)` + `catch_discover_tests(${target})`.    |

#### Lexical rules (own lexer)

| ID  | Rule                     | Definition                                                                                   |
| --- | ------------------------ | -------------------------------------------------------------------------------------------- |
| L1  | `space_before_paren`     | Command name and opening `(` are adjacent: `set(`, not `set (`.                              |
| L2  | `whitespace`             | Spaces only (no tabs), 4-space indentation, no trailing whitespace.                          |
| L3  | `line_length`            | Maximum 180 columns (aligned with `.clang-format`'s `ColumnLimit: 180`).                     |
| L4  | `quoting`                | Source lists use consistent quoting (all args quoted, or all unquoted) within a single list. |
| L5  | `blank_lines`            | At most one consecutive blank line; single blank line between logical blocks.                |
| L6  | `no_commented_out_code`  | No commented-out CMake code inside argument lists (e.g. `# "src/Foo.cpp"`).                  |

---

### Component 4: Configuration (`.cmake-check.yaml`)

Placed at repo root; discovered by walking up from each checked target, with
`--config` override. Absence of a file ⇒ built-in defaults equal to the
documented rules. Severities: `off` | `warn` | `error`.

```yaml
line_length: 180
indent: 4
rules:
  R1.one_target_per_folder: error
  R2a.library_name:         error
  R2b.executable_name:      error
  R2c.test_name:            error
  R3.target_location:       error
  R4.link_keywords:         error
  R5.include_directories:   error
  R6.known_commands:        error
  R7.catch2_test_pattern:   error
  L1.space_before_paren:    error
  L2.whitespace:            error
  L3.line_length:           warn
  L4.quoting:               warn
  L5.blank_lines:           warn
  L6.no_commented_out_code: warn
exclude:
  - "src/ide/**"
  - "src/pocs/**"
  - "src/cmake-checker/**"
  - "src/libcmake-checker-core/**"
  - "src/cmake-checker-test/**"
  - "build*/**"
```

CLI precedence: command-line flags > `.cmake-check.yaml` > built-in defaults.

---

### Component 5: Mise Wiring

Mirrors the `install:tidy` / `configure:tidy` / `tidy` task family.

##### Tool bootstrap — `install:cmake-check:{release,debug}`

Builds the checker component in-tree, per the tidy pattern:

```bash
cd "$REPO_ROOT/src/cmake-checker"
conan install . --build=missing -s build_type="$CONFIG" \
    -pr:h "$PROFILE" -pr:b "$PROFILE" \
    -of "build-cmake-check/$CONFIG" \
    -c "tools.cmake.cmaketoolchain:user_presets="
```

The binary lands at a deterministic path:
`src/cmake-checker/build-cmake-check/<cfg>/bin/cmake-checker`.

##### Trace configure — `configure:cmake-check:{release,debug}`

For `src/engine`: create the File API query, then configure with
`--trace-format=json-v1 --trace-redirect=` into `src/engine/build-cmake-check/<cfg>/`
(analogous to `configure-tidy.sh`).

##### Check — `cmake-check:{release,debug}` / `cmake-check`

Invoke
`src/cmake-checker/build-cmake-check/<cfg>/bin/cmake-checker --build-dir src/engine/build-cmake-check/<cfg>`.

##### New files

```
mise/setup-cmake-check.sh     mise/setup-cmake-check.ps1
mise/configure-cmake-check.sh mise/configure-cmake-check.ps1
mise/cmake-check.sh           mise/cmake-check.ps1
```

##### `mise.toml` task graph

```
install:cmake-check:release ─> configure:cmake-check:release ─> cmake-check:release
install:cmake-check:debug   ─> configure:cmake-check:debug   ─> cmake-check:debug
                                                                  └─> cmake-check  (both)
```

No `export-recipes` change is required: the checker is built in-tree from its
own `conanfile.py`, not exported/deployed as a standalone recipe.

---

### Component 6: Tests (`src/cmake-checker-test/`)

Catch2 v3 unit tests covering `cmake-checker-core`:

- **Lexer** — tokenization fixtures (quoted/unquoted/bracket args, comments,
  multi-line commands, escapes).
- **Parsers** — golden JSON fixtures for trace json-v1 and File API codemodel /
  cmakeFiles.
- **Rule engine** — one pass/fail pair per rule ID (R1–R7, L1–L6).
- **Config** — YAML loading, severity overrides, discovery/`--config`, defaults.
- **Reporting & exit codes** — text/JSON output; `error` vs `warn` exit policy.

---

### Component 7: Documentation

- **`docs/CMAKE.md`** — reconciliation tracked by the owner (review item 6).
  The confirmed rules (R2 type-dependent naming, L1 no-space `set(`, R4 explicit
  link keywords with `PUBLIC` → `PRIVATE` → `INTERFACE` ordering) remain the
  encoding target and are cross-linked from this plan.
- **`AGENTS.md`** — add a `cmake-check` step to the implementation checklist
  (after Ensure Dependencies / before or after static analysis).
- **`docs/plans/CMAKE_STYLE_CHECKER_PLAN.md`** — this document.

---

## Adoption Strategy

The existing codebase will not pass the new rules immediately (tabs in
`libxe-core/CMakeLists.txt`, missing link scopes, `set (` spacing, misaligned
lists, etc.). Because the tool is checker-only, adoption must be deliberate:

1. **Phase A — land tooling in `warn` mode.** All rules default to `warn`;
   `cmake-check` reports without failing. Measure the violation surface in
   `src/engine`.
2. **Phase B — bring `src/engine` into compliance** (manual/scripted fixes,
   rule by rule), then flip rules to `error` in `.cmake-check.yaml`.
3. **Optional baseline.** If full compliance is deferred, add a
   `--baseline <file>` mode that records accepted findings and only fails on
   new ones. *(Recommended follow-up; not required for v1.)*

---

## Verification Plan

### Automated

```bash
# 1. Build the checker tool (Release)
mise run install:cmake-check:release

# 2. Configure trace + File API for src/engine
mise run configure:cmake-check:release

# 3. Run the checker
mise run cmake-check:release
```

### Tool self-tests

```bash
# Build + run the tool's own Catch2 suite
cd src/cmake-checker
conan install . --build=missing -o with_tests=True -s build_type=Release
conan build . -o with_tests=True -s build_type=Release
```

### Manual verification

- Confirm `src/cmake-checker/build-cmake-check/Release/bin/cmake-checker --help`
  documents the CLI.
- Confirm the checker reports a known-bad fixture and passes a known-good fixture.
- Confirm zero-warning compilation of the tool (`-Wall -Wextra -Werror`).
- Confirm `mise run cmake-check` exits non-zero only on `error`-level findings.

---

## Risks & Open Items

| # | Item                                                                                   | Disposition                                   |
| - | -------------------------------------------------------------------------------------- | --------------------------------------------- |
| 1 | `rapidyaml` exact Conan package name/version and CMake target.                         | **Resolved** — ConanCenter `rapidyaml/0.7.2`; target `ryml::ryml`. |
| 2 | Trace only covers commands that **execute** (skips `if(FALSE)`/disabled branches).      | Acceptable; lexical pass covers all listfiles.|
| 3 | BOM/multi-config generator paths for the binary (MSVC `bin/Debug/`).                    | Handle in `setup-cmake-check.ps1` (flatten runtime output to `bin/`). |
| 4 | Semantic layer requires an install+configured build tree.                               | `cmake-check` depends on `configure:cmake-check` (as `tidy` depends on configure). |
| 5 | Large pre-existing violation surface in `src/engine`.                                   | Phased adoption + optional baseline (above).  |

---

## Out of Scope (v1)

- Auto-formatting / in-place rewrites.
- Inline suppression comments (e.g. `# cmake-check: disable=R4`) — candidate for v2.
- User-defined custom rules / DSL — not planned.
- Checking `src/ide` and `src/pocs` (review item 1 — scope is `src/engine` only)
  and the checker's own folders (`src/cmake-checker`, `src/libcmake-checker-core`,
  `src/cmake-checker-test`); excluded by config.