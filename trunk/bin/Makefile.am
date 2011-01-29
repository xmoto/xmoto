pkgdatadir = $(datadir)/@PACKAGE@
pkgdata_DATA = xmoto.bin

pkgMusicsdir = $(pkgdatadir)/Textures/Musics
pkgMusics_DATA = Textures/Musics/menu.ogg Textures/Musics/ridealong.ogg Textures/Musics/batcave.ogg Textures/Musics/legolodio.ogg Textures/Musics/MadeiraStew.ogg Textures/Musics/speeditup.ogg Textures/Musics/foxrun.ogg

pkgFontsdir = $(pkgdatadir)/Textures/Fonts
pkgFonts_DATA = Textures/Fonts/DejaVuSans.ttf Textures/Fonts/DejaVuSansMono.ttf

xmoto.bin: package.lst
if CROSS_COMPILING
	echo "Cross-compiling, cannot build xmoto.bin"
else
if WIN32
	echo "Windows, cannot build xmoto.bin in the src directory ; build it from the exec directory."
else
	if test -e $<; then ../src/xmoto --pack $@ `dirname $<`; fi
endif
endif

# nothing to do for package.lst, but this entry exists because this file is not included in releases.
package.lst:
	

EXTRA_DIST = $(pkgdata_DATA) $(pkgMusics_DATA) $(pkgFonts_DATA)
