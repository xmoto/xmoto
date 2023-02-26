#!/bin/sh

# A simple script to `strip` all DLL files in the specified folder

usage() {
    cat <<EOF
Usage:
    $(basename "$0") <strip executable> <dll folder> -r
EOF
}

if [ "$#" -lt 2 ]; then
    >&2 usage
    exit 1
fi

for arg in "$@"; do
  if [ -z "$arg" ]; then
    >&2 usage
    exit 1
  fi
done

strip="$1"
dll_folder="$2"

if ! echo "$strip" | grep -q -- "-\?strip"; then
    >&2 echo "Error: Not a \`strip\` program"
    exit 1
fi

if ! command -v -- "$strip" >/dev/null; then
    >&2 echo "Error: Provided \`strip\` command not found"
    exit 1
fi

num_dlls="$(find "$dll_folder" -mindepth 1 -maxdepth 1 -iname "*.dll" \
            -print -exec "$strip" "{}" \; | wc -l)"
echo "$num_dlls DLLs stripped"
