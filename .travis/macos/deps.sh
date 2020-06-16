#!/bin/sh

set -ex

brew update
brew install \
  cmake ninja jpeg libpng gettext sdl sdl_mixer \
  sdl_net sdl_ttf bz2 curl libxml2 xz zlib
