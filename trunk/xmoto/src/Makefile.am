
WANTED_WARNNING= -pedantic -Wcast-align -Wcast-qual -Wchar-subscripts -Wcomment -Wconversion -Wfloat-equal -Wformat -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wimplicit -Wimport -Wuninitialized -Wlong-long -Wmissing-braces -Wmissing-format-attribute -Wmissing-noreturn -Wpacked -Wparentheses -Wpointer-arith -Wredundant-decls -Wreturn-type -Wshadow -Wswitch  -Wswitch-default  -Wswitch-enum -Wtrigraphs -Wundef -Wunknown-pragmas -Wunused -Wunused-function -Wunused-label -Wunused-value -Wunused-variable -Wwrite-strings
# -Wunreachable-code

AM_CXXFLAGS = -DNOMMGR $(WANTED_WARNNING) -DGAMEDATADIR=\"$(datadir)/@PACKAGE@\" $(GL_CFLAGS)
AM_CFLAGS = -DNOMMGR $(WANTED_WARNNING) -DGAMEDATADIR=\"$(datadir)/@PACKAGE@\" $(GL_CFLAGS)
#AM_LDFLAGS = -s

image_SOURCES = image/tim.cpp image/tim_io_stdio.cpp \
	image/tim_jpeg.cpp image/tim_memory_crt.cpp \
	image/tim_png.cpp image/tim.h

tinyxml_SOURCES = tinyxml/tinystr.cpp tinyxml/tinyxml.cpp \
	tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp \
	tinyxml/tinystr.h tinyxml/tinyxml.h

md5_SOURCES = md5sum/md5.h md5sum/md5.c \
	md5sum/md5file.h md5sum/md5file.cpp

compression_SOURCES = compression/FileCompression.h compression/FileCompression.cpp

arch_SOURCES = arch/SwapEndian.h arch/SwapEndian.cpp

common_SOURCES = BuiltInFont.cpp Image.cpp LevelSrc.cpp \
	VApp.cpp VBezier.cpp VDraw.cpp \
	VDrawText.cpp VFileIO.cpp VMath.cpp VTexture.cpp \
	VXml.cpp \
	$(image_SOURCES) $(tinyxml_SOURCES) \
	$(md5_SOURCES) $(compression_SOURCES) \
	$(arch_SOURCES) \
	Image.h LevelSrc.h \
	VApp.h VBezier.h VCommon.h VDraw.h \
	VExcept.h VFileIO.h VMath.h VTexture.h VXml.h \
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
	WWWAppInterface.h

common_LDADD = $(GL_LIBS) $(SDL_LIBS)

bin_PROGRAMS = xmoto xmoto-edit

xmoto_edit_SOURCES = $(common_SOURCES) Editor.cpp EditorMain.cpp \
	EditorData.cpp EditorLog.cpp Editor.h EditorData.h \
	EditorLog.h Packager.cpp Packager.h

xmoto_edit_LDADD = $(common_LDADD)

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
	SomersaultCounter.cpp

xmoto_LDADD = $(common_LDADD)