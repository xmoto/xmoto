
WANTED_WARNNING= -w
#WANTED_WARNNING= -Wall
#WANTED_WARNNING= -pedantic -Wcast-align -Wcast-qual -Wchar-subscripts -Wcomment -Wconversion -Wfloat-equal -Wformat -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wimplicit -Wimport -Wuninitialized -Wlong-long -Wmissing-braces -Wmissing-format-attribute -Wmissing-noreturn -Wpacked -Wparentheses -Wpointer-arith -Wredundant-decls -Wreturn-type -Wshadow -Wswitch  -Wswitch-default  -Wswitch-enum -Wtrigraphs -Wundef -Wunknown-pragmas -Wunused -Wunused-function -Wunused-label -Wunused-value -Wunused-variable -Wwrite-strings
# -Wunreachable-code

SVNDEF = -D'SVN_REV="$(shell if test -d .svn; then svnversion -n .; fi)"'

AM_CXXFLAGS = -DNOMMGR $(WANTED_WARNNING) -DGAMEDATADIR=\"$(GAMEDATADIR)\" -DLOCALESDIR=\"$(LOCALESDIR)\" -DALLOW_ZOOMING=$(ALLOW_ZOOMING) $(GL_CFLAGS) $(SDL_CFLAGS) -DUSE_OPENGL=$(USE_OPENGL) -DUSE_SDLGFX=$(USE_SDLGFX) $(SVNDEF)
AM_CFLAGS = -DNOMMGR $(WANTED_WARNNING) -DGAMEDATADIR=\"$(GAMEDATADIR)\" -DLOCALESDIR=\"$(LOCALESDIR)\" -DALLOW_ZOOMING=$(ALLOW_ZOOMING) $(GL_CFLAGS) $(SDL_CFLAGS) -DUSE_OPENGL=$(USE_OPENGL) -DUSE_SDLGFX=$(USE_SDLGFX) $(SVNDEF)
# AM_LDFLAGS = -s

#AM_LDFLAGS=

image_SOURCES = image/tim.cpp image/tim_io_stdio.cpp \
  image/tim_jpeg.cpp image/tim_memory_crt.cpp \
  image/tim_png.cpp image/tim.h

tinyxml_SOURCES = tinyxml/tinystr.cpp tinyxml/tinyxml.cpp \
  tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp \
  tinyxml/tinystr.h tinyxml/tinyxml.h

md5_SOURCES = md5sum/md5.h md5sum/md5.c \
  md5sum/md5file.h md5sum/md5file.cpp

helpers_SOURCES = helpers/VMath.h             \
                  helpers/VMath.cpp           \
                  helpers/VExcept.h           \
                  helpers/SwapEndian.h        \
                  helpers/SwapEndian.cpp      \
                  helpers/FileCompression.h   \
                  helpers/FileCompression.cpp \
                  helpers/Color.h             \
                  helpers/Color.cpp           \
                  helpers/HighPrecisionTimer.h \
		  helpers/utf8.h   \
		  helpers/utf8.cpp \
                  iqsort.h

common_SOURCES = Image.cpp \
  VApp.cpp VBezier.cpp VDraw.cpp VDrawOpenGL.cpp PolyDraw.h PolyDraw.cpp VDrawSDLgfx.cpp \
  VFileIO.cpp VTexture.cpp \
  VXml.cpp \
  $(helpers_SOURCES) \
  $(image_SOURCES) $(tinyxml_SOURCES) \
  $(md5_SOURCES) \
  Image.h \
  VApp.h VBezier.h VCommon.h VDraw.h \
  VFileIO.h VTexture.h VXml.h \
  BuildConfig.h \
  DBuffer.cpp \
  DBuffer.h \
  CRCHash.cpp \
  CRCHash.h \
  Theme.h \
  Theme.cpp \
  WWW.cpp \
  WWW.h \
  WWWAppInterface.h \
  XMotoLoadLevelsInterface.h \
  Locales.h \
  Locales.cpp \
  Packager.cpp \
  Packager.h \
  svn_version.h \
  svn_version.cpp

bz2_SOURCES = bzip2/blocksort.c     \
              bzip2/bzlib.c         \
              bzip2/bzlib.h         \
              bzip2/bzlib_private.h \
              bzip2/compress.c      \
              bzip2/crctable.c      \
              bzip2/decompress.c    \
              bzip2/huffman.c       \
              bzip2/randtable.c

if WIN32
common_SOURCES += $(bz2_SOURCES)
endif

common_LDADD = $(GL_LIBS) $(SDL_LIBS) $(LIBINTL)
if WIN32
common_LDADD += $(MINGW32_LIBS)
endif

bin_PROGRAMS = xmoto

xmoto_SOURCES = $(common_SOURCES) BSP.cpp Game.cpp GameMain.cpp \
  GameMenus.cpp GUIBestTimes.cpp GUIButton.cpp GUIButtonDrawn.cpp GUI.cpp \
  GUIFrame.cpp GUIList.cpp GUIStatic.cpp GUITabView.cpp \
  Renderer.cpp RendererInit.cpp Replay.cpp Sound.cpp \
  LuaLibGame.h LuaLibGame.cpp \
  UserConfig.cpp \
  BSP.h \
  Game.h \
  GameText.h \
  LevelsText.h \
  GUI.h \
  GUIXMoto.h \
  GUIXMoto.cpp \
  PhysSettings.h \
  Renderer.h \
  Replay.h \
  Sound.h \
  UserConfig.h \
  RendererParticles.cpp \
  RendererBike.cpp \
  Input.cpp \
  Input.h \
  GameSerializer.cpp \
  Collision.cpp \
  Collision.h \
  GUIEdit.cpp \
  RendererFBO.cpp \
  glext.h \
  ScriptDynamicObjects.h \
  ScriptDynamicObjects.cpp \
  SomersaultCounter.h \
  SomersaultCounter.cpp \
  GameEvents.h \
  GameEvents.cpp \
  Credits.h \
  Credits.cpp \
  GameInit.cpp \
  GameDrawFrame.cpp \
  LevelsManager.h \
  LevelsManager.cpp \
  SysMessage.h \
  SysMessage.cpp \
  db/xmDatabase.h \
  db/xmDatabase.cpp \
  db/xmDatabase_stats.cpp \
  db/xmDatabase_levels.cpp \
  db/xmDatabase_profiles.cpp \
  db/xmDatabase_replays.cpp \
  db/xmDatabase_web.cpp \
  db/xmDatabase_themes.cpp \
  db/xmDatabaseUpdateInterface.h \
  xmscene/BasicSceneStructs.h \
  xmscene/Scene.h \
  xmscene/Scene.cpp \
  xmscene/Level.h \
  xmscene/Level.cpp \
  xmscene/Zone.h \
  xmscene/Zone.cpp \
  xmscene/Block.h   \
  xmscene/Block.cpp \
  xmscene/Entity.h   \
  xmscene/Entity.cpp \
  xmscene/SkyApparence.h   \
  xmscene/SkyApparence.cpp \
  xmscene/BikeController.h \
  xmscene/BikeController.cpp \
  xmscene/BikeParameters.h \
  xmscene/BikeParameters.cpp \
  xmscene/BikeAnchors.h \
  xmscene/BikeAnchors.cpp \
  xmscene/Bike.h        \
  xmscene/Bike.cpp      \
  xmscene/BikeGhost.h   \
  xmscene/BikeGhost.cpp \
  xmscene/BikePlayer.h  \
  xmscene/BikePlayer.cpp \
  xmscene/Camera.h \
  xmscene/Camera.cpp

if WIN32
xmoto_SOURCES += xmoto_icone.ico
.ico.o:
	echo "1 ICON \"""$<""\"" > tmp.rc && i586-mingw32msvc-windres tmp.rc "$@"
endif

xmoto_LDADD = $(common_LDADD)

icndatadir = $(datadir)/@PACKAGE@
icndata_DATA = xmoto_icone_x.ico

EXTRA_DIST = $(icndata_DATA)

all:
	touch svn_version.cpp
