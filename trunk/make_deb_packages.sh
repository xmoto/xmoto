#!/bin/bash

getVersion() {
    RELATIVE_TRUNK_DIR="$1"

    if ! test -e ./"$RELATIVE_TRUNK_DIR"/configure.in
	then
	return 1
    fi

    grep "AM_INIT_AUTOMAKE" ./"$RELATIVE_TRUNK_DIR"/configure.in |
    sed -e s+".*AM_INIT_AUTOMAKE(\(.*\),\(.*\)).*"+"\\2"+
}

getSvnVersion() {
    RELATIVE_TRUNK_DIR="$1"

    if ! svnversion "$RELATIVE_TRUNK_DIR"
    then
	return 1
    fi
}

getDistribution() {
    grep -E '^DISTRIB_CODENAME=' /etc/lsb-release | sed -e s+'^DISTRIB_CODENAME='++
}

TMPDIR="debian_tmp"
VERSION=$(getVersion .)
SVN=$(getSvnVersion .)
SVN=3363
DISTRIBUTION=$(getDistribution)
XDIR="xmoto-""$VERSION"
TARFILE="xmoto-""$VERSION"".tar.gz"

# check that the svn version is only an integer
if ! echo "$SVN" | grep -qE '^[1-9][0-9]*$'
    then
    echo "Svn version is not an integer" >&2
    exit 1
fi

# check the distribution
if test -z "$DISTRIBUTION"
then
    echo "Disribution not found" >&2
    exit 1
fi

# temporary directory
if test -e "$TMPDIR"
then
    rm -rf "$TMPDIR"
fi

if ! mkdir "$TMPDIR"
then
    exit 1
fi

# make gzip package
echo "make dist-gzip"
if ! make dist-gzip > /dev/null
then
    exit 1
fi

# get tar (needed as orig.tar)
echo "cp ""$TARFILE"" ""$TMPDIR""/""$TARFILE"
if ! cp "$TARFILE" "$TMPDIR""/""$TARFILE"
then
    exit 1
fi

# untar
echo "tar zxf "$TMPDIR""/"""$TARFILE"" -C ""$TMPDIR"
if ! tar zxf "$TARFILE" -C "$TMPDIR"
then
    exit 1
fi

# add debian directory
echo "cp -r debian ""$TMPDIR""/""$XDIR"
if ! cp -r debian "$TMPDIR""/""$XDIR"
then
    exit 1
fi

# changelog
if ! (cd "$TMPDIR""/""$XDIR" && dch --create --newversion "$VERSION"~"$SVN" --package xmoto --distribution "$DISTRIBUTION" "svn version")
then
    exit 1
fi

# debuild
if ! (cd "$TMPDIR""/""$XDIR" && debuild --no-tgz-check)
then
    exit 1
fi

find "$TMPDIR" -maxdepth 1 -name "*.deb"

exit 0
