#!/bin/bash

XMOTO_DIR="${HOME}/.local/share/xmoto"

if test $# -ne 4
then
    echo "Syntax: ""$0"" ""<weblevels.xml>"" ""<favoriteLevels.xml>"" ""<directory>"" ""<packname>" 1>&2
    exit 1
fi

WEBLEVELFILE="$1"
FAVORITEFILE="$2"
DIRECTORY="$3"
PACKNAME="$4"

if test -d "$DIRECTORY"
then
    echo "The directory ""$DIRECTORY"" already exists" 1>&2
    exit 1
fi

if ! mkdir -p "$DIRECTORY"
then
    echo "Unable to create ""$DIRECTORY" 1>&2
    exit 1
fi

FILTER=`(
cat "$FAVORITEFILE"   |
grep -E "^<level id=" |
sed -e s+"^[ ]*<level[ ]*id=\"\([^\"]*\)\".*"+"\\1"+ |
xargs echo |
tr " " "|"
)`

cat "$WEBLEVELFILE" |
grep -E "^<level[ ]*level_id=" |
sed -e s+"^<level[ ]*level_id=\"\([^\"]*\)\" .* url=\"[^\"]*[^/\"]*/\([^/\"]*.lvl\)\".*"+"\\1 \\2"+ |
grep -E "^""$FILTER"" " |
(while read LEVELID LEVELFILE
    do
    find "$XMOTO_DIR""/""Levels" -name "$LEVELFILE"
    done
) |
while read FILE
  do
  if ! cp "$FILE" "$DIRECTORY"
      then
      echo "Unable to copy the file ""$FILE" 1>&2
      exit 1
  fi
done

find "$DIRECTORY""" -name "*.lvl" |
while read FILE
do
  if grep -E "<level id=\"[^\"]*\"[ ]*levelpack=\"[\"]*\"" "$FILE"
      then
      echo "BOUH the level is already in a pack" 1>&2
      exit 1
  fi

  cat "$FILE" |
  sed -e s+"\(<level id=\"[^\"]*\"\)"+"\\1 levelpack=\"""$PACKNAME""\""+ > "$FILE"".tmp" || return 1
  mv "$FILE"".tmp" "$FILE"    || return 1
done || return 1

echo "succes"
