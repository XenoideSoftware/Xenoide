#!/bin/bash
set -e

if [ -z "$1" ]; then
    echo "Usage: $0 <Debug|Release>"
    exit 1
fi

CONFIG="$1"

case "$(uname -s 2>/dev/null || echo Windows)" in
    Linux*|Darwin*) PROFILE="conan/profiles/unix" ;;
    CYGWIN*|MINGW*|MSYS*) PROFILE="conan/profiles/windows" ;;
    *) PROFILE="conan/profiles/windows" ;;
esac

conan install . --build=missing -s build_type="$CONFIG" -pr:h "$PROFILE" -pr:b "$PROFILE"
