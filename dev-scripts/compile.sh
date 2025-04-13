#!/bin/sh
# Run this from the root directory. Will compile and install a debug build.

rm -r /tmp/xm
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/tmp/xm ..
THREAD_COUNT="$(( $(nproc) - 2 > 0 ? $(nproc) - 2 : 1 ))"
make -j$THREAD_COUNT
make install
