pkgdatadir = $(datadir)/@PACKAGE@

pkgdata_DATA = editor.dat fonts.dat xmoto.bin xmoto.ogg

xmoto.bin: package.lst ../src/xmoto-edit
	../src/xmoto-edit -pack

EXTRA_DIST = $(pkgdata_DATA)



