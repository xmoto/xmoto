#!/bin/bash

function getVersion {
    RELATIVE_TRUNK_DIR="$1"

    if ! test -e ./"$RELATIVE_TRUNK_DIR"/configure.in
	then
	return 1
    fi

    grep "AM_INIT_AUTOMAKE" ./"$RELATIVE_TRUNK_DIR"/configure.in |
    sed -e s+".*AM_INIT_AUTOMAKE(\(.*\),\(.*\)).*"+"\\2"+
}

function makeNSI {
  NSI_FILENAME="$1"
  VERSION="$2"

  echo "Name \"X-Moto ""$VERSION""\""
  echo "OutFile \"xmoto-""$VERSION""-win32-setup.exe\""
  echo "InstallDir \$PROGRAMFILES\XMoto"
  echo "InstallDirRegKey HKLM \"Software\XMoto\" \"Install_Dir\""
  echo
  echo "BGGradient 000000 800000 FFFFFF"
  echo "InstallColors FF8080 000030"
  echo "XPStyle on"
  echo
  echo "ChangeUI all \"\${NSISDIR}\Contrib\UIs\default.exe\""
  echo
  echo "LicenseData \"COPYING.txt\""
  echo
  echo "Page license"
  echo "Page components"
  echo "Page directory"
  echo "Page instfiles"
  echo
  echo "UninstPage uninstConfirm"
  echo "UninstPage instFiles"
  echo
  echo "Section \"Game (required)\""
  echo
  echo "	SectionIn RO"
  echo "	SetOutPath \$INSTDIR"
  echo
  echo "	File xmoto.exe"
  echo "	File xmoto.bin"
  echo "        File /r Textures"
  echo "        File /r locale"
  echo "        File README.txt"
  echo "        File jpeg62.dll"
  echo "	File libcurl-4.dll"
  echo "	File libpng12.dll"
  echo "	File lua5.1.dll"
  echo "	File mgwz.dll"
  echo "	File ogg.dll"
  echo "	File SDL.dll"
  echo "	File SDL_mixer.dll"
  echo "	File SDL_net.dll"
  echo "	File vorbis.dll"
  echo "	File vorbisfile.dll"
  echo "	File zlib1.dll"
  echo "        File libintl3.dll"
  echo "        File libiconv2.dll"
  echo "        File sqlite3.dll"
  echo "        File SDL_ttf.dll"
  echo "        File mingwm10.dll"
  echo "	File COPYING.txt"
  echo "	File ChangeLog.txt"
  echo
  echo "	WriteRegStr HKLM SOFTWARE\XMoto \"Install_Dir\" \"\$INSTDIR\""
  echo
  echo "	WriteRegStr HKLM \"Software\Microsoft\Windows\CurrentVersion\Uninstall\XMoto\" \"DisplayName\" \"X-Moto\""
  echo "	WriteRegStr HKLM \"Software\Microsoft\Windows\CurrentVersion\Uninstall\XMoto\" \"UninstallString\" '\"\$INSTDIR\uninstall.exe\"'"
  echo "	WriteRegDWORD HKLM \"Software\Microsoft\Windows\CurrentVersion\Uninstall\XMoto\" \"NoRepair\" 1"
  echo "	WriteUninstaller \"uninstall.exe\""
  echo
  echo "SectionEnd"
  echo
  echo "Section \"Start Menu Shortcuts\""
  echo "	CreateDirectory \"\$SMPROGRAMS\X-Moto\""
  echo "        CreateShortCut \"\$SMPROGRAMS\X-Moto\X-Moto.lnk\" \"\$INSTDIR\xmoto.exe\" \"\" \"\$INSTDIR\xmoto.exe\" 0"
  echo "        CreateShortCut \"\$SMPROGRAMS\X-Moto\X-Moto (Windowed).lnk\" \"\$INSTDIR\xmoto.exe\" \"-win\" \"\$INSTDIR\xmoto.exe\" 0"
  echo "        CreateShortCut \"\$SMPROGRAMS\X-Moto\X-Moto for children.lnk\" \"\$INSTDIR\xmoto.exe\" \"--children\" \"\$INSTDIR\xmoto.exe\" 0"
  echo "        CreateDirectory \"\$PROFILE\\.local\share\xmoto\""
  echo "        CreateShortCut \"\$SMPROGRAMS\X-Moto\X-Moto User Data.lnk\" \"\$PROFILE\\.local\share\xmoto\""
  echo "	CreateShortCut \"\$SMPROGRAMS\X-Moto\X-Moto Uninstall.lnk\" \"\$INSTDIR\uninstall.exe\" \"\" \"\$INSTDIR\uninstall.exe\" 0"
  echo "SectionEnd"
  echo
  echo "Section \"Uninstall\""
  echo
  echo "	DeleteRegKey HKLM \"Software\Microsoft\Windows\CurrentVersion\Uninstall\XMoto\""
  echo "	DeleteRegKey HKLM SOFTWARE\XMoto"
  echo
  echo "	RMDir /r \"\$SMPROGRAMS\X-Moto\""
  echo "  	RMDir /r \"\$INSTDIR\""
  echo
  echo "SectionEnd"
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
    RELATIVE_TRUNK_DIR="$2"

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

    if test ! -f "../""$RELATIVE_TRUNK_DIR""/mingw_lib.zip"
	then
	echo "mingw_lib.zip does not exist" 1>&2
	return 1
    fi

    cp ../src/xmoto.exe .            	       || return 1
    i586-mingw32msvc-strip xmoto.exe 	       || return 1
    cp ../bin/xmoto.bin .     	     	       || return 1
    unzip -q "../""$RELATIVE_TRUNK_DIR""/mingw_lib.zip" || return 1
    mv mingw_lib/* .          	     	       || return 1
    rmdir mingw_lib           	     	       || return 1

    # musics
    mkdir -p Textures/Musics         || return 1
    cp ../"$RELATIVE_TRUNK_DIR"/bin/Textures/Musics/*.ogg Textures/Musics || return 1

    # fonts
    mkdir -p Textures/Fonts          || return 1
    cp ../"$RELATIVE_TRUNK_DIR"/bin/Textures/Fonts/*.ttf Textures/Fonts || return 1

    # langs
    mkdir locale     || return 1
    for PO in ../"$RELATIVE_TRUNK_DIR"/po/*.gmo
      do
      POLANG="`basename "$PO" | sed -e s+"\(\.[^\.]*\)$"+""+`"
      mkdir -p "locale/""$POLANG""/LC_MESSAGES"            || return 1
      cp "$PO" "locale/""$POLANG""/LC_MESSAGES""/xmoto.mo" || return 1
    done

    # extra files
    cp ../"$RELATIVE_TRUNK_DIR"/README    README.txt    || return 1
    unix2dos        README.txt                 || return 1
    cp ../"$RELATIVE_TRUNK_DIR"/COPYING   COPYING.txt   || return 1
    unix2dos        COPYING.txt                || return 1
    cp ../"$RELATIVE_TRUNK_DIR"/ChangeLog ChangeLog.txt || return 1
    unix2dos        ChangeLog.txt              || return 1
    )
}

function make_zip {
    ZIPDIR="$1"
    ZIPFILE="$2"

    if test -e "$ZIPFILE"
	then
	rm "$ZIPFILE" || return 1
    fi

    zip -q -r "$ZIPFILE" "$ZIPDIR" || return 1
}

function buildSetup {
  DIR="$1"
  VERSION="$2"
  NSI_FILE="xmoto.nsi"

  makeNSI "$NSI_FILE" "$VERSION" > "$DIR""/""$NSI_FILE" || return 1
  (
  cd "$DIR" || return 1
  makensis "$NSI_FILE" > /dev/null || return 1
  ) || return 1
  mv "$DIR""/""xmoto-""$VERSION""-win32-setup.exe" . || return 1
}

RELATIVE_TRUNK_DIR="`dirname "$0"`"
if echo "$RELATIVE_TRUNK_DIR" | grep -E "^/"
    then
    echo "$0"" must be called with a relative path" 1>&2
    exit 1
fi

VERSION=`getVersion "$RELATIVE_TRUNK_DIR"`
ZIPDIR="xmoto-""$VERSION""-win32"
ZIPFILE="$ZIPDIR"".zip"

if ! prepare_tmp_directory "$ZIPDIR"
then
    echo "Unable to make the tmp directory" 1>&2
    exit 1
fi

if ! fill_tmp_directory "$ZIPDIR" "$RELATIVE_TRUNK_DIR"
then
    rm -rf "$ZIPDIR"
    echo "Unable to get required files" 1>&2
    exit 1
fi

if ! make_zip "$ZIPDIR" "$ZIPFILE"
then
    rm -rf "$ZIPDIR"
    echo "Unable to make the zip" 1>&2
    exit 1    
fi

if ! buildSetup "$ZIPDIR" "$VERSION"
then
    rm -rf "$ZIPDIR"
    echo "Unable to make setup.exe" 1>&2
    exit 1  
fi

rm -rf "$ZIPDIR"
echo "Succes"
