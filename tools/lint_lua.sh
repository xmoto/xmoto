#!/bin/bash

# Dependencies:
# - luacheck (https://github.com/luarocks/luacheck)
# - xmllint
# - recode
# - ctags


# Error/warning codes:
# W611: line contains only whitespace
# W612: line contains trailing whitespace

usage() {
  echo "Usage: $(basename "$0") <level file>"
}

header_file="$(readlink -f "${PWD}/../src/xmoto/LuaLibGame.h")"
level="$1"

[ -f "$header_file" ] || {
  >&2 echo "Couldn't find header file: $header_file"
  exit 1
}

[ -z "$1" ] && { >&2 usage; exit 1; }

gen_config() {
  echo "globals={"
  ctags \
      -x \
      --kinds-c++=p \
      --language-force=c++ \
      "$header_file" \
    | grep '^L_Game' | cut -d' ' -f1 \
    | sed 's/L_Game_/Game\./; s/.*/"&",/'
  echo "}"
}

xmllint --noout --noent --xpath "//script/text()" "$level" \
  | recode html..ascii \
  | luacheck \
    --config <(gen_config) \
    --ignore '611' '612' \
    --codes \
    --std lua54 \
    --globals \
    --no-cache \
    --allow-defined \
    --module \
    -- -
