#!/bin/sh
# Run this from the root directory. Will compile and install a debug build.
# Requires ninja.

rm -r /tmp/xm
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_INSTALL_PREFIX=/tmp/xm -G Ninja
THREAD_COUNT="$(( $(nproc) - 2 > 0 ? $(nproc) - 2 : 1 ))"
CMAKE_BUILD_PARALLEL_LEVEL="$THREAD_COUNT" cmake --build build --target all
cmake --build build --target install
