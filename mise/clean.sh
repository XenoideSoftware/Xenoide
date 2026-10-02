#!/bin/bash
set -e

. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"

PROJECT="${1:-all}"

# Legacy build trees left at the repository root by the pre-split layout.
rm -rf "$REPO_ROOT/build" "$REPO_ROOT"/build-*

for folder in $(resolve_projects "$PROJECT"); do
    echo "=== [$folder] Removing build directories ==="
    rm -rf "$REPO_ROOT/$folder/build" "$REPO_ROOT/$folder"/build-*
done
