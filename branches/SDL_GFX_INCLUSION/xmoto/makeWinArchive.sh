#!/bin/bash

TMP_DIR="/tmp"
ARCHIVE_SUFFIX="-win32-src"
NSI_FILE="bin/xmoto.nsi"

function getVersion {
    if ! test -e ./configure.in
	then
	return 1
    fi

    grep "AM_INIT_AUTOMAKE" ./configure.in | 
    sed -e s+".*AM_INIT_AUTOMAKE(\(.*\),\(.*\)).*"+"\\2"+
}

function updateNSIversion {
  NSI_FILE="$1"
  XMOTO_VERSION="$2"

  cat "$NSI_FILE" | 
  sed -e s+"^Name \".*\"$"+"Name \"X-Moto ""$XMOTO_VERSION""\""+ |
  sed -e s+"^OutFile \".*\"$"+"OutFile \"xmoto-""$XMOTO_VERSION""-win32-setup.exe\""+ > "$NSI_FILE"_tmp
  mv "$NSI_FILE"_tmp "$NSI_FILE"
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

function cpDir {
    SUBDIR="$1"
    EXTENSIONS="$2"
    DEST_DIR="$3"

    for EXTENSION in $EXTENSIONS
      do
      find "$SUBDIR" -type f -name "*.""$EXTENSION" |
      while read FILE
	do
	mkdir -p "$DEST_DIR""/""`dirname "$FILE"`" || return 1
	cp "$FILE" "$DEST_DIR""/""$FILE"           || return 1
      done
    done
}

function global_remove_previous_archive {
    if test -e ./"$VERSION_DIR"".zip"
	then
	rm ./"$VERSION_DIR"".zip" || return 1
    fi
}

function global_remake_win_tree {
  rm -rf win_tree || return 1
  mkdir  win_tree || return 1
  cd win_tree
  unzip ../Win32Libs.zip > /dev/null || return 1
  cd ..
}

function global_make_destination_dir {
    cpDir "src"   "cpp h c"                                         "$DEST_DIR" || return 1
    cpDir "bin"   "bin dat lst nsi ogg wav jpg png lvl lvs lpk xml frag vert" "$DEST_DIR" || return 1
    cpDir "po"    "po gmo"                                          "$DEST_DIR" || return 1
    cpDir "vcnet" "vcproj ico sln lib aps h rc"                     "$DEST_DIR" || return 1
    #cpDir "tools" "py inx"                                          "$DEST_DIR" || return 1

    for i in ChangeLog README COPYING TODO BUGS # tools/svg2lvl/README
    do
      cp "$i" "$DEST_DIR""/""$i"".txt"  || return 1
      unix2dos "$DEST_DIR""/""$i"".txt" || return 1
    done

    # add windows requirements
    cd ./win_tree
    cpDir "bin"   "dll"     "$DEST_DIR" || return 1
    cpDir "src"   "h cpp c" "$DEST_DIR" || return 1
    cd ..
}

VERSION="`getVersion`"
VERSION_DIR="xmoto-""$VERSION""$ARCHIVE_SUFFIX"
DEST_DIR="$TMP_DIR""/""$VERSION_DIR"

# remove previous archive  
global_remove_previous_archive || exit 1

# remake win_tree
global_remake_win_tree         || exit 1

# update nsi file
updateNSIversion "$NSI_FILE" "`getVersion`"

# make destination dir
mkdir "$DEST_DIR" || exit 1
global_make_destination_dir || exit 1

# make zip
make_zip "$TMP_DIR""/""$VERSION_DIR" "`pwd`""/""$VERSION_DIR"".zip" > /dev/null || exit 1

# delete tmp dir
rm -rf "$DEST_DIR" || exit 1

echo "Succes"
