#!/bin/bash

old_env="$(env | sort)"

export MXE_TARGET=i686-w64-mingw32.shared

export MXE_PATH=/usr/lib/mxe
export MXE_CROSS_ROOT="$MXE_PATH/usr"

export MXE_HOST_BIN="$MXE_PATH/usr/x86_64-pc-linux-gnu/bin"
export MXE_CROSS_BIN="$MXE_CROSS_ROOT/$MXE_TARGET/bin"

export WINEPATH="$MXE_PATH/usr/$MXE_TARGET/bin${WINEPATH:+:${WINEPATH}}"
export WINEARCH=win32

export CMAKE_BINARY="${MXE_TARGET}-cmake"
export CPACK_BINARY="${MXE_TARGET}-cpack"

# export all the new variables in the environment
comm -2 -3 <(env | sort) <(echo "$old_env") \
  | while IFS= read -r line; do
  echo "$line" >>"$GITHUB_ENV"
done

echo "$MXE_CROSS_ROOT/bin" >>"$GITHUB_PATH"
