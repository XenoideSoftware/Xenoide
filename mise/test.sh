#!/bin/bash
set -e

. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"

if [ -z "$1" ] || [ -z "$2" ]; then
    echo "Usage: $0 <Debug|Release> <engine|ide|pocs|all|dir>"
    exit 1
fi

CONFIG="$1"
PROJECT="$2"
PRESET="conan-$(echo "$CONFIG" | tr '[:upper:]' '[:lower:]')"

for folder in $(resolve_projects "$PROJECT"); do
    echo ""
    echo "=== [$folder] Running tests ($CONFIG) ==="
    (
        cd "$REPO_ROOT/$folder"
        if [ -n "$MSYSTEM" ] || [ "$(uname -s 2>/dev/null)" = "Linux" ] || [ "$(uname -s 2>/dev/null)" = "Darwin" ]; then
            ctest --preset "$PRESET" --output-on-failure
        else
            ctest --preset "$PRESET" -C "$CONFIG" --output-on-failure
        fi
    )
done
