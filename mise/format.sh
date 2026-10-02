#!/bin/bash
set -e

. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"

PROJECT="${1:-all}"

for folder in $(resolve_projects "$PROJECT"); do
    if [ ! -d "$REPO_ROOT/$folder/src" ]; then
        echo "=== [$folder] No src directory found, skipping ==="
        continue
    fi
    echo "=== [$folder] Formatting C/C++ sources ==="
    find "$REPO_ROOT/$folder/src" -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.c" -o -name "*.cc" -o -name "*.cxx" \) -exec clang-format -i {} +
done
