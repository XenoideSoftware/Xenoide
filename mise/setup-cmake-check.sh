#!/bin/bash
set -e

. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"

if [ -z "$1" ]; then
    echo "Usage: $0 <Debug|Release>"
    exit 1
fi

CONFIG="$1"
PROFILE="$(conan_profile)"

echo ""
echo "=== [src/cmake-checker] Building cmake-checker ($CONFIG) ==="
(
    cd "$REPO_ROOT/src/cmake-checker"
    conan install . --build=missing -s build_type="$CONFIG" \
        -pr:h "$PROFILE" -pr:b "$PROFILE" \
        -of "build-cmake-check/$CONFIG" \
        -c "tools.cmake.cmaketoolchain:user_presets="
    cmake -S . -B "build-cmake-check/$CONFIG" \
        -DCMAKE_TOOLCHAIN_FILE="build-cmake-check/$CONFIG/conan_toolchain.cmake" \
        -DCMAKE_BUILD_TYPE="$CONFIG"
    cmake --build "build-cmake-check/$CONFIG"
)

echo ""
echo "=== [src/engine] Installing CMake checker dependencies ($CONFIG) ==="
(
    cd "$REPO_ROOT/src/engine"
    conan install . --build=missing -s build_type="$CONFIG" \
        -pr:h "$PROFILE" -pr:b "$PROFILE" \
        -of "build-cmake-check/$CONFIG" \
        -c "tools.cmake.cmaketoolchain:user_presets="
)