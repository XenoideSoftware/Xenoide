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
    echo "=== [$folder] Installing tidy Conan dependencies ($CONFIG) ==="
    (
        cd "$REPO_ROOT/$folder"
        # The tidy tree does not use presets; skip the root CMakeUserPresets.json
        # update so duplicate preset names do not break `cmake --preset`.
        conan install . --build=missing -s build_type="$CONFIG" \
            -pr:h "$PROFILE" -pr:b "$PROFILE" \
            -of "build-tidy/$CONFIG" \
            -c "tools.cmake.cmaketoolchain:user_presets="
    )
done
