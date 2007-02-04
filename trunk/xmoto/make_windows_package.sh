#!/bin/bash

function getVersion {
    if ! test -e ./configure.in
	then
	return 1
    fi

    grep "AM_INIT_AUTOMAKE" ./configure.in |
    sed -e s+".*AM_INIT_AUTOMAKE(\(.*\),\(.*\)).*"+"\\2"+
}

function prepare_tmp_directory {
    ZIPDIR="$1"

    if test -e "$ZIPDIR"
	then
	rm -rf "$ZIPDIR" || return 1
    fi
    mkdir "$ZIPDIR" || return 1

    return 0
}

function fill_tmp_directory {
    ZIPDIR="$1"

    (
    cd "$ZIPDIR" || return 1
    if test ! -f ../src/xmoto.exe
	then
	echo "xmoto.exe does not exist" 1>&2
	return 1
    fi

    if test ! -f ../bin/xmoto.bin
	then
	echo "xmoto.bin does not exist" 1>&2
	return 1
    fi

    if test ! -f ../mingw_lib.zip
	then
	echo "mingw_lib.zip does not exist" 1>&2
	return 1
    fi

    cp ../src/xmoto.exe .            || return 1
    i586-mingw32msvc-strip xmoto.exe || return 1
    cp ../bin/xmoto.bin .     	     || return 1
    unzip -q ../mingw_lib.zip 	     || return 1
    mv mingw_lib/* .          	     || return 1
    rmdir mingw_lib           	     || return 1

    # extra files
    cp ../README    README.txt    || return 1
    unix2dos        README.txt    || return 1
    cp ../COPYING   COPYING.txt   || return 1
    unix2dos        COPYING.txt   || return 1
    cp ../ChangeLog ChangeLog.txt || return 1
    unix2dos        ChangeLog.txt || return 1
    )
}

function make_zip {
    ZIPDIR="$1"
    ZIPFILE="$2"

    zip -q -r "$ZIPFILE" "$ZIPDIR" || return 1
}

VERSION=`getVersion`
ZIPDIR="xmoto-""$VERSION"
ZIPFILE="$ZIPDIR"".zip"

if ! prepare_tmp_directory "$ZIPDIR"
then
    echo "Unable to make the tmp directory" 1>&2
    exit 1
fi

if ! fill_tmp_directory "$ZIPDIR"
then
    rm -rf "$ZIPDIR"
    echo "Unable to get required files" 1>&2
    exit 1
fi

if ! make_zip "$ZIPDIR" "$ZIPFILE"
then
    pwd
    echo rm -rf "$ZIPDIR"
    rm -rf "$ZIPDIR"
    echo "Unable to make the zip" 1>&2
    exit 1    
fi

rm -rf "$ZIPDIR"
echo "Succes"
