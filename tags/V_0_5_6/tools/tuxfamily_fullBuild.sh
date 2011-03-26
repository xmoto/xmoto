#!/bin/bash

cd /home/nicolas/xm/build || return 1

FTPD="ftp://ftp.tuxfamily.org/xmoto/xmoto-repository/xmoto/dev"
PM="/media/xmoto_repository_svn"
LOGIN="aspegic500"
PASSWORD="xxx"

function checkBuilds {
    (
	cd builds || return 1
	find . -type f |
	while read FILE
	  do
	  if test ! -f "$PM""/""$FILE"
	      then
	      return 1
	  fi
	done || return 1
    ) || return 1
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
	  cp "$FILE" "$PM""/""$FILE" ||  return 1
	done || return 1
    ) || return 1
}

echo "Building packages..."
./buildPackages.sh || exit 1

#echo no ftping
#exit 1

echo "Mounting ""$PM"
echo curlftpfs "$FTPD" "$PM" -o user="$LOGIN":"$PASSWORD"
curlftpfs "$FTPD" "$PM" -o user="$LOGIN":"$PASSWORD" || exit 1

echo "Checking builds"
FAILED=0
if ! checkBuilds
then
    if ! updateBuilds
	then
	FAILED=1
    fi
fi

echo "Umount ""$PM" || exit 1
fusermount -u "$PM" || exit 1

if test "$FAILED" -eq 1
then
    exit 1
fi

echo success
