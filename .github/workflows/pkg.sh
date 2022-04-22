#!/bin/sh

append_pkg_name() {
  if [ -z "$1" ] || [ -z "$2" ]; then
    >&2 echo "Error: [append_pkg_name]: 2 parameters required"
  fi
  file="$1"
  ext="${file##*.}"
  mv "$file" "${file%.$ext}-$2${ext:+.${ext}}"
}
