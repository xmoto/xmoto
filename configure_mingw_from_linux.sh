#!/bin/bash

export CXX="i586-mingw32msvc-g++"
export CC="i586-mingw32msvc-gcc"

CONFIGURE_PATH="`dirname "$0"`""/configure"

"$CONFIGURE_PATH" --host=i586-mingw32 \
                  --build=i686-linux  \
                  --with-sdl-prefix=/usr/i586-mingw32msvc $*
