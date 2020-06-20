#!/bin/sh

set -ex

# needed for cmake to find some programs
export PATH="/usr/lib/mxe/usr/bin:$PATH"

mkdir build && cd build
mkdir -p /tmp/xmoto

MXE_PATH=/usr/lib/mxe
MXE_TARGET="i686-w64-mingw32.shared"

is_broken_symlink() {
  if [ -L "$1" ] && [ ! -e "$1" ]; then
    return 0
  else
    return 1
  fi
}

relink() {
  for prog in "$@"; do
    if is_broken_symlink "$MXE_HOST_PFX-$prog"; then
      sudo rm "$MXE_HOST_PFX-$prog"
      sudo ln -s "$MXE_CROSS_PFX-$prog" "$MXE_HOST_PFX-$prog"
    fi
  done
}

MXE_HOST_PFX="$MXE_PATH/usr/x86_64-pc-linux-gnu/bin/$MXE_TARGET"
MXE_CROSS_PFX="$MXE_PATH/usr/bin/$MXE_TARGET"

# add the mxe DLLs to wine's path
export WINEPATH="$MXE_PATH/usr/$MXE_TARGET/bin${WINEPATH:+:${WINEPATH}}"
export WINEARCH=win32

echo "MXE_PATH: $MXE_PATH"
echo "MXE HOST PREFIX: $MXE_HOST_PFX"
echo "MXE CROSS PREFIX: $MXE_CROSS_PFX"
echo "WINEPATH: $WINEPATH"

relink "gcc" "g++"

"$MXE_CROSS_PFX-cmake" \
  -DCMAKE_INSTALL_PREFIX=/tmp/xmoto \
  -DCMAKE_BUILD_TYPE=Release \
  -DXMOTO_PACK=manual ..

make -j"$(nproc)"

Xvfb :0 -screen 0 1024x768x16 &
xvfb_pid=$!

sleep 5

echo "pidof Xvfb:"
pidof Xvfb

make xmoto_pack

# not that any reasonable PID 1 responds to SIGTERM anyway ;)
if [ "$xvfb_pid" -gt 1 ]; then
  kill -15 "$xvfb_pid" || true
fi

# aaand if the graceful termination doesn't work..
jobs -p | xargs -r kill -9

#make install

"$MXE_CROSS_PFX"-cpack -G "NSIS"
"$MXE_CROSS_PFX"-cpack -G "ZIP"

fix_pkg_name() {
  if [ -z "$1" ] || [ -z "$2" ]; then
    >&2 echo "Error: [fix_pkg_name]: 2 parameters required"
  fi
  file="$1"
  ext="${file##*.}"
  mv "$file" "${file%.$ext}-$2${ext:+.${ext}}"
}

# add -win32 to .zip files, and -setup to .exe files
for pkg in xmoto-*.zip; do fix_pkg_name "$pkg" "win32"; done
for pkg in xmoto-*.exe; do fix_pkg_name "$pkg" "setup"; done


