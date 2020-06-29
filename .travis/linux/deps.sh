#!/bin/sh

set -ex

sudo apt-get update
sudo apt-get install -y \
  curl ninja-build rpm \
  libsqlite3-dev libjpeg-dev libbz2-dev liblua5.1-0-dev \
  zlib1g-dev libpng-dev libglu1-mesa-dev \
  libcurl3-openssl-dev libxdg-basedir-dev libxml2-dev libgl1-mesa-dev \
  libsdl2-dev libsdl2-mixer-dev libsdl2-ttf-dev libsdl2-net-dev

