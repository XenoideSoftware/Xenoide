#!/bin/bash
set -e

. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"

if [ -z "$1" ] || [ -z "$2" ]; then
    echo "Usage: $0 <Debug|Release> <engine|ide|pocs|all|dir>"
    exit 1
fi

CONFIG="$1"
PROJECT="$2"

for folder in $(resolve_projects "$PROJECT"); do
    echo ""
    echo "=== [$folder] Configuring tidy analysis ($CONFIG) ==="
    build_dir="$REPO_ROOT/$folder/build-tidy/$CONFIG"

    toolchain=$(find "$build_dir" -name conan_toolchain.cmake -print -quit 2>/dev/null)
    if [ -z "$toolchain" ]; then
        echo "Error: conan_toolchain.cmake not found under $build_dir. Run setup-tidy first."
        exit 1
    fi

    # The subprojects resolve `CMAKE_MODULE_PATH` entries such as "../cmake"
    # against the working directory, so CMake must run from the subproject.
    (
        cd "$REPO_ROOT/$folder"
        cmake -B "$build_dir" \
            -DCMAKE_TOOLCHAIN_FILE="$toolchain" \
            -DCMAKE_BUILD_TYPE="$CONFIG" \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    )
done
