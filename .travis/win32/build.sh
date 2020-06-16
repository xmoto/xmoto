#!/bin/sh

set -ex

export PATH="/C/tools/msys64/mingw32/bin:/C/tools/msys64/usr/bin:$PATH"
export MSYS2_ARCH=i686
export MSYSTEM=MINGW32

mkdir build && cd build
cmake -G 'MSYS Makefiles' ..

make -j"$(nproc)"

i686-w64-mingw32.shared-cpack -G NSIS
i686-w64-mingw32.shared-cpack -G ZIP

