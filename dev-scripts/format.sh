#!/bin/sh
# Requires clang-tools
find src/ -type f \( -name "*.cpp" -o -name "*.h" \) -print0 | xargs -0 clang-format -i
