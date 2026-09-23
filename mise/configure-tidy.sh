#!/bin/bash
set -e

if [ -z "$1" ]; then
    echo "Usage: $0 <Debug|Release>"
    exit 1
fi

CONFIG="$1"

toolchain=$(find "build-tidy/$CONFIG" -name conan_toolchain.cmake -print -quit 2>/dev/null)
if [ -z "$toolchain" ]; then
    echo "Error: conan_toolchain.cmake not found under build-tidy/$CONFIG. Run setup-tidy first."
    exit 1
fi

cmake -B "build-tidy/$CONFIG" \
    -DCMAKE_TOOLCHAIN_FILE="$toolchain" \
    -DCMAKE_BUILD_TYPE="$CONFIG" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON