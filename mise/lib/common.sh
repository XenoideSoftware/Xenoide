#!/bin/bash
#
# Shared helpers for the mise build scripts.
#
# Dot-source this file from every mise/*.sh script:
#   . "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"
#
# This file lives in mise/lib, so the repository root is two levels above it.

MISE_LIB_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$MISE_LIB_DIR/../.." && pwd)"
MISE_DIR="$REPO_ROOT/mise"

# resolve_projects <selector>
#
# Maps a --project selector to the list of subproject directories (relative to
# the repository root) that a script has to operate on. Accepts either one of
# the canonical project names (engine, ide, pocs), "all" (the default), or an
# explicit directory path.
resolve_projects() {
    case "${1:-all}" in
        engine) echo "src/engine" ;;
        ide)    echo "src/ide" ;;
        pocs)   echo "src/pocs" ;;
        all)    echo "src/engine src/ide src/pocs" ;;
        *)      echo "$1" ;;
    esac
}

# conan_profile
#
# Prints the absolute path of the Conan profile matching the host OS. The
# profiles live at the repository root, while scripts operate inside a
# subproject, so they can never be referenced with a relative path.
conan_profile() {
    case "$(uname -s 2>/dev/null || echo Windows)" in
        Linux*|Darwin*) echo "$REPO_ROOT/conan/profiles/unix" ;;
        *)              echo "$REPO_ROOT/conan/profiles/windows" ;;
    esac
}
