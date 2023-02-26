#!/bin/sh

usage() {
  echo "Usage: $(basename "$0") <winepfx dir> <input dir> <build dir> <toolchain path>"
}

if [ $# -ne 4 ]; then >&2 usage; exit 1; fi

for arg in "$@"; do
    if [ -z "$arg" ]; then
        >&2 usage
        exit 1
    fi
done

# Needed for CI
[ -z "$DISPLAY" ] && export DISPLAY=:0.0

export WINEPREFIX="$1"
input_dir="$2"
build_dir="$3"
toolchain_path="$4"

if [ -z "$toolchain_path" ]; then
    >&2 echo "Failed to get MXE path"
    exit 1
fi

output_file="${build_dir}/bin/xmoto.bin"
mxe_target="$(basename -- "$toolchain_path")"

case "$mxe_target" in
  i686-*)
    export WINEARCH=win32
    ;;
  x86_64-*)
    export WINEARCH=win64
    ;;
  *)
    >&2 echo "Invalid MXE target: '$mxe_target'"
    exit 1
    ;;
esac

export WINEPATH="$toolchain_path/bin${WINEPATH:+";${WINEPATH}"}"
export WINEDEBUG=fixme-all,err+all

echo "Wine arch: $WINEARCH"
echo "Wine prefix: $WINEPREFIX"
echo "Wine dll path: $WINEPATH"

echo "Input directory: $input_dir"
echo "Build directory: $build_dir"
echo "Output file: $output_file"
echo "Toolchain path: $toolchain_path"
echo "MXE target: $mxe_target"

env DISPLAY= wine "${build_dir}/src/xmoto.exe" \
    --pack "$output_file" "$input_dir" \
    || exit $?
wineserver --kill
