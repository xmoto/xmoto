
WANTED_WARNNING= -w
#WANTED_WARNNING= -Wall
#WANTED_WARNNING= -pedantic -Wcast-align -Wcast-qual -Wchar-subscripts -Wcomment -Wconversion -Wfloat-equal -Wformat -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wimplicit -Wimport -Wuninitialized -Wlong-long -Wmissing-braces -Wmissing-format-attribute -Wmissing-noreturn -Wpacked -Wparentheses -Wpointer-arith -Wredundant-decls -Wreturn-type -Wshadow -Wswitch  -Wswitch-default  -Wswitch-enum -Wtrigraphs -Wundef -Wunknown-pragmas -Wunused -Wunused-function -Wunused-label -Wunused-value -Wunused-variable -Wwrite-strings
# -Wunreachable-code

AM_CXXFLAGS = -DNOMMGR $(WANTED_WARNNING) -DGAMEDATADIR=\"$(GAMEDATADIR)\" -DLOCALESDIR=\"$(LOCALESDIR)\" -DALLOW_WEBACCESS=$(ALLOW_WEBACCESS) -DALLOW_ZOOMING=$(ALLOW_ZOOMING) $(GL_CFLAGS) $(SDL_CFLAGS) -DUSE_OPENGL=$(USE_OPENGL) -DUSE_SDLGFX=$(USE_SDLGFX) -DUSE_SDLGFXTEXTURED=$(USE_SDLGFXTEXTURED)
AM_CFLAGS = -DNOMMGR $(WANTED_WARNNING) -DGAMEDATADIR=\"$(GAMEDATADIR)\" -DLOCALESDIR=\"$(LOCALESDIR)\" -DALLOW_WEBACCESS=$(ALLOW_WEBACCESS) -DALLOW_ZOOMING=$(ALLOW_ZOOMING) $(GL_CFLAGS) $(SDL_CFLAGS) -DUSE_OPENGL=$(USE_OPENGL) -DUSE_SDLGFX=$(USE_SDLGFX) -DUSE_SDLGFXTEXTURED=$(USE_SDLGFXTEXTURED)
# AM_LDFLAGS = -s

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
                  helpers/HighPrecisionTimer.h

common_SOURCES = BuiltInFont.cpp Image.cpp \
  VApp.cpp VBezier.cpp VDraw.cpp VDrawOpenGL.cpp VDrawSDLgfx.cpp \
  VFileIO.cpp VTexture.cpp \
  VXml.cpp \
  $(helpers_SOURCES) \
  $(image_SOURCES) $(tinyxml_SOURCES) \
  $(md5_SOURCES) \
  Image.h \
  VApp.h VBezier.h VCommon.h VDraw.h \
  VFileIO.h VTexture.h VXml.h \
  BuildConfig.h \
  BuiltInFont.h \
  DBuffer.cpp \
  DBuffer.h \
  CRCHash.cpp \
  CRCHash.h \
  Theme.h \
  Theme.cpp \
  WWW.cpp \
  WWW.h \
  WWWAppInterface.h \
  Locales.h \
  Locales.cpp \
  Packager.cpp \
  Packager.h

common_LDADD = $(GL_LIBS) $(SDL_LIBS)

bin_PROGRAMS = xmoto

xmoto_SOURCES = $(common_SOURCES) BSP.cpp Game.cpp GameMain.cpp \
  GameMenus.cpp GUIBestTimes.cpp GUIButton.cpp GUI.cpp \
  GUIFrame.cpp GUIList.cpp GUIStatic.cpp GUITabView.cpp \
  MotoGame.cpp MotoGameL.cpp MPhysics.cpp PlayerData.cpp \
  Renderer.cpp RendererInit.cpp Replay.cpp Sound.cpp \
  UserConfig.cpp \
  BSP.h \
  Game.h \
  GameText.h \
  GUI.h \
  GUIXMoto.h \
  GUIXMoto.cpp \
  MotoGame.h \
  PhysSettings.h \
  PlayerData.h \
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
  ReplayList.cpp \
  Stats.cpp \
  Stats.h \
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
  xmscene/BasicSceneStructs.h \
  xmscene/Level.h \
  xmscene/Level.cpp \
  xmscene/Zone.h \
  xmscene/Zone.cpp \
  xmscene/Block.h   \
  xmscene/Block.cpp \
  xmscene/Entity.h   \
  xmscene/Entity.cpp \
  xmscene/BikeController.h \
  xmscene/BikeController.cpp \
  xmscene/BikeParameters.h \
  xmscene/BikeParameters.cpp \
  xmscene/BikeAnchors.h \
  xmscene/BikeAnchors.cpp \
  xmscene/Bike.h \
  xmscene/Bike.cpp

xmoto_LDADD = $(common_LDADD)
