#!/bin/sh

set -ex

mkdir build && cd build
mkdir -p /tmp/xminstall
cmake -DCMAKE_INSTALL_PREFIX=/tmp/xminstall -G Ninja ..
ninja
ninja xmoto_pack
ninja install

cpack -G "DEB"
cpack -G "RPM"

