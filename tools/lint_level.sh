#!/bin/bash -e

print_usage() {
  basename="$(basename "$0")"
  cat <<EOF
Usage: $basename <file>
EOF
}

level_file="$1"

if [ "$#" -ne 1 ]; then
  >&2 print_usage
  exit 1
fi

if [ ! -f "$level_file" ]; then
  >&2 echo "$level_file: No such file of directory"
  exit 1
fi


git_root="$(git rev-parse --show-toplevel)"
luacheckrc="/tmp/luacheckrc"
decl_file="$git_root/src/xmoto/LuaLibGame.h"

declare -a ignore_list=(
  "611" # line contains only whitespace
  "612" # line contains trailing whitespace
)

{
  echo "globals={"
  ctags \
      -x \
      --kinds-c++=p \
      --language-force=c++ \
      "$decl_file" \
    | grep '^L_Game' | cut -d' ' -f1 \
    | sed 's/L_Game_/Game\./; s/.*/"&",/'
  echo "}"
} >"$luacheckrc"

script_offset="$(awk '/<script>/ {print FNR}' "$level_file")"

xmllint --noout --noent --xpath "//script/text()" "$level_file" \
  | recode html..ascii \
  | luacheck \
    --config "$luacheckrc" \
    --ignore "${ignore_list[@]}" \
    --codes \
    --std lua54 \
    --globals \
    --no-cache \
    --allow-defined \
    --module \
    -- - \
  | perl -sne \
    's/stdin:(\d+)/"$filename:".($1+$offset-1)/e; print' \
    -- \
    -offset="$script_offset" \
    -filename="$level_file"

