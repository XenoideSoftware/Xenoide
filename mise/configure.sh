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
    echo "=== [$folder] Configuring CMake ($CONFIG) ==="
    (
        cd "$REPO_ROOT/$folder"
        cmake --preset "$PRESET"
    )
done
