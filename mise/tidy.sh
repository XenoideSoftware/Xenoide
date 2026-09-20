#!/bin/bash
set -e

compile_db=$(find build build-* -name compile_commands.json -print -quit 2>/dev/null)
if [ -z "$compile_db" ]; then
    echo "Error: compile_commands.json not found in build directories. Please build the project first."
    exit 1
fi

compile_dir=$(dirname "$compile_db")
if [ "$#" -gt 0 ]; then
    clang-tidy -p "$compile_dir" "$@"
else
    files=$(git diff --name-only HEAD 2>/dev/null | grep -E '\.(cpp|h|hpp|c|cc|cxx)$' | grep '^src/' || true)
    if [ -z "$files" ]; then
        files=$(find src/ -type f \( -name "*.cpp" -o -name "*.cc" -o -name "*.cxx" \))
    fi
    if [ -n "$files" ]; then
        clang-tidy -p "$compile_dir" $files
    else
        echo "No C++ files found to tidy."
    fi
fi