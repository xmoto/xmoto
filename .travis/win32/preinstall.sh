#!/bin/sh

set -ex

[[ ! -f C:/tools/msys64/msys2_shell.cmd ]] && rm -rf C:/tools/msys64

choco uninstall -y mingw
choco upgrade --no-progress -y msys2

