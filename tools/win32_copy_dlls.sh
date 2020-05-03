#!/usr/bin/sh

is_mxe=0
mxe_compiler_suffix="-gcc"

usage() {
    cat <<EOF
Usage:
    $(basename "$0") -t <target> -o <output_dir> [-e <executable>] [-r]
EOF
}

# expects <compiler> to be set
guess_mxe_path() {
    compiler="$1${mxe_compiler_suffix}"
    if MXE_PATH="$(command -v -- "$compiler")"; then
        MXE_PATH="$(dirname "$MXE_PATH" | sed -e 's|\(.\)\{0,1\}usr/bin||g')"
    fi
    if [ ! -d "$MXE_PATH" ]; then
        >&2 echo "fatal: failed to guess MXE_PATH"
        exit 1
    fi
}

if [ $# -eq 0 ]; then >&2 usage; exit 1; fi

[ -n "$MSYSTEM_PREFIX" ]
is_mxe=$?

target=""
output_dir=""
executable=""
recursive=0
while getopts "t:o:e:r" opt; do
    case "${opt}" in
        t) target="${OPTARG}"
            if [ -z "$target" ]; then >&2 usage; exit 1; fi
            ;;
        o) output_dir="${OPTARG}"
            if [ -z "$output_dir" ]; then >&2 usage; exit 1; fi
            ;;
        e) executable="${OPTARG}"
            if [ -z "$executable" ]; then >&2 usage; exit 1; fi
            ;;
        r) recursive=1 ;;
        *)
            >&2 usage; exit 1
            ;;
    esac
done
shift $((OPTIND-1))

# check if path is relative
if case "$output_dir" in /*) false;; esac; then
    if ! _output_dir="$(readlink -nf "$output_dir")"; then
        >&2 echo "fatal: failed to resolve relative path '$output_dir'"
        exit 1
    fi
    output_dir="$_output_dir"
fi
mkdir -p "$output_dir" || echo 'warning: mkdir -p failed'

if [ ! -d "$output_dir" ]; then
    >&2 echo "fatal: directory '$output_dir' does not exist!"
fi
echo "output_dir: $output_dir"

if [ -z "$executable" ]; then
    echo "no executable specified - running on first .exe found"
    executable="$(find . -name "*.exe" | head -n1)"
fi
echo "executable: $executable"

# check if we're using MXE
# (should probably use the opposite boolean convention)
if [ "$is_mxe" -ne 0 ]; then
    if ! command -v -- "${target}${mxe_compiler_suffix}" >/dev/null; then
        >&2 echo "fatal: invalid target '$target'"
        exit 1
    elif ! echo "$target" | grep -q -- "shared"; then
        echo "not a shared target, exiting"
        exit 0
    fi

    if [ -z "$MXE_PATH" ]; then
        guess_mxe_path "$target"
    fi
    echo "MXE_PATH: $MXE_PATH"
else
    # assume MSYS2
    MSYS2_PATH="$target"
    echo "MSYS2_PATH: $MSYS2_PATH"
fi

printf "\ncopying required DLLs...\n"

dll_check_deps() {
    [ -z "$1" ] && { >&2 echo "argument required"; exit 1; }
    path="$1"
    force="${2:-0}"
    # make sure the specified file exists
    if [ ! -f "$path" ]; then
        echo "warning: file '$path' does not exist"
    fi

    [ "$force" -eq 0 ] && if ! file -b -- "$path" | grep -q "(DLL)"; then
        return
    fi

    # yes, using strings(1) is a dirty hack
    strings "$path" | grep -i '\.dll$' \
        | while IFS="" read -r dll || [ -n "$dll" ]; do

        if [ "$is_mxe" -ne 0 ]; then
            dll_path="$MXE_PATH/usr/$target/bin/$dll"
        else
            dll_path="$MSYS2_PATH/bin/$dll"
        fi
        echo "dll_path: $dll_path"

        # check if the file has already been copied to $output_dir
        if [ ! -f "$output_dir/$dll" ] && [ -f "$dll_path" ]; then
            cp "$dll_path" "$output_dir/$dll"
            dll_check_deps "$dll_path"
        fi
    done
}

if [ "$recursive" -ne 0 ]; then
    # technically, even with this,
    # you may still need to rely on copydlldeps.sh
    dll_check_deps "$executable" 1
else
    # check out:
    # https://github.com/mxe/mxe/blob/master/tools/copydlldeps.md

    OBJDUMP="$MXE_PATH/usr/bin/$target-objdump"
    if [ ! -e "$OBJDUMP" ]; then
        OBJDUMP="$(command -v "objdump")" || \
            { >&2 echo "fatal: objdump not found"; exit 1; }
    fi

    "$MXE_PATH/tools/copydlldeps.sh" \
        --infile "$executable" \
        --destdir "$output_dir" \
        --recursivesrcdir "$MXE_PATH/usr/$target/" \
        --copy \
        --objdump "$OBJDUMP"
fi

