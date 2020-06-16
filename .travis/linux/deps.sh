#!/bin/sh

set -ex

sudo apt-get update
sudo apt-get install -y \
  curl ninja-build rpm \
  libsqlite3-dev libjpeg-dev libbz2-dev liblua5.1-0-dev \
  zlib1g-dev libpng-dev libglu1-mesa-dev \
  libcurl3-openssl-dev libxdg-basedir-dev libxml2-dev \ libgl1-mesa-dev \
  libsdl1.2-dev libsdl-mixer1.2-dev libsdl-ttf2.0-dev libsdl-net1.2-dev

