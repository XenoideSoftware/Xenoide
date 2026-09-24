#!/bin/bash
set -e

if [ -z "$1" ]; then
    echo "Usage: $0 <Debug|Release>"
    exit 1
fi

CONFIG="$1"
BUILD_DIR="build-tidy/$CONFIG"

if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "Error: compile_commands.json not found in $BUILD_DIR."
    echo "Please run 'mise run configure:tidy:$(echo "$CONFIG" | tr '[:upper:]' '[:lower:]')' first."
    exit 1
fi

echo "=== Running clang-tidy ($CONFIG) ==="

# Build run-clang-tidy arguments
RCT_ARGS="-p $BUILD_DIR -use-color=1"

if [ "${usage_fix:-false}" = "true" ]; then
    RCT_ARGS="$RCT_ARGS -fix"

    if [ "${usage_format:-false}" = "true" ]; then
        RCT_ARGS="$RCT_ARGS -format"
    fi
fi

if [ "${usage_format:-false}" = "true" ] && [ "${usage_fix:-false}" != "true" ]; then
    echo "Warning: --format has no effect without --fix. Ignoring."
fi

if [ -n "${usage_file:-}" ] && [ "${usage_full:-false}" = "true" ]; then
    echo "Error: --file and --full are mutually exclusive."
    exit 1
fi

if [ -n "${usage_file:-}" ]; then
    file_regex=$(printf '%s' "$usage_file" | sed 's/\./\\./g')

    if ! grep -qE "\"file\": \".*${file_regex}\"" "$BUILD_DIR/compile_commands.json"; then
        echo "Error: '$usage_file' not found in compile database ($BUILD_DIR/compile_commands.json)."
        exit 1
    fi

    echo "Running clang-tidy on $usage_file ($CONFIG)..."
    run-clang-tidy $RCT_ARGS "$file_regex"
elif [ "${usage_full:-false}" = "true" ]; then
    echo "Running clang-tidy on all files in compile database ($CONFIG)..."
    run-clang-tidy $RCT_ARGS
else
    files=$(git diff --name-only HEAD 2>/dev/null | grep -E '\.(cpp|h|hpp|c|cc|cxx)$' | grep '^src/' || true)

    if [ -z "$files" ]; then
        echo "No modified C++ source files found. Nothing to tidy."
        echo "Tip: Use --full to check all files."
        exit 0
    fi

    file_regex=$(echo "$files" | sed 's/\./\\./g' | tr '\n' '|' | sed 's/|$//')

    echo "Running clang-tidy on $(echo "$files" | wc -l | tr -d ' ') modified file(s) ($CONFIG)..."
    run-clang-tidy $RCT_ARGS "$file_regex"
fi