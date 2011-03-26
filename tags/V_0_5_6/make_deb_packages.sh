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

# make gzip package
if ! make dist-gzip
then
    exit 1
fi

#
VERSION="`getVersion .`"
TARFILE="xmoto-""$VERSION"".tar.gz"

# get tar
if ! cp "$TARFILE" "$TMPDIR""/""$TARFILE"
then
    exit 1
fi

# untar
if ! tar zxf "$TARFILE" -C "$TMPDIR"
then
    exit 1
fi

XDIR="xmoto-""$VERSION"

# add changelog
CHLOG="$TMPDIR""/""$XDIR""/""debian/changelog"
CHLOG_TMP="$CHLOG"".tmp"
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

if ! (cd "$TMPDIR""/""$XDIR" && debuild --no-tgz-check)
then
    exit 1
fi

find "$TMPDIR" -maxdepth 1 -name "*.deb"

exit 0
