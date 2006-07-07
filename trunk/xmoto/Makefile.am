AM_CXXFLAGS = -DNOMMGR -w -DGAMEDATADIR=\"$(datadir)/@PACKAGE@\" $(GL_CFLAGS)
AM_CFLAGS = -DNOMMGR -w -DGAMEDATADIR=\"$(datadir)/@PACKAGE@\" $(GL_CFLAGS)

image_SOURCES = src/image/tim.cpp src/image/tim_io_stdio.cpp \
	src/image/tim_jpeg.cpp src/image/tim_memory_crt.cpp \
	src/image/tim_png.cpp src/image/tim.h

tinyxml_SOURCES = src/tinyxml/tinystr.cpp src/tinyxml/tinyxml.cpp \
	src/tinyxml/tinyxmlerror.cpp src/tinyxml/tinyxmlparser.cpp \
	src/tinyxml/tinystr.h src/tinyxml/tinyxml.h

md5_SOURCES = src/md5sum/md5.h src/md5sum/md5.c \
	src/md5sum/md5file.h src/md5sum/md5file.cpp

compression_SOURCES = src/compression/FileCompression.h src/compression/FileCompression.cpp

common_SOURCES = src/BuiltInFont.cpp src/Image.cpp src/LevelSrc.cpp \
	src/VApp.cpp src/VBezier.cpp src/VDraw.cpp \
	src/VDrawText.cpp src/VFileIO.cpp src/VMath.cpp src/VTexture.cpp \
	src/VXml.cpp \
	$(image_SOURCES) $(tinyxml_SOURCES) $(md5_SOURCES) $(compression_SOURCES) \
	src/Image.h src/LevelSrc.h \
	src/VApp.h src/VBezier.h src/VCommon.h src/VDraw.h \
	src/VExcept.h src/VFileIO.h src/VMath.h src/VTexture.h src/VXml.h \
	src/BuildConfig.h \
	src/BuiltInFont.h \
	src/DBuffer.cpp \
	src/DBuffer.h \
	src/CRCHash.cpp \
	src/CRCHash.h \
	src/Theme.h \
	src/Theme.cpp

common_LDADD = $(GL_LIBS)

bin_PROGRAMS = xmoto xmoto-edit

xmoto_edit_SOURCES = $(common_SOURCES) src/Editor.cpp src/EditorMain.cpp \
	src/EditorData.cpp src/EditorLog.cpp src/Editor.h src/EditorData.h \
	src/EditorLog.h src/Packager.cpp src/Packager.h

xmoto_edit_LDADD = $(common_LDADD)

xmoto_SOURCES = $(common_SOURCES) src/BSP.cpp src/Game.cpp src/GameMain.cpp \
	src/GameMenus.cpp src/GUIBestTimes.cpp src/GUIButton.cpp src/GUI.cpp \
	src/GUIFrame.cpp src/GUIList.cpp src/GUIStatic.cpp src/GUITabView.cpp \
	src/MotoGame.cpp src/MotoGameL.cpp src/MPhysics.cpp src/PlayerData.cpp \
	src/Renderer.cpp src/RendererInit.cpp src/Replay.cpp src/Sound.cpp \
	src/UserConfig.cpp \
	src/BSP.h \
	src/Game.h \
	src/GameText.h \
	src/GUI.h \
	src/GUIXMoto.h \
	src/GUIXMoto.cpp \
	src/MotoGame.h \
	src/PhysSettings.h \
	src/PlayerData.h \
	src/Renderer.h \
	src/Replay.h \
	src/Sound.h \
	src/UserConfig.h \
	src/RendererParticles.cpp \
	src/RendererBike.cpp \
	src/Input.cpp \
	src/Input.h \
	src/GameSerializer.cpp \
	src/Collision.cpp \
	src/Collision.h \
	src/WWW.cpp \
	src/WWW.h \
	src/WWWAppInterface.h \
	src/GUIEdit.cpp \
	src/RendererFBO.cpp \
	src/glext.h \
	src/ReplayList.cpp \
	src/Stats.cpp \
	src/Stats.h \
	src/ScriptDynamicObjects.h \
	src/ScriptDynamicObjects.cpp

xmoto_LDADD = $(common_LDADD)

SUBDIRS = bin

EXTRA_DIST = bootstrap

uninstall-hook:
	rm -r $(datadir)/@PACKAGE@
