AM_CXXFLAGS = -DNOMMGR -w -DGAMEDATADIR=\"$(datadir)/@PACKAGE@\" $(GL_CFLAGS)
AM_CFLAGS = -DNOMMGR -w -DGAMEDATADIR=\"$(datadir)/@PACKAGE@\" $(GL_CFLAGS)

image_SOURCES = image/tim.cpp image/tim_io_stdio.cpp \
	image/tim_jpeg.cpp image/tim_memory_crt.cpp \
	image/tim_png.cpp image/tim.h

tinyxml_SOURCES = tinyxml/tinystr.cpp tinyxml/tinyxml.cpp \
	tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp \
	tinyxml/tinystr.h tinyxml/tinyxml.h

common_SOURCES = BuiltInFont.cpp Image.cpp LevelSrc.cpp \
	VApp.cpp VBezier.cpp VDraw.cpp \
	VDrawText.cpp VFileIO.cpp VMath.cpp VTexture.cpp \
	VXml.cpp $(image_SOURCES) $(tinyxml_SOURCES) \
	Image.h LevelSrc.h \
	VApp.h VBezier.h VCommon.h VDraw.h \
	VExcept.h VFileIO.h VMath.h VTexture.h VXml.h \
	BuildConfig.h \
	BuiltInFont.h \
	DBuffer.cpp \
	DBuffer.h

common_LDADD = $(GL_LIBS)

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
	GameSerializer.cpp

xmoto_LDADD = $(common_LDADD)
		
