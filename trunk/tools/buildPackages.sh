#!/bin/bash

BUILD_PATH="/home/nicolas/xmoto_dev/build"

# require a subpath trunk with a svn working directory
# require a subdir sources
# require a subdir windows
# require a subdir builds

function update_svn {
    (
	cd trunk || return 1
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
    # update this po file
    cp trunk/po/remove-potcdate.sed sources/po/remove-potcdate.sed || return 1

    # try to build
    (
	cd sources       &&
	make > /dev/null &&
	make dist-gzip > /dev/null &&
	cp "xmoto-"""`getVersion`""".tar.gz" ../builds
    ) && return 0

    prepare_sources || return 1

    # retry with configure
    (
	cd sources &&
	../trunk/configure > /dev/null &&
	make > /dev/null &&
	make dist-gzip > /dev/null && 
	cp "xmoto-"""`getVersion`""".tar.gz" ../builds
    ) && return 0

    return 1
}

function make_windows {
    (
	cd windows &&
	cp ../sources/bin/xmoto.bin bin/xmoto.bin &&
	make > /dev/null && 
	../trunk/make_windows_package.sh > /dev/null &&
	cp "xmoto-"""`getVersion`"""-win32-setup.exe" ../builds &&
	cp "xmoto-"""`getVersion`"""-win32.zip"       ../builds
    ) && return 0

    # retry with configure
    (
	cd windows &&
	../trunk/configure_mingw_from_linux.sh > /dev/null &&
	make > /dev/null &&
	../trunk/make_windows_package.sh > /dev/null &&
	cp "xmoto-"""`getVersion`"""-win32-setup.exe" ../builds &&
	cp "xmoto-"""`getVersion`"""-win32.zip"       ../builds
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

# make unix package
echo "Make sources package..."
if ! make_sources
    then
    echo "Erreur" >&2
    exit 1
fi

# make windows packages
echo "Building windows packages..."
if ! make_windows
    then
    echo "Erreur" >&2
    exit 1
fi

# finally, fix the last svn build version
mv "SVNVERSION" "SVNVERSION_PREVIOUS" || return 1

echo "Success"
