#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

TOOL="${1:-}"
if [ -z "$TOOL" ] || { [ "$TOOL" != "gcov" ] && [ "$TOOL" != "llvm-cov" ]; }; then
    echo "Usage: $0 <gcov|llvm-cov> [--config Debug|Release|all] [--export text|html|csv] [--check [threshold]] [--output-dir DIR] [--clean]"
    exit 1
fi
shift

CONFIG="all"
EXPORT="text"
CHECK=""
OUTPUT_DIR=""
CLEAN="false"

while [ $# -gt 0 ]; do
    case "$1" in
        --config|-c)
            CONFIG="${2:-}"
            [ -n "$CONFIG" ] || { echo "Error: --config requires a value"; exit 1; }
            shift 2
            ;;
        --export|-e)
            EXPORT="${2:-}"
            [ -n "$EXPORT" ] || { echo "Error: --export requires a value"; exit 1; }
            shift 2
            ;;
        --check)
            if [ $# -gt 1 ] && [[ "${2:-}" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
                CHECK="$2"
                shift 2
            else
                CHECK="90"
                shift
            fi
            ;;
        --output-dir)
            OUTPUT_DIR="${2:-}"
            [ -n "$OUTPUT_DIR" ] || { echo "Error: --output-dir requires a value"; exit 1; }
            shift 2
            ;;
        --clean)
            CLEAN="true"
            shift
            ;;
        *)
            echo "Error: unknown option '$1'"
            exit 1
            ;;
    esac
done

# mise exposes user-provided usage flags as environment variables; let them
# override the arguments baked into the mise task definitions.
[ -n "${usage_config:-}" ] && CONFIG="$usage_config"
[ -n "${usage_export:-}" ] && EXPORT="$usage_export"
[ -n "${usage_check:-}" ] && CHECK="$usage_check"
[ -n "${usage_output_dir:-}" ] && OUTPUT_DIR="$usage_output_dir"
[ -n "${usage_clean:-}" ] && CLEAN="$usage_clean"

CONFIG="$(printf '%s' "$CONFIG" | tr '[:upper:]' '[:lower:]')"
EXPORT="$(printf '%s' "$EXPORT" | tr '[:upper:]' '[:lower:]')"

case "$CONFIG" in
    debug|release|all) ;;
    *) echo "Error: invalid --config '$CONFIG' (expected Debug, Release or all)"; exit 1 ;;
esac

case "$EXPORT" in
    text|html|csv) ;;
    *) echo "Error: invalid --export '$EXPORT' (expected text, html or csv)"; exit 1 ;;
esac

if [ -n "$CHECK" ]; then
    [ "$CHECK" = "true" ] && CHECK="90"
    if ! [[ "$CHECK" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
        echo "Error: invalid --check threshold '$CHECK' (expected a number)"
        exit 1
    fi
fi

if [ "$TOOL" = "gcov" ]; then
    REQUIRED_BINS=(gcc g++ gcov cmake ctest conan python3)
else
    REQUIRED_BINS=(clang clang++ llvm-cov llvm-profdata cmake ctest conan python3)
fi
for bin in "${REQUIRED_BINS[@]}"; do
    command -v "$bin" >/dev/null 2>&1 || { echo "Error: required binary '$bin' not found"; exit 1; }
done

case "$(uname -s 2>/dev/null || echo Windows)" in
    Linux*|Darwin*) PROFILE="conan/profiles/unix" ;;
    *) PROFILE="conan/profiles/windows" ;;
esac

if [ "$CONFIG" = "all" ]; then
    CONFIGS=(Debug Release)
else
    CONFIGS=("$(printf '%s' "$CONFIG" | sed 's/^d/D/; s/^r/R/')")
fi

if [ -z "$OUTPUT_DIR" ]; then
    OUTPUT_DIR="coverage/$TOOL"
    if [ "$CONFIG" != "all" ]; then
        OUTPUT_DIR="coverage/$TOOL/$CONFIGS"
    fi
fi
mkdir -p "$OUTPUT_DIR"

# ---------------------------------------------------------------------------
# 1. Configure, build and test every selected configuration in its own
#    isolated build tree.
# ---------------------------------------------------------------------------

declare -a LLVM_BINARIES=()

for cfg in "${CONFIGS[@]}"; do
    BUILD_DIR="build/coverage-$TOOL/$cfg"

    echo ""
    echo "=== [$TOOL / $cfg] Installing Conan dependencies ==="
    conan install . --build=missing -s build_type="$cfg" \
        -pr:h "$PROFILE" -pr:b "$PROFILE" -of "$BUILD_DIR"

    TOOLCHAIN="$(find "$BUILD_DIR" -name conan_toolchain.cmake -print -quit 2>/dev/null || true)"
    if [ -z "$TOOLCHAIN" ]; then
        echo "Error: conan_toolchain.cmake not found under $BUILD_DIR"
        exit 1
    fi

    echo "=== [$TOOL / $cfg] Configuring CMake ==="
    CMAKE_ARGS=(
        -B "$BUILD_DIR"
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN"
        -DCMAKE_BUILD_TYPE="$cfg"
        -DXE_ENABLE_COVERAGE=ON
    )
    if [ "$TOOL" = "llvm-cov" ]; then
        CMAKE_ARGS+=(-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++)
    fi
    cmake "${CMAKE_ARGS[@]}"

    TEST_TARGETS="$(ctest --test-dir "$BUILD_DIR" --show-only=json-v1 2>/dev/null | python3 -c '
import json, sys, re
try:
    data = json.load(sys.stdin)
except Exception:
    sys.exit(0)
targets = set()
for t in data.get("tests", []):
    command = t.get("command")
    if command:
        targets.add(command[0].rsplit("/", 1)[-1])
        continue
    match = re.match(r"^(.*?)_NOT_BUILT-", t.get("name", ""))
    if match:
        targets.add(match.group(1))
    else:
        targets.add(t.get("name", ""))
print(" ".join(sorted(t for t in targets if t)))
')"

    echo "=== [$TOOL / $cfg] Building test targets ==="
    if [ -n "$TEST_TARGETS" ]; then
        # shellcheck disable=SC2086
        cmake --build "$BUILD_DIR" --parallel --target $TEST_TARGETS
    else
        echo "Warning: no test targets discovered, building the full tree"
        cmake --build "$BUILD_DIR" --parallel
    fi

    echo "=== [$TOOL / $cfg] Running CTest ==="
    if [ "$CLEAN" = "true" ]; then
        find "$BUILD_DIR" \( -name '*.gcda' -o -name '*.profraw' \) -delete 2>/dev/null || true
    fi

    if [ "$TOOL" = "llvm-cov" ]; then
        PROFILE_DIR="$BUILD_DIR/profiles"
        mkdir -p "$PROFILE_DIR"
        LLVM_PROFILE_FILE="$PROJECT_ROOT/$PROFILE_DIR/%p.profraw" \
            ctest --test-dir "$BUILD_DIR" --output-on-failure
        mapfile -t found_binaries < <(ctest --test-dir "$BUILD_DIR" --show-only=json-v1 2>/dev/null | python3 -c '
import json, sys
try:
    data = json.load(sys.stdin)
except Exception:
    sys.exit(0)
seen = set()
for t in data.get("tests", []):
    command = t.get("command")
    if command:
        exe = command[0]
        if exe not in seen:
            seen.add(exe)
            print(exe)
')
        for exe in "${found_binaries[@]:-}"; do
            [ -n "$exe" ] && LLVM_BINARIES+=("$exe")
        done
    else
        ctest --test-dir "$BUILD_DIR" --output-on-failure
    fi
done

# ---------------------------------------------------------------------------
# 2. Merge / aggregate the coverage data across configurations.
# ---------------------------------------------------------------------------

REPORTER_ARGS=(--tool "$TOOL" --export "$EXPORT" --output-dir "$OUTPUT_DIR")
[ -n "$CHECK" ] && REPORTER_ARGS+=(--check "$CHECK")

if [ "$TOOL" = "llvm-cov" ]; then
    echo ""
    echo "=== Merging LLVM profiles ==="
    PROFRAWS="$(find "build/coverage-$TOOL" -name '*.profraw' 2>/dev/null || true)"
    if [ -z "$PROFRAWS" ]; then
        echo "Error: no .profraw profile files found under build/coverage-$TOOL"
        exit 1
    fi
    MERGED="$OUTPUT_DIR/merged.profdata"
    # shellcheck disable=SC2086
    llvm-profdata merge -o "$MERGED" $PROFRAWS
    echo "Merged profiles into $MERGED"

    if [ ${#LLVM_BINARIES[@]} -eq 0 ]; then
        echo "Error: no instrumented test executables found"
        exit 1
    fi

    EXPORT_DIR="$OUTPUT_DIR/.llvm-exports"
    mkdir -p "$EXPORT_DIR"
    echo "=== Exporting coverage from ${#LLVM_BINARIES[@]} test executable(s) ==="
    INDEX=0
    for exe in "${LLVM_BINARIES[@]}"; do
        OUT_JSON="$EXPORT_DIR/report-$INDEX.json"
        if llvm-cov export -instr-profile="$MERGED" "$exe" > "$OUT_JSON" 2>/dev/null; then
            INDEX=$((INDEX + 1))
        else
            rm -f "$OUT_JSON"
            echo "  (skipped '$(basename "$exe")': no coverage data)"
        fi
    done
    if [ "$INDEX" -eq 0 ]; then
        echo "Error: no llvm-cov export JSON files could be generated"
        exit 1
    fi
    REPORTER_ARGS+=("$EXPORT_DIR"/report-*.json)
else
    echo ""
    echo "=== Aggregating gcov data ==="
    GCOV_DIR="$OUTPUT_DIR/.gcov-json"
    mkdir -p "$GCOV_DIR"
    for cfg in "${CONFIGS[@]}"; do
        CONFIG_DIR="$GCOV_DIR/$cfg"
        mkdir -p "$CONFIG_DIR"
        BUILD_DIR="build/coverage-$TOOL/$cfg"
        GCNO_COUNT=0
        while IFS= read -r -d '' gcno; do
            gcda="${gcno%.gcno}.gcda"
            if [ ! -f "$gcda" ]; then
                continue
            fi
            (cd "$CONFIG_DIR" && gcov -j "$gcno" >/dev/null 2>&1 || true)
            GCNO_COUNT=$((GCNO_COUNT + 1))
        done < <(find "$PROJECT_ROOT/$BUILD_DIR" -name '*.gcno' -print0 2>/dev/null || true)
        echo "  [$cfg] generated gcov data for $GCNO_COUNT executed translation unit(s)"
    done
    REPORTER_ARGS+=(--gcov-dir "$GCOV_DIR")
fi

# ---------------------------------------------------------------------------
# 3. Produce the requested report and enforce the quality gate.
# ---------------------------------------------------------------------------

echo ""
echo "=== Generating $EXPORT report ==="
python3 mise/coverage_reporter.py "${REPORTER_ARGS[@]}"