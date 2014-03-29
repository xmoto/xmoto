#!/bin/bash

BUILD_PATH="/home/nicolas/xm/build"

# require a subpath trunk with a svn working directory
# require a subdir sources
# require a subdir windows
# require a subdir builds

function svnVersion {
    (
	cd trunk   || return 1
	svnversion || return 1
    ) || return 1
}

function update_svn {
    (
	cd trunk || return 1
	svn -q status | while read STATUS FILE; do svn revert "$FILE"; done
	svn up   || return 1

	# check that svn version is only a number
	SVNVERSION="`svnversion`"
	echo "$SVNVERSION" > ../SVNVERSION || return 1
	echo "$SVNVERSION" | grep -E "^[0-9]*\$" >/dev/null || return 1
    ) || return 1
}

function prepare_sources {
    (cd trunk && rm -rf src/.deps && sh ./bootstrap) || return 1
}

function getVersion {
    grep -E "^VERSION = " Makefile | sed -e s+"VERSION = "+""+
}

function make_sources {
    SVNVERSION="$1"

    # try to build
    (
	cd sources       &&
	make > /dev/null &&
	make dist-gzip > /dev/null &&
	cp "xmoto-""`getVersion`"".tar.gz" ../builds/"xmoto-""`getVersion`""-src-svn""$SVNVERSION"".tar.gz"
    ) && return 0

    prepare_sources || return 1

    # retry with configure
    (
	cd sources &&
	../trunk/configure > /dev/null &&
	make > /dev/null &&
	make dist-gzip > /dev/null && 
	cp "xmoto-""`getVersion`"".tar.gz" ../builds/"xmoto-""`getVersion`""-src-svn""$SVNVERSION"".tar.gz"
    ) && return 0

    return 1
}

function make_deb {
    (
	cd sources       &&
	make dist-gzip > /dev/null &&
	../trunk/make_deb_packages.sh &&
	mv *.deb "../builds/"
    ) && return 0
}

function make_windows {
    SVNVERSION="$1"

    (
	cd windows &&
	cp ../sources/bin/xmoto.bin bin/xmoto.bin &&
	make > /dev/null && 
	../trunk/make_windows_package.sh > /dev/null &&
	cp "xmoto-""`getVersion`""-win32-setup.exe" ../builds/"xmoto-""`getVersion`""-win32-setup-svn""$SVNVERSION"".exe" &&
	cp "xmoto-""`getVersion`""-win32.zip"       ../builds/"xmoto-""`getVersion`""-win32-svn""$SVNVERSION"".zip"
    ) && return 0

    # retry with configure
    (
	cd windows &&
	../trunk/configure_mingw_from_linux.sh > /dev/null &&
	make > /dev/null &&
	../trunk/make_windows_package.sh > /dev/null &&
	cp "xmoto-""`getVersion`""-win32-setup.exe" ../builds/"xmoto-""`getVersion`""-win32-setup-svn""$SVNVERSION"".exe" &&
	cp "xmoto-""`getVersion`""-win32.zip"       ../builds/"xmoto-""`getVersion`""-win32-svn""$SVNVERSION"".zip"
    ) && return 0

    return 1
}

# change dir
if ! cd "$BUILD_PATH"
    then
    echo "Erreur" >&2
    exit 1
fi

# update svn
echo "Updating svn..."
if ! update_svn
    then
    echo "Erreur" >&2
    exit 1
fi

# don't build if build version is the same
if test -f "SVNVERSION_PREVIOUS"
then
    if test -z "`diff SVNVERSION SVNVERSION_PREVIOUS`"
	then
	echo "Meme version"
	exit 0
    fi
fi

# remove previous build
rm -rf builds || return 1
mkdir builds  || return 1

SVNVERSION="`svnVersion`"

# make unix package
echo "Make sources package..."
if ! make_sources "$SVNVERSION"
    then
    echo "Erreur" >&2
    exit 1
fi

# make windows packages
echo "Building windows packages..."
if ! make_windows "$SVNVERSION"
    then
    echo "Erreur" >&2
    exit 1
fi

# make unix package
echo "Make deb packages..."
if ! make_deb
    then
    echo "Erreur" >&2
    exit 1
fi

# finally, fix the last svn build version
mv "SVNVERSION" "SVNVERSION_PREVIOUS" || return 1

echo "Success"
