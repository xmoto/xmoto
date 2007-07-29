#!/bin/bash

export CXX="i586-mingw32msvc-g++"
export CC="i586-mingw32msvc-gcc"

./configure --host=i586-mingw32 \
            --build=i686-linux  \
            --with-sdl-prefix=/usr/i586-mingw32msvc $*


