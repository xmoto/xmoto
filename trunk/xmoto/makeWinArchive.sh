#!/bin/bash

TMP_DIR="/tmp"
ARCHIVE_SUFFIX="-win32-src"

function getVersion {
    if ! test -e ./configure.in
	then
	return 1
    fi

    grep "AM_INIT_AUTOMAKE" ./configure.in | 
    sed -e s+".*AM_INIT_AUTOMAKE(\(.*\),\(.*\)).*"+"\\1-\\2"+
}

function make_zip {
    ZIP_DIR="$1"
    ZIP_FILE="$2"

    RES=0

    CUR_DIR="`pwd`"
    cd "$ZIP_DIR""/.."
    zip -r "$ZIP_FILE" "`basename "$ZIP_DIR"`" || RES=1
    cd "$CUR_DIR"

    return "$RES"
}

function global_remove_previous_archive {
    if test -e ./"$VERSION_DIR"".zip"
	then
	rm ./"$VERSION_DIR"".zip" || exit 1
    fi
}

function global_make_destination_dir {
    # main dir
    mkdir "$DEST_DIR""/""bin"   || return 1
    mkdir "$DEST_DIR""/""src"   || return 1
    mkdir "$DEST_DIR""/""vcnet" || return 1
    cp "./ChangeLog" "$DEST_DIR""/""ChangeLog.txt" || return 1
    cp "./README"    "$DEST_DIR""/""README.txt"    || return 1
    cp "./COPYING"   "$DEST_DIR""/""COPYING.txt"   || return 1
    cp "./TODO"      "$DEST_DIR""/""TODO.txt"      || return 1
    cp "./BUGS"      "$DEST_DIR""/""BUGS.txt"      || return 1
    
    # bin dir
    mkdir "$DEST_DIR""/""bin/Levels"   || return 1
    mkdir "$DEST_DIR""/""bin/Replays"  || return 1
    mkdir "$DEST_DIR""/""bin/Sounds"   || return 1
    mkdir "$DEST_DIR""/""bin/Textures" || return 1
    cp "./bin/xmoto.bin"    "$DEST_DIR""/""bin" || return 1
    cp ./bin/*.dat          "$DEST_DIR""/""bin" || return 1
    cp "./bin/fonts.dat"    "$DEST_DIR""/""bin" || return 1
    cp "./bin/package.lst"  "$DEST_DIR""/""bin" || return 1
    cp "./bin/xmoto.nsi"    "$DEST_DIR""/""bin" || return 1
    cp "./bin/xmoto.ogg"    "$DEST_DIR""/""bin" || return 1

    # bin/Levels dir
    cp ./bin/Levels/*.lvl "$DEST_DIR""/""bin/Levels" || return 1
    cp ./bin/Levels/*.lvs "$DEST_DIR""/""bin/Levels" || return 1
    cp ./bin/Levels/*.lpk "$DEST_DIR""/""bin/Levels" || return 1

    # bin/Theme dir
    mkdir "$DEST_DIR""/""bin/Themes"                               || return 1
    cp ./bin/Themes/*.xml "$DEST_DIR""/""bin/Themes"               || return 1

    # bin/Sounds dir
    mkdir "$DEST_DIR""/""bin/Sounds/Engine"                         || return 1
    cp ./bin/Sounds/*.ogg  "$DEST_DIR""/""bin/Sounds"               || return 1
    cp ./bin/Sounds/Engine/*.wav  "$DEST_DIR""/""bin/Sounds/Engine" || return 1

    # po
    mkdir "$DEST_DIR""/""po"
    cp ./po/*.po ./po/*.gmo po/*.pot "$DEST_DIR""/""po"

    # bin/Textures dir
    mkdir "$DEST_DIR""/""bin/Textures/Textures" || return 1
    mkdir "$DEST_DIR""/""bin/Textures/Anims"    || return 1
    mkdir "$DEST_DIR""/""bin/Textures/Effects"  || return 1
    mkdir "$DEST_DIR""/""bin/Textures/Fonts"    || return 1
    mkdir "$DEST_DIR""/""bin/Textures/Misc"     || return 1
    mkdir "$DEST_DIR""/""bin/Textures/Riders"   || return 1
    mkdir "$DEST_DIR""/""bin/Textures/Sprites"  || return 1
    mkdir "$DEST_DIR""/""bin/Textures/UI"       || return 1
    cp ./bin/Textures/Textures/*.jpg "$DEST_DIR""/""bin/Textures/Textures" || return 1
    cp ./bin/Textures/Anims/*.png    "$DEST_DIR""/""bin/Textures/Anims"    || return 1
    cp ./bin/Textures/Effects/*.jpg  "$DEST_DIR""/""bin/Textures/Effects"  || return 1
    cp ./bin/Textures/Effects/*.png  "$DEST_DIR""/""bin/Textures/Effects"  || return 1
    cp ./bin/Textures/Fonts/*.png    "$DEST_DIR""/""bin/Textures/Fonts"    || return 1
    cp ./bin/Textures/Misc/*.png     "$DEST_DIR""/""bin/Textures/Misc"     || return 1
    cp ./bin/Textures/Riders/*.png   "$DEST_DIR""/""bin/Textures/Riders"   || return 1
    cp ./bin/Textures/Sprites/*.png  "$DEST_DIR""/""bin/Textures/Sprites"  || return 1
    cp ./bin/Textures/UI/*.jpg       "$DEST_DIR""/""bin/Textures/UI"       || return 1
    cp ./bin/Textures/UI/*.png       "$DEST_DIR""/""bin/Textures/UI"       || return 1

    # src dir
    cp ./src/*.h   "$DEST_DIR""/""src" || return 1
    cp ./src/*.cpp "$DEST_DIR""/""src" || return 1

    mkdir "$DEST_DIR""/""src/image"   || return 1
    mkdir "$DEST_DIR""/""src/md5sum"  || return 1
    mkdir "$DEST_DIR""/""src/tinyxml" || return 1
    mkdir "$DEST_DIR""/""src/compression" || return 1
    mkdir "$DEST_DIR""/""src/arch" || return 1
    cp ./src/image/*.h     "$DEST_DIR""/""src/image"   || return 1
    cp ./src/image/*.cpp   "$DEST_DIR""/""src/image"   || return 1
    cp ./src/md5sum/*.h    "$DEST_DIR""/""src/md5sum"  || return 1
    cp ./src/md5sum/*.c    "$DEST_DIR""/""src/md5sum"  || return 1
    cp ./src/md5sum/*.cpp  "$DEST_DIR""/""src/md5sum"  || return 1
    cp ./src/tinyxml/*.h   "$DEST_DIR""/""src/tinyxml" || return 1
    cp ./src/tinyxml/*.cpp "$DEST_DIR""/""src/tinyxml" || return 1
    cp ./src/compression/*.h   "$DEST_DIR""/""src/compression" || return 1
    cp ./src/compression/*.cpp "$DEST_DIR""/""src/compression" || return 1
    cp ./src/arch/*.h   "$DEST_DIR""/""src/arch" || return 1
    cp ./src/arch/*.cpp "$DEST_DIR""/""src/arch" || return 1

    # vcnet dir
    cp ./vcnet/*.vcproj        "$DEST_DIR""/""vcnet" || return 1
    cp "./vcnet/icon1.ico"     "$DEST_DIR""/""vcnet" || return 1
    cp "./vcnet/xmoto.sln"     "$DEST_DIR""/""vcnet" || return 1
    cp ./vcnet/*.lib           "$DEST_DIR""/""vcnet" || return 1
    cp "./vcnet/resource.aps"  "$DEST_DIR""/""vcnet" || return 1
    cp "./vcnet/resource.h"    "$DEST_DIR""/""vcnet" || return 1
    cp "./vcnet/resource.rc"   "$DEST_DIR""/""vcnet" || return 1

    # add windows requirements
    cp ./win_tree/bin/*.dll "$DEST_DIR""/""bin" || return 1
    mkdir "$DEST_DIR""/""src/ode"   || return 1
    mkdir "$DEST_DIR""/""src/SDL"   || return 1
    mkdir "$DEST_DIR""/""src/curl"  || return 1
    mkdir "$DEST_DIR""/""src/bzip2" || return 1
    cp ./win_tree/src/*.h       "$DEST_DIR""/""src"       || return 1
    cp ./win_tree/src/*.cpp     "$DEST_DIR""/""src"       || return 1
    cp ./win_tree/src/ode/*.h   "$DEST_DIR""/""src/ode"   || return 1
    cp ./win_tree/src/SDL/*.h   "$DEST_DIR""/""src/SDL"   || return 1
    cp ./win_tree/src/curl/*.h  "$DEST_DIR""/""src/curl"  || return 1
    cp ./win_tree/src/image/*.h "$DEST_DIR""/""src/image" || return 1
    cp ./win_tree/src/bzip2/*.h "$DEST_DIR""/""src/bzip2" || return 1
    cp ./win_tree/src/bzip2/*.c "$DEST_DIR""/""src/bzip2" || return 1
}

VERSION="`getVersion`"
VERSION_DIR="$VERSION""$ARCHIVE_SUFFIX"
DEST_DIR="$TMP_DIR""/""$VERSION_DIR"

# remove previous archive  
global_remove_previous_archive || exit 1

# remake win_tree
rm -rf win_tree || exit 1
mkdir  win_tree || exit 1
cd win_tree
unzip ../Win32Libs.zip > /dev/null || exit 1
cd ..

# make destination dir
mkdir "$DEST_DIR" || exit 1
global_make_destination_dir || exit 1

# make zip
make_zip "$TMP_DIR""/""$VERSION_DIR" "`pwd`""/""$VERSION_DIR"".zip" > /dev/null || exit 1

# delete dir
rm -rf "$DEST_DIR" || exit 1

echo "Succes"
