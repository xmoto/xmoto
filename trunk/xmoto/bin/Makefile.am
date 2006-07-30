pkgdatadir = $(datadir)/@PACKAGE@

pkgdata_DATA = editor.dat fonts.dat xmoto.bin xmoto.ogg

xmoto.bin: package.lst
	if test -e package.lst; then ../src/xmoto-edit -pack; fi

# nothing to do for package.lst, but this entry exists because this file is not included in releases.
package.lst:
	

EXTRA_DIST = $(pkgdata_DATA)



