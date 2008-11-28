#!/bin/bash
# Kees Jongenburger
# proposed indenting standard
# usages ../scripts/xmoto-indent.sh file.c
# WARNING: the indenting is replaced inline

INDENT=2
if [ "$1" == "-n" ]; then
	shift
	INDENT=$1
	shift
fi


FLAGS="-bad -bap -bbo -nbc -br -brs -c33 -cd33 -ncdb -ce -ci$INDENT -cli0 -cp33 -ncs -d0 -di1 -nfc1  -hnl -i$INDENT -ip0 -l75 -lp -npcs -nprs -npsl -saf -sai -saw -nsc -nsob -nss"

indent $FLAGS $@
