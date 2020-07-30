#!/bin/sh

set -ex

mkdir build && cd build
mkdir -p /tmp/xminstall
cmake -D CMAKE_INSTALL_PREFIX=/tmp/xminstall \
      -D CMAKE_BUILD_TYPE=Release -G Ninja ..
ninja
ninja xmoto_pack
ninja install

cpack -G "DEB"
cpack -G "RPM"

mkdir artifacts
mv xmoto-*.deb artifacts/
mv xmoto-*.rpm artifacts/
