#!/bin/bash
set -e

. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"

if [ -z "$1" ] || [ -z "$2" ]; then
    echo "Usage: $0 <Debug|Release> <engine|ide|pocs|all|dir>"
    exit 1
fi

CONFIG="$1"
PROJECT="$2"
PROFILE="$(conan_profile)"

for folder in $(resolve_projects "$PROJECT"); do
    echo ""
    echo "=== [$folder] Installing Conan dependencies ($CONFIG) ==="
    (
        cd "$REPO_ROOT/$folder"
        conan install . --build=missing -s build_type="$CONFIG" \
            -pr:h "$PROFILE" -pr:b "$PROFILE"
    )
done
