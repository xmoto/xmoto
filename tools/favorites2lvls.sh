#!/bin/bash

if test $# -ne 1
then
    echo "$0"" <profile name>"
    exit 1
fi

ID_PROFILE="$1"

echo "SELECT b.filepath FROM levels_favorite AS a, levels AS b ON a.id_level=b.id_level WHERE a.id_profile=\"""$ID_PROFILE""\";" |
sqlite3 ~/.local/share/xmoto/xm.db |
while read FILE
do
    if ! echo "$FILE" | grep -qE "^/"
    then
	echo "Warning: ""$FILE"" is not available on the disk" >&2
    else
	if ! cp "$FILE" .
	    then
	    echo "Warning: get ""$FILE"" failed" >&2
	fi
    fi
done
