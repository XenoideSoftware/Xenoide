#!/bin/bash
set -e

. "$(dirname "${BASH_SOURCE[0]}")/lib/common.sh"

if [ -z "$1" ]; then
    echo "Usage: $0 <Debug|Release>"
    exit 1
fi

CONFIG="$1"
build_dir="$REPO_ROOT/src/engine/build-cmake-check/$CONFIG"

toolchain=$(find "$build_dir" -name conan_toolchain.cmake -print -quit 2>/dev/null)
if [ -z "$toolchain" ]; then
    echo "Error: conan_toolchain.cmake not found under $build_dir. Run install:cmake-check first."
    exit 1
fi

query_dir="$build_dir/.cmake/api/v1/query/client-xe-cmake-check"
mkdir -p "$query_dir"
cat > "$query_dir/query.json" <<'EOF'
{"requests": [{"kind": "codemodel", "version": 2}, {"kind": "cmakeFiles", "version": 1}]}
EOF

rm -f "$build_dir/trace.json"

# The subprojects resolve `CMAKE_MODULE_PATH` entries such as "../cmake"
# against the working directory, so CMake must run from the subproject.
(
    cd "$REPO_ROOT/src/engine"
    cmake -B "$build_dir" \
        -DCMAKE_TOOLCHAIN_FILE="$toolchain" \
        -DCMAKE_BUILD_TYPE="$CONFIG" \
        --trace-format=json-v1 \
        --trace-redirect="$build_dir/trace.json"
)