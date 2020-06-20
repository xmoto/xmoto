#!/bin/bash

set -ex

sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 86B72ED9
sudo add-apt-repository 'deb [arch=amd64] http://mirror.mxe.cc/repos/apt bionic main'

# for xvfb
sudo add-apt-repository universe

sudo apt-get update

sudo apt-get install -y \
  mxe-i686-w64-mingw32.shared-{cc,curl,jpeg,libpng,zlib,libxml2,sqlite,gettext} \
  mxe-i686-w64-mingw32.shared-{smpeg2,sdl,sdl-mixer,sdl-net,sdl-ttf,lzma,bzip2,nsis} \
  wine-stable wine32 xvfb

