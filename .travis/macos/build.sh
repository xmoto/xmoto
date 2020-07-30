#!/bin/sh

set -ex

mkdir build && cd build
cmake -D BUILD_MACOS_BUNDLE=ON \
      -D CMAKE_BUILD_TYPE=Release -G Ninja ..
ninja

# Homebrew on Catalina (10.15) sets inconsistent permissions on dylibs
# and CMake happily copies them as is, and then chokes on its own brilliance.
sudo \
  cpack -G DragNDrop

mkdir artifacts
mv xmoto-*.dmg artifacts/
