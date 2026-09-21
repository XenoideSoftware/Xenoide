#!/bin/bash
set -e

if [ -z "$1" ]; then
    echo "Usage: $0 <Debug|Release>"
    exit 1
fi

CONFIG="$1"
PRESET="conan-$(echo "$CONFIG" | tr '[:upper:]' '[:lower:]')"

if [ -n "$MSYSTEM" ] || [ "$(uname -s 2>/dev/null)" = "Linux" ] || [ "$(uname -s 2>/dev/null)" = "Darwin" ]; then
    ctest --preset "$PRESET" --output-on-failure
else
    ctest --preset "$PRESET" -C "$CONFIG" --output-on-failure
fi
