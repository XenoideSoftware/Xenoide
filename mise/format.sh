#!/bin/bash
set -e

find src/ -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.c" -o -name "*.cc" -o -name "*.cxx" \) -exec clang-format -i {} +