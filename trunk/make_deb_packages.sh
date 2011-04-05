#!/bin/bash

TMPDIR="debian_tmp"

getVersion() {
    RELATIVE_TRUNK_DIR="$1"
    
    if ! test -e ./"$RELATIVE_TRUNK_DIR"/configure.in
    then
	return 1
    fi

    grep "AM_INIT_AUTOMAKE" ./"$RELATIVE_TRUNK_DIR"/configure.in |
    sed -e s+".*AM_INIT_AUTOMAKE(\(.*\),\(.*\)).*"+"\\2"+
}

getLog() {
    VERSION="$1"
    DATE="`date -R`"

    cat  <<EOF
xmoto (${VERSION}~svn-1) unstable; urgency=low

  * Svn version.

 -- X-Moto <xmoto@tuxfamily.org>  ${DATE}

EOF

}

if test -e "$TMPDIR"
then
    rm -rf "$TMPDIR"
fi

if ! mkdir "$TMPDIR"
then
    exit 1
fi

#
VERSION="`getVersion .`"
XDIR="xmoto-""$VERSION"
TARFILE="xmoto-""$VERSION"".tar.gz"
CHLOG="$TMPDIR""/""$XDIR""/""debian/changelog"
CHLOG_TMP="$CHLOG"".tmp"

# make gzip package
echo "make dist-gzip"
if ! make dist-gzip > /dev/null
then
    exit 1
fi

# get tar
echo "cp ""$TARFILE"" ""$TMPDIR""/""$TARFILE"
if ! cp "$TARFILE" "$TMPDIR""/""$TARFILE"
then
    exit 1
fi

# untar
echo "tar zxf ""$TARFILE"" -C ""$TMPDIR"
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

# add changelog
echo "create changelog"
if ! getLog "$VERSION" > "$CHLOG_TMP"
then
    exit 1
fi

if ! cat "$CHLOG" >> "$CHLOG_TMP"
then
    exit 1
fi

if ! mv "$CHLOG_TMP" "$CHLOG"
then
    exit 1
fi

# debuild
echo debuild
if ! (cd "$TMPDIR""/""$XDIR" && debuild --no-tgz-check)
then
    exit 1
fi

find "$TMPDIR" -maxdepth 1 -name "*.deb"

exit 0
