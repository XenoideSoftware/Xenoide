#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

usage() {
    cat <<'EOF'
Usage: ./mise/mutation.sh [options]

Runs Mull mutation testing against the Xenoide unit test suite.

Options:
  --generate            Discover and generate mutants without running tests (dry run)
  --kill                Execute unit tests against each mutant to kill them (default)
  --config <cfg>        Build type: Debug (default) or Release
  --reporters <list>    Comma-separated report formats: IDE,SQLite,Elements,Patches,Sarif
                        (default: IDE,SQLite,Elements)
  --output-dir <dir>    Directory for report files (default: mutation/mull)
  --report-name <name>  Report database base name (default: xenoide)
  --clean               Delete existing build/mutation artifacts before running
  --target <name>       Run against a specific test target (e.g. xe-core-test)
  --timeout <ms>        Maximum test execution timeout per mutant
  --threshold <score>   Minimum required mutation score (0-100); fails the run when below
  -h, --help            Show this help message
EOF
}

die() {
    echo "Error: $*" >&2
    exit 1
}

GENERATE="false"
KILL="false"
CONFIG="Debug"
REPORTERS="IDE,SQLite,Elements"
OUTPUT_DIR="mutation/mull"
REPORT_NAME="xenoide"
CLEAN="false"
TARGET=""
TIMEOUT=""
THRESHOLD=""

# ---------------------------------------------------------------------------
# CLI option parsing
# ---------------------------------------------------------------------------

require_value() {
    [ -n "${2:-}" ] || die "$1 requires a value"
}

while [ $# -gt 0 ]; do
    case "$1" in
        --generate)
            GENERATE="true"
            shift
            ;;
        --kill)
            KILL="true"
            shift
            ;;
        --config)
            require_value "$1" "${2:-}"
            CONFIG="$2"
            shift 2
            ;;
        --reporters)
            require_value "$1" "${2:-}"
            REPORTERS="$2"
            shift 2
            ;;
        --output-dir)
            require_value "$1" "${2:-}"
            OUTPUT_DIR="$2"
            shift 2
            ;;
        --report-name)
            require_value "$1" "${2:-}"
            REPORT_NAME="$2"
            shift 2
            ;;
        --clean)
            CLEAN="true"
            shift
            ;;
        --target)
            require_value "$1" "${2:-}"
            TARGET="$2"
            shift 2
            ;;
        --timeout)
            require_value "$1" "${2:-}"
            TIMEOUT="$2"
            shift 2
            ;;
        --threshold)
            require_value "$1" "${2:-}"
            THRESHOLD="$2"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Error: unknown option '$1'" >&2
            usage >&2
            exit 1
            ;;
    esac
done

# mise exposes user-provided usage flags as environment variables; let them
# override the arguments parsed from the command line.
[ "${usage_generate:-}" = "true" ] && GENERATE="true"
[ "${usage_kill:-}" = "true" ] && KILL="true"
[ -n "${usage_config:-}" ] && CONFIG="$usage_config"
[ -n "${usage_reporters:-}" ] && REPORTERS="$usage_reporters"
[ -n "${usage_output_dir:-}" ] && OUTPUT_DIR="$usage_output_dir"
[ -n "${usage_report_name:-}" ] && REPORT_NAME="$usage_report_name"
[ -n "${usage_clean:-}" ] && CLEAN="$usage_clean"
[ -n "${usage_target:-}" ] && TARGET="$usage_target"
[ -n "${usage_timeout:-}" ] && TIMEOUT="$usage_timeout"
[ -n "${usage_threshold:-}" ] && THRESHOLD="$usage_threshold"

# ---------------------------------------------------------------------------
# Argument validation
# ---------------------------------------------------------------------------

[ "$GENERATE" = "true" ] && [ "$KILL" = "true" ] && \
    die "--generate and --kill are mutually exclusive"

CONFIG="$(printf '%s' "$CONFIG" | tr '[:upper:]' '[:lower:]')"
case "$CONFIG" in
    debug) CONFIG="Debug" ;;
    release) CONFIG="Release" ;;
    *) die "invalid --config '$CONFIG' (expected Debug or Release)" ;;
esac

case "$CLEAN" in
    true|1|yes|on) CLEAN="true" ;;
    *) CLEAN="false" ;;
esac

[ -n "$OUTPUT_DIR" ] || die "--output-dir cannot be empty"
[ -n "$REPORT_NAME" ] || die "--report-name cannot be empty"

canon_reporter() {
    case "$(printf '%s' "$1" | tr '[:upper:]' '[:lower:]')" in
        ide) printf 'IDE' ;;
        sqlite) printf 'SQLite' ;;
        elements) printf 'Elements' ;;
        patches) printf 'Patches' ;;
        sarif) printf 'Sarif' ;;
        *) return 1 ;;
    esac
}

declare -a REPORTER_LIST=()
IFS=',' read -r -a raw_reporters <<< "$REPORTERS"
for raw in "${raw_reporters[@]}"; do
    item="$(printf '%s' "$raw" | sed 's/^[[:space:]]*//; s/[[:space:]]*$//')"
    [ -n "$item" ] || continue
    canonical="$(canon_reporter "$item")" || \
        die "invalid reporter '$item' (expected IDE, SQLite, Elements, Patches or Sarif)"
    REPORTER_LIST+=("$canonical")
done
[ ${#REPORTER_LIST[@]} -gt 0 ] || die "--reporters cannot be empty"

if [ -n "$TIMEOUT" ] && ! [[ "$TIMEOUT" =~ ^[1-9][0-9]*$ ]]; then
    die "invalid --timeout '$TIMEOUT' (expected a positive number of milliseconds)"
fi

if [ -n "$THRESHOLD" ]; then
    if ! [[ "$THRESHOLD" =~ ^[0-9]+(\.[0-9]+)?$ ]] || \
        [ "$(printf '%s' "$THRESHOLD" | awk '{print ($1 > 100) ? 1 : 0}')" = "1" ]; then
        die "invalid --threshold '$THRESHOLD' (expected a number between 0 and 100)"
    fi
fi

# ---------------------------------------------------------------------------
# Required tools
# ---------------------------------------------------------------------------

for bin in cmake ctest conan python3; do
    command -v "$bin" >/dev/null 2>&1 || die "required binary '$bin' not found"
done

# ---------------------------------------------------------------------------
# Mull / Clang version pairing
#
# LLVM pass plugins are bound to an exact LLVM major version, so the
# mull-runner-N, mull-reporter-N, clang++-N and mull-ir-frontend-N tools must
# all come from the same version. Pick the newest installed mull-runner-N and
# fail fast when any companion tool is missing.
# ---------------------------------------------------------------------------

MULL_VERSION=""
shopt -s nullglob
mull_runners=()
for dir in ${PATH//:/ }; do
    for candidate in "$dir"/mull-runner-*; do
        mull_runners+=("$candidate")
    done
done
shopt -u nullglob

if [ ${#mull_runners[@]} -gt 0 ]; then
    MULL_VERSION="$(
        for runner in "${mull_runners[@]}"; do
            version="${runner##*-}"
            case "$version" in
                ''|*[!0-9]*) continue ;;
            esac
            printf '%s\n' "$version"
        done | sort -rn | head -n 1
    )"
fi

[ -n "$MULL_VERSION" ] || \
    die "no versioned mull-runner found in PATH (expected mull-runner-<LLVM version>, e.g. mull-runner-19)"

MULL_RUNNER="mull-runner-$MULL_VERSION"
MULL_REPORTER="mull-reporter-$MULL_VERSION"
CC_BIN="clang-$MULL_VERSION"
CXX_BIN="clang++-$MULL_VERSION"

for bin in "$MULL_RUNNER" "$MULL_REPORTER" "$CC_BIN" "$CXX_BIN"; do
    command -v "$bin" >/dev/null 2>&1 || \
        die "version mismatch: found $MULL_RUNNER but '$bin' is not available (install matching Mull and Clang $MULL_VERSION packages)"
done

MULL_PLUGIN=""
for candidate in \
    "/usr/lib/mull-ir-frontend-$MULL_VERSION" \
    "/usr/local/lib/mull-ir-frontend-$MULL_VERSION" \
    "/usr/lib64/mull-ir-frontend-$MULL_VERSION"; do
    if [ -f "$candidate" ]; then
        MULL_PLUGIN="$candidate"
        break
    fi
done
if [ -z "$MULL_PLUGIN" ]; then
    MULL_PLUGIN="$(find /usr/lib /usr/local/lib -maxdepth 3 \
        -name "mull-ir-frontend-$MULL_VERSION" -type f 2>/dev/null | head -n 1 || true)"
fi
[ -n "$MULL_PLUGIN" ] || \
    die "mull-ir-frontend-$MULL_VERSION not found (expected under /usr/lib)"

echo "=== Mull toolchain ==="
echo "  mull-runner : $MULL_RUNNER"
echo "  mull-reporter: $MULL_REPORTER"
echo "  compiler     : $CXX_BIN / $CC_BIN"
echo "  ir frontend  : $MULL_PLUGIN"
echo "  mode         : $( [ "$GENERATE" = "true" ] && echo 'generate (dry run)' || echo 'kill' )"
echo "  config       : $CONFIG"

# ---------------------------------------------------------------------------
# Optional clean-up
# ---------------------------------------------------------------------------

if [ "$CLEAN" = "true" ]; then
    echo ""
    echo "=== Cleaning build and mutation artifacts ==="
    rm -rf build-mutation-mull "$OUTPUT_DIR"
fi

mkdir -p "$OUTPUT_DIR"

case "$(uname -s 2>/dev/null || echo Windows)" in
    Linux*|Darwin*) PROFILE="conan/profiles/unix" ;;
    *) PROFILE="conan/profiles/windows" ;;
esac

BUILD_DIR="build-mutation-mull/$CONFIG"

# ---------------------------------------------------------------------------
# 1. Configure and build the instrumented test targets in an isolated tree.
# ---------------------------------------------------------------------------

echo ""
echo "=== [$CONFIG] Installing Conan dependencies ==="
# Disable the root CMakeUserPresets.json update: this isolated tree does not
# use presets, and accumulating include entries would break `cmake --preset`
# with duplicate preset names.
conan install . --build=missing -s build_type="$CONFIG" \
    -pr:h "$PROFILE" -pr:b "$PROFILE" -of "$BUILD_DIR" \
    -c "tools.cmake.cmaketoolchain:user_presets="

TOOLCHAIN="$(find "$BUILD_DIR" -name conan_toolchain.cmake -print -quit 2>/dev/null || true)"
if [ -z "$TOOLCHAIN" ]; then
    die "conan_toolchain.cmake not found under $BUILD_DIR"
fi

echo "=== [$CONFIG] Configuring CMake with Mull instrumentation ==="
MULL_FLAGS="-fpass-plugin=$MULL_PLUGIN -O0 -g -grecord-command-line"
CMAKE_ARGS=(
    -B "$BUILD_DIR"
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN"
    -DCMAKE_BUILD_TYPE="$CONFIG"
    -DCMAKE_C_COMPILER="$CC_BIN"
    -DCMAKE_CXX_COMPILER="$CXX_BIN"
)
if [ "$CONFIG" = "Release" ]; then
    # Keep Release at -O0 so Mull generates reliable mutants.
    CMAKE_ARGS+=(
        -DCMAKE_C_FLAGS_RELEASE="-O0 -g -DNDEBUG"
        -DCMAKE_CXX_FLAGS_RELEASE="-O0 -g -DNDEBUG"
    )
fi
CFLAGS="$MULL_FLAGS" CXXFLAGS="$MULL_FLAGS" cmake "${CMAKE_ARGS[@]}"

# CFLAGS/CXXFLAGS only seed the cache on the first configure of a tree; catch
# build trees that were previously configured without Mull instrumentation.
if ! grep -q "fpass-plugin" "$BUILD_DIR/CMakeCache.txt" 2>/dev/null; then
    die "build tree at $BUILD_DIR was not configured with Mull flags; re-run with --clean"
fi

# Extract buildable test target names from ctest. catch_discover_tests()
# registers "<target>_NOT_BUILT-<hash>" placeholders until the binary exists.
TEST_TARGETS="$(ctest --test-dir "$BUILD_DIR" --show-only=json-v1 2>/dev/null | python3 -c '
import json, sys, re
try:
    data = json.load(sys.stdin)
except Exception:
    sys.exit(0)
targets = set()
for t in data.get("tests", []):
    command = t.get("command")
    name = t.get("name", "")
    raw = command[0].rsplit("/", 1)[-1] if command else name
    match = re.match(r"^(.*?)_NOT_BUILT", raw) or re.match(r"^(.*?)_NOT_BUILT", name)
    if match:
        targets.add(match.group(1))
    elif command:
        targets.add(raw)
    elif name:
        targets.add(name)
print(" ".join(sorted(x for x in targets if x)))
')"

if [ -n "$TARGET" ] && [ -n "$TEST_TARGETS" ]; then
    filtered_targets=""
    for name in $TEST_TARGETS; do
        [ "$name" = "$TARGET" ] && filtered_targets="$TARGET"
    done
    if [ -n "$filtered_targets" ]; then
        TEST_TARGETS="$filtered_targets"
    else
        # Target may not be registered yet (e.g. PRE_BUILD placeholders are
        # missing on a fresh tree); let CMake decide whether it exists.
        TEST_TARGETS="$TARGET"
    fi
fi

echo "=== [$CONFIG] Building test targets ==="
if [ -n "$TEST_TARGETS" ]; then
    # shellcheck disable=SC2086
    cmake --build "$BUILD_DIR" --parallel --target $TEST_TARGETS
else
    echo "Warning: no test targets discovered, building the full tree"
    cmake --build "$BUILD_DIR" --parallel
fi

# ---------------------------------------------------------------------------
# 2. Discover the built test executables.
# ---------------------------------------------------------------------------

mapfile -t TEST_EXECUTABLES < <(ctest --test-dir "$BUILD_DIR" --show-only=json-v1 2>/dev/null | python3 -c '
import json, sys
try:
    data = json.load(sys.stdin)
except Exception:
    sys.exit(0)
seen = set()
for t in data.get("tests", []):
    command = t.get("command")
    if not command:
        continue
    exe = command[0]
    if "_NOT_BUILT" in exe:
        continue
    if exe not in seen:
        seen.add(exe)
        print(exe)
')

declare -a RUN_EXECUTABLES=()
for exe in "${TEST_EXECUTABLES[@]:-}"; do
    [ -n "$exe" ] || continue
    if [ -n "$TARGET" ]; then
        base="$(basename "$exe")"
        base="${base%.exe}"
        [ "$base" = "$TARGET" ] || continue
    fi
    if [ -f "$exe" ]; then
        RUN_EXECUTABLES+=("$exe")
    fi
done

if [ ${#RUN_EXECUTABLES[@]} -eq 0 ]; then
    if [ -n "$TARGET" ]; then
        available="$(printf '%s\n' "${TEST_EXECUTABLES[@]:-}" | sed 's|.*/||' | sort -u | tr '\n' ' ')"
        die "test target '$TARGET' not found (available: ${available:-none})"
    fi
    die "no test executables discovered under $BUILD_DIR"
fi

# ---------------------------------------------------------------------------
# 3. Execute (or dry-run) every target, accumulating results in SQLite.
# ---------------------------------------------------------------------------

DB_PATH="$OUTPUT_DIR/$REPORT_NAME.sqlite"
rm -f "$DB_PATH"

RUNNER_ARGS=(
    --reporters SQLite
    --report-dir "$OUTPUT_DIR"
    --report-name "$REPORT_NAME"
    --allow-surviving
)
[ "$GENERATE" = "true" ] && RUNNER_ARGS+=(--dry-run)
[ -n "$TIMEOUT" ] && RUNNER_ARGS+=(--timeout "$TIMEOUT")

echo ""
echo "=== Running mull-runner on ${#RUN_EXECUTABLES[@]} test executable(s) ==="
for exe in "${RUN_EXECUTABLES[@]}"; do
    echo ""
    echo "--- $(basename "$exe") ---"
    "$MULL_RUNNER" "${RUNNER_ARGS[@]}" "$exe"
done

[ -f "$DB_PATH" ] || die "expected SQLite report at $DB_PATH was not created"

# ---------------------------------------------------------------------------
# 4. Consolidate the aggregated database with mull-reporter.
# ---------------------------------------------------------------------------

declare -a REPORTER_CLI_ARGS=()
for reporter in "${REPORTER_LIST[@]}"; do
    if [ "$reporter" = "SQLite" ]; then
        echo "Note: SQLite database already written by mull-runner at $DB_PATH"
        continue
    fi
    REPORTER_CLI_ARGS+=(--reporters "$reporter")
done
# Always gate the run through mull-reporter, even when only SQLite was asked.
[ ${#REPORTER_CLI_ARGS[@]} -gt 0 ] || REPORTER_CLI_ARGS=(--reporters IDE)

REPORTER_ARGS=(
    --report-dir "$OUTPUT_DIR"
    --report-name "$REPORT_NAME"
)
if [ -n "$THRESHOLD" ]; then
    REPORTER_ARGS+=(--mutation-score-threshold "$THRESHOLD")
else
    REPORTER_ARGS+=(--allow-surviving)
fi

echo ""
echo "=== Generating consolidated reports ==="
"$MULL_REPORTER" "${REPORTER_ARGS[@]}" "${REPORTER_CLI_ARGS[@]}" "$DB_PATH"

echo ""
echo "=== Mutation testing artifacts ==="
echo "  database : $DB_PATH"
echo "  reports  : $OUTPUT_DIR (report name: $REPORT_NAME)"
