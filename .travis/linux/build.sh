#!/bin/sh

set -ex

mkdir build && cd build
mkdir -p /tmp/xminstall
cmake -DCMAKE_INSTALL_PREFIX=/tmp/xminstall \
      -DCMAKE_BUILD_TYPE=Release -G Ninja ..
ninja
ninja xmoto_pack
ninja install

if case "$TRAVIS_COMPILER" in clang*) ;; *) false;; esac; then
  # Don't package when building with clang; this is just for building
  exit 0
fi

cpack -G "DEB"
cpack -G "RPM"

mkdir artifacts
mv xmoto-*.deb artifacts/
mv xmoto-*.rpm artifacts/

