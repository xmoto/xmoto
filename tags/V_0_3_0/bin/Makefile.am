pkgdatadir = $(datadir)/@PACKAGE@
pkgdata_DATA = xmoto.bin

pkgMusicsdir = $(pkgdatadir)/Textures/Musics
pkgMusics_DATA = Textures/Musics/menu.ogg Textures/Musics/ridealong.ogg Textures/Musics/batcave.ogg

pkgFontsdir = $(pkgdatadir)/Textures/Fonts
pkgFonts_DATA = Textures/Fonts/DejaVuSans.ttf

xmoto.bin: package.lst
if CROSS_COMPILING
	echo "Cross-compiling, cannot build xmoto.bin"
else
	if test -e package.lst; then ../src/xmoto -pack; fi
endif

# nothing to do for package.lst, but this entry exists because this file is not included in releases.
package.lst:
	

EXTRA_DIST = $(pkgdata_DATA) $(pkgMusics_DATA) $(pkgFonts_DATA)



