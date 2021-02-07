#!/bin/sh

set -ex

mkdir build && cd build
mkdir -p /tmp/xminstall

echo "MXE_PATH: $MXE_PATH"
echo "MXE HOST PREFIX: $MXE_HOST_PFX"
echo "MXE CROSS PREFIX: $MXE_CROSS_PFX"
echo "WINEPATH: $WINEPATH"

"$MXE_TARGET-cmake" -D CMAKE_INSTALL_PREFIX=/tmp/xminstall \
  -D XMOTO_PACK=manual \
  -D CMAKE_BUILD_TYPE=Release -G Ninja ..

ninja

Xvfb :0 -screen 0 1024x768x16 &
xvfb_pid=$!

sleep 5

echo "pidof Xvfb:"
pidof Xvfb

ninja xmoto_pack

# not that any reasonable PID 1 responds to SIGTERM anyway ;)
if [ "$xvfb_pid" -gt 1 ]; then
  kill -15 "$xvfb_pid" || true
fi

# aaand if the graceful termination doesn't work..
jobs -p | xargs -r kill -9

#ninja install

"$MXE_TARGET"-cpack -G "NSIS"
"$MXE_TARGET"-cpack -G "ZIP"

fix_pkg_name() {
  if [ -z "$1" ] || [ -z "$2" ]; then
    >&2 echo "Error: [fix_pkg_name]: 2 parameters required"
  fi
  file="$1"
  ext="${file##*.}"
  mv "$file" "${file%.$ext}-$2${ext:+.${ext}}"
}

# add -win32 to .zip files, and -win32-setup to .exe files
for pkg in xmoto-*.zip; do fix_pkg_name "$pkg" "win32"; done
for pkg in xmoto-*.exe; do fix_pkg_name "$pkg" "win32-setup"; done

mkdir artifacts
mv xmoto-*.exe artifacts/
mv xmoto-*.zip artifacts/
