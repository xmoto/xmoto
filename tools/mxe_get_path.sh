#!/bin/sh

# Prints the current MXE installation's path if found

usage() {
    echo "Usage: $(basename "$0") <mxe target>"
}

if [ -z "$1" ]; then
    >&2 usage
    exit 1
fi

target="$1"

if mxe_path="$(command -v -- "${target}-gcc")"; then
    mxe_path="$(dirname "$mxe_path" | sed -e 's|\(.\)\{0,1\}usr/bin||g')"
fi

if [ ! -d "$mxe_path" ]; then
    exit 1
fi

echo "$mxe_path"
