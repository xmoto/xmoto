pkgdatadir = $(datadir)/@PACKAGE@

pkgdata_DATA = editor.dat fonts.dat xmoto.bin xmoto.ogg

xmoto.bin: package.lst ../xmoto-edit
	../xmoto-edit -pack

EXTRA_DIST = $(pkgdata_DATA)



