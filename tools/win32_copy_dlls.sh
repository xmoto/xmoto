#!/bin/sh

usage() {
    cat <<EOF
Usage:
    $(basename "$0") -t <target> -o <output_dir> [-e <executable>] [-r]
EOF
}

if [ $# -eq 0 ]; then >&2 usage; exit 1; fi

is_mxe=0

[ -n "$MSYSTEM_PREFIX" ]
is_mxe=$?

tools_dir="$(readlink -f "$(dirname "$0")")"

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
            >&2 usage
            exit 1
            ;;
    esac
done
shift $((OPTIND-1))

# Check if path is relative
if case "$output_dir" in /*) false;; esac; then
    if ! _output_dir="$(readlink -nf "$output_dir")"; then
        >&2 echo "Error: Failed to resolve relative path: '$output_dir'"
        exit 1
    fi
    output_dir="$_output_dir"
fi

if ! mkdir -p "$output_dir"; then
    >&2 echo "Error: Failed to create output directory"
    exit 1
fi

if [ -z "$executable" ]; then
    echo "No executable specified - Picking first .exe found"
    executable="$(find . -name "*.exe" | head -n1)"
fi

echo "Output directory: $output_dir"
echo "Executable: $executable"

# Check if we're using MXE
# (should probably use the opposite boolean convention)
if [ "$is_mxe" -ne 0 ]; then
    if ! command -v -- "${target}-gcc" >/dev/null; then
        >&2 echo "Error: Invalid target '$target'"
        exit 1
    elif ! echo "$target" | grep -q -- "shared"; then
        echo "Not a shared target, exiting"
        exit 0
    fi

    if [ -z "$MXE_PATH" ]; then
        MXE_PATH="$(command -- "$tools_dir/mxe_get_path.sh" "$target")"
    fi

    if [ -z "$MXE_PATH" ]; then
        >&2 echo "Failed to get MXE path"
        >&2 echo "Tools directory: $tools_dir"
        exit 1
    fi

    echo "MXE path: $MXE_PATH"
else
    # assume MSYS2
    MSYS2_PATH="$target"
    echo "MSYS2 path: $MSYS2_PATH"
fi

copy_dlls() {
    [ -z "$1" ] && { >&2 echo "Error: Argument required"; exit 1; }
    path="$1"
    force="${2:-0}"

    if [ ! -f "$path" ]; then
        >&2 echo "Warning: File '$path' does not exist"
    fi

    [ "$force" -eq 0 ] && if ! file -b -- "$path" | grep -q "(DLL)"; then
        return
    fi

    if [ "$is_mxe" -ne 0 ]; then
        dll_directory="$MXE_PATH/usr/$target/bin"
    else
        dll_directory="$MSYS2_PATH/bin"
    fi

    strings "$path" | grep -i '\.dll$' \
        | while IFS="" read -r dll || [ -n "$dll" ]; do

        source_path="$dll_directory/$dll"
        dest_path="$output_dir/$dll"

        [ ! -f "$source_path" ] && continue
        [ -f "$dest_path" ] && continue
        echo "  $dll"

        cp -n -- "$source_path" "$output_dir/$dll"
        copy_dlls "$source_path"
    done
}

printf "\nCopying required DLLs...\n"

if [ "$recursive" -ne 0 ]; then
    copy_dlls "$executable" 1
else
    OBJDUMP="$MXE_PATH/usr/bin/$target-objdump"

    if [ ! -e "$OBJDUMP" ]; then
        OBJDUMP="$(command -v "objdump")" || \
            { >&2 echo "Error: objdump not found"; exit 1; }
    fi

    "$MXE_PATH/tools/copydlldeps.sh" \
        --infile "$executable" \
        --destdir "$output_dir" \
        --recursivesrcdir "$MXE_PATH/usr/$target/" \
        --copy \
        --objdump "$OBJDUMP"
fi

echo "Done copying DLLs"
