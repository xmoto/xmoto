#!/bin/sh

set -ex

export HOMEBREW_NO_AUTO_UPDATE=1
export HOMEBREW_NO_ANALYTICS=1

# packages to be checked for before installing them
for pkg in cmake ninja ccache jpeg libpng \
           bzip2 curl sqlite3 libxml2 zlib; do
  brew list "$pkg" &>/dev/null || brew install "$pkg"
done

# install the rest of the packages
brew install gettext sdl sdl_mixer sdl_net sdl_ttf

brew unlink gettext
brew link --force gettext

