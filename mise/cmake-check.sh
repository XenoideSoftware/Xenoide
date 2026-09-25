#!/bin/bash
set -e

. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"

if [ -z "$1" ]; then
    echo "Usage: $0 <Debug|Release>"
    exit 1
fi

CONFIG="$1"
LOWER="$(echo "$CONFIG" | tr '[:upper:]' '[:lower:]')"
BIN="$REPO_ROOT/src/cmake-checker/build-cmake-check/$CONFIG/bin/cmake-checker"

if [ ! -x "$BIN" ]; then
    echo "Error: cmake-checker binary not found at $BIN."
    echo "Run 'mise run install:cmake-check:$LOWER' first."
    exit 1
fi

"$BIN" --project "$REPO_ROOT/src/engine" --project-build-dir "$REPO_ROOT/src/engine/build-cmake-check/$CONFIG"