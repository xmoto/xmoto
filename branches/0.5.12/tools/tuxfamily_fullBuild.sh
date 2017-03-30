#!/bin/bash

cd ~/xm/build || return 1

SSHDIR="aspegic500@ssh.tuxfamily.org:~/xmoto/xmoto-repository/xmoto/dev"

function checkBuilds {
    #(
    #	cd builds || return 1
    #	find . -type f |
    #	while read FILE
    #	  do
    #	  if test ! -f "$PM""/""$FILE"
    #	      then
    #	      return 1
    #	  fi
    #	done || return 1
    #) || return 1
    return 1
}

function updateBuilds {
# remove files
#    find "$PM" -type f |
#    while read FILE
#      do
#      rm -rf "$FILE" || return 1
#    done

    (
	cd builds || return 1
	find . -type f |
	while read FILE
	  do
	  echo "Copying ""$FILE"
	  echo scp "$FILE" "$SSHDIR"
	  scp "$FILE" "$SSHDIR"
	done || return 1
    ) || return 1
}

echo "Building packages..."
./buildPackages.sh || exit 1

echo "Checking builds"
FAILED=0
if ! checkBuilds
then
    if ! updateBuilds
	then
	FAILED=1
    fi
fi

if test "$FAILED" -eq 1
then
    exit 1
fi

echo success
