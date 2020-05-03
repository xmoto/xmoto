#!/usr/bin/sh

# A simple script to `strip` all DLL files in the specified folder

usage() {
    cat <<EOF
Usage:
    $(basename "$0") <strip executable> <dll folder> -r
EOF
}

if [ "$#" -lt 2 ]; then
    >&2 usage; exit 1
fi

check_empty_arg() {
    [ -z "$1" ] && return 1 || return 0
}

check_empty_arg "$1" || { >&2 echo "empty argument given: \$1"; exit 1; }
check_empty_arg "$2" || { >&2 echo "empty argument given: \$2"; exit 1; }

strip="$1"
dll_folder="$2"

if ! echo "$strip" | grep -q -- "-\?strip"; then
    >&2 echo "fatal: Not a \`strip\` program!"
    exit 1
fi

if ! command -v -- "$strip" >/dev/null; then
    >&2 echo "fatal: invalid \`strip\` command!"
    exit 1
fi

num_dlls="$(find "$dll_folder" -mindepth 1 -maxdepth 1 -iname "*.dll" \
            -print -exec "$strip" "{}" \; | wc -l)"
echo "$num_dlls DLLs stripped"

