pkgdatadir = $(datadir)/@PACKAGE@

pkgdata_DATA = editor.dat fonts.dat xmoto.bin xmoto.ogg credits.rpl

xmoto.bin: package.lst
if CROSS_COMPILING
	echo "Cross-compiling, cannot build xmoto.bin"
else
	if test -e package.lst; then ../src/xmoto -pack; fi
endif

# nothing to do for package.lst, but this entry exists because this file is not included in releases.
package.lst:
	

EXTRA_DIST = $(pkgdata_DATA)



