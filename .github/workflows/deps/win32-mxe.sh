#!/bin/bash

sudo apt-get update
sudo apt-get install -y gnupg2 software-properties-common

# for wine32
sudo dpkg --add-architecture i386

sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 86B72ED9
sudo add-apt-repository 'deb [arch=amd64] https://mirror.mxe.cc/repos/apt focal main'
sudo apt-get update

# This fixes https://github.com/actions/runner-images/issues/4589
sudo apt install -y --allow-downgrades \
  libpcre2-8-0/focal \
  libpcre2-16-0/focal \
  libpcre2-32-0/focal \
  libpcre2-posix2/focal \
  libgd3/focal

sudo apt-get install -y wine wine32

sudo apt-get install -y \
  mxe-i686-w64-mingw32.shared-{cc,cmake-conf,curl,jpeg,libpng,zlib,libxml2,sqlite,gettext} \
  mxe-i686-w64-mingw32.shared-{smpeg2,sdl2,sdl2-mixer,sdl2-net,sdl2-ttf,lzma,bzip2,nsis}
