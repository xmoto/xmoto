pkgdatadir = $(datadir)/@PACKAGE@

pkgdata_DATA = editor.dat fonts.dat xmoto.bin xmoto.ogg

xmoto.bin: ../src/xmoto-edit
	if test -e package.lst; then ../src/xmoto-edit -pack; fi

EXTRA_DIST = $(pkgdata_DATA)



