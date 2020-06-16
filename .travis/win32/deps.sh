#!/bin/bash

set -ex

#export msys2='cmd //C RefreshEnv.cmd '
#export msys2+='& set MSYS=winsymlinks:nativestrict '
#export msys2+='& C:\\tools\\msys64\\msys2_shell.cmd -defterm -no-start'
#export mingw64="$msys2 -mingw64 -full-path -here -c "\"\$@"\" --"
#export msys2+=" -msys2 -c "\"\$@"\" --"

pacman --noconfirm --needed -S \
  make \
  mingw-w64-i686-{gcc,cmake} \
  mingw-w64-i686-{curl,libpng,openjpeg,zlib,libxml2,sqlite3} \
  mingw-w64-i686-{SDL,SDL_mixer,SDL_net,SDL_ttf}

#export PATH="/C/tools/msys64/mingw64/bin:$PATH"
#export MAKE=mingw32-make  # so that Autotools can find it

#taskkill //IM gpg-agent.exe //F || exit 0 # https://travis-ci.community/t/4967

