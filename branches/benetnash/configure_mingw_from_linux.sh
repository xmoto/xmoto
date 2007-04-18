#!/bin/bash

export CXX="i586-mingw32msvc-g++"
export CC="i586-mingw32msvc-gcc"

./configure --target=i586-pc-mingw32 \
            --host=i586-pc-linux-gnu \
            --with-sdl-prefix=/usr/i586-mingw32msvc $*