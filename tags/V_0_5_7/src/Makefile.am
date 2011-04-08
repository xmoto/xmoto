WANTED_WARNNING= -Wall

SVNDEF = -D'SVN_REV="$(shell if test -d $(srcdir)/.svn; then svnversion -n $(srcdir); fi)"'

XM_FLAGS = -DHAVEDGETCONFIGURATION=$(HAVEDGETCONFIGURATION) -DNOMMGR $(WANTED_WARNNING) -DGAMEDATADIR=\"$(GAMEDATADIR)\" -DLOCALESDIR=\"$(LOCALESDIR)\" -DASIAN_TTF_FILE=\"$(ASIAN_TTF_FILE)\" -DALLOW_DEV=$(ALLOW_DEV) -DCONFIGURE_DEFAULT_THEME=\"$(CONFIGURE_DEFAULT_THEME)\" $(GL_CFLAGS) $(SDL_CFLAGS) -DUSE_OPENGL=$(USE_OPENGL) -DUSE_SDLGFX=$(USE_SDLGFX) $(SVNDEF)

AM_CFLAGS =   $(CFLAGS)   $(XM_FLAGS) 
AM_CXXFLAGS = $(CXXFLAGS) $(XM_FLAGS) 

xminclude_SOURCES = include/xm_endian.h    \
           	    include/xm_hashmap.h   \
           	    include/xm_lua.h       \
           	    include/xm_ode.h       \
           	    include/xm_OpenGL.h    \
           	    include/xm_SDL.h       \
           	    include/xm_SDL_mixer.h \
           	    include/xm_SDL_net.h   \
           	    include/xm_SDL_ttf.h


image_SOURCES = image/tim.cpp image/tim_io_stdio.cpp \
  image/tim_jpeg.cpp image/tim_memory_crt.cpp \
  image/tim_png.cpp image/tim.h

tinyxml_SOURCES = tinyxml/tinystr.cpp tinyxml/tinyxml.cpp \
  tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp \
  tinyxml/tinystr.h tinyxml/tinyxml.h

md5_SOURCES = md5sum/md5.h md5sum/md5.c \
  md5sum/md5file.h md5sum/md5file.cpp

gui_basic_SOURCES = gui/basic/GUI.h               \
		    gui/basic/GUI.cpp             \
		    gui/basic/GUIButton.cpp       \
		    gui/basic/GUIButtonDrawn.cpp  \
		    gui/basic/GUIEdit.cpp         \
		    gui/basic/GUIFrame.cpp        \
		    gui/basic/GUIList.cpp         \
		    gui/basic/GUIStatic.cpp       \
		    gui/basic/GUITabView.cpp      \
		    gui/basic/GUIConsole.h        \
		    gui/basic/GUIConsole.cpp

gui_specific_SOURCES = gui/specific/GUIXMoto.h   \
                       gui/specific/GUIXMoto.cpp

drawlib_SOURCES = drawlib/DrawLib.h   \
                  drawlib/DrawLib.cpp

if USE_OPENGL
drawlib_SOURCES += drawlib/DrawLibOpenGL.h   \
                   drawlib/DrawLibOpenGL.cpp
endif

if USE_SDLGFX
drawlib_SOURCES += drawlib/DrawLibSDLgfx.h   \
                   drawlib/DrawLibSDLgfx.cpp
endif

chipmunk_SOURCES= chipmunk/chipmunk.c \
                  chipmunk/chipmunk.h \
                  chipmunk/cpArbiter.c \
                  chipmunk/cpArbiter.h \
                  chipmunk/cpArray.c \
                  chipmunk/cpArray.h \
                  chipmunk/cpBB.c \
                  chipmunk/cpBB.h \
                  chipmunk/cpBody.c \
                  chipmunk/cpBody.h \
                  chipmunk/cpCollision.c \
                  chipmunk/cpCollision.h \
                  chipmunk/cpHashSet.c \
                  chipmunk/cpHashSet.h \
                  chipmunk/cpJoint.c \
                  chipmunk/cpJoint.h \
                  chipmunk/cpPolyShape.c \
                  chipmunk/cpPolyShape.h \
                  chipmunk/cpShape.c \
                  chipmunk/cpShape.h \
                  chipmunk/cpSpace.c \
                  chipmunk/cpSpace.h \
                  chipmunk/cpSpaceHash.c \
                  chipmunk/cpSpaceHash.h \
                  chipmunk/cpVect.c \
                  chipmunk/cpVect.h \
                  chipmunk/prime.h

helpers_SOURCES = helpers/VMath.h             \
                  helpers/VMath.cpp           \
                  helpers/VExcept.h           \
                  helpers/SwapEndian.h        \
                  helpers/SwapEndian.cpp      \
                  helpers/FileCompression.h   \
                  helpers/FileCompression.cpp \
                  helpers/Color.h             \
                  helpers/HighPrecisionTimer.h \
		  helpers/utf8.h   \
		  helpers/utf8.cpp \
		  helpers/Text.h   \
		  helpers/Text.cpp \
                  helpers/Environment.h \
                  helpers/Environment.cpp \
		  helpers/Log.h \
		  helpers/Log.cpp \
		  helpers/System.h \
		  helpers/System.cpp \
                  iqsort.h \
		  helpers/Singleton.h \
		  helpers/MultiSingleton.h \
		  helpers/TFunctor.h \
		  helpers/Random.h \
		  helpers/Random.cpp \
		  helpers/CmdArgumentParser.h \
		  helpers/CmdArgumentParser.cpp \
		  helpers/RenderSurface.h \
		  helpers/RenderSurface.cpp

common_SOURCES = Image.cpp \
  VBezier.cpp PolyDraw.h PolyDraw.cpp \
  XMArgs.h XMArgs.cpp XMSession_default.h XMSession_default.cpp XMSession.h XMSession.cpp \
  XMBuild.h XMBuild.cpp XMBuild_info.h \
  VFileIO_types.h VFileIO.cpp VTexture.cpp \
  VXml.cpp \
  $(xminclude_SOURCES) \
  $(helpers_SOURCES) \
  $(image_SOURCES) $(tinyxml_SOURCES) \
  $(md5_SOURCES) \
  $(chipmunk_SOURCES) \
  Image.h \
  VBezier.h VCommon.h \
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
  XMotoLoadReplaysInterface.h \
  Locales.h \
  Locales.cpp \
  Languages.h \
  Packager.cpp \
  Packager.h \
  CameraAnimation.h CameraAnimation.cpp \
  svn_version.h \
  svn_version.cpp

states_SOURCES = states/StateManager.h       	  \
                 states/StateManager.cpp     	  \
		 states/GameState.h               \
		 states/GameState.cpp             \
                 states/StateMenu.h          	  \
                 states/StateMenu.cpp        	  \
                 states/StateScene.h          	  \
                 states/StateScene.cpp        	  \
                 states/StateMainMenu.h       	  \
                 states/StateMainMenu.cpp      	  \
                 states/StateOptions.h       	  \
                 states/StateOptions.cpp      	  \
                 states/StateHelp.h          	  \
                 states/StateHelp.cpp        	  \
		 states/StateMessageBoxReceiver.h \
	         states/StateMessageBox.h      	  \
                 states/StateMessageBox.cpp    	  \
	         states/StateServerConsole.h   	  \
                 states/StateServerConsole.cpp    \
                 states/StatePause.h         	  \
                 states/StatePause.cpp       	  \
                 states/StateVote.h         	  \
                 states/StateVote.cpp       	  \
                 states/StateDeadJust.h      	  \
                 states/StateDeadJust.cpp    	  \
                 states/StateDeadMenu.h      	  \
                 states/StateDeadMenu.cpp    	  \
                 states/StateFinished.h      	  \
                 states/StateFinished.cpp    	  \
                 states/StateCreditsMode.h   	  \
                 states/StateCreditsMode.cpp 	  \
                 states/StateEditProfile.h   	  \
                 states/StateEditProfile.cpp 	  \
                 states/StateEditWebConfig.h 	  \
                 states/StateEditWebConfig.cpp   \
                 states/StateLevelInfoViewer.h   \
                 states/StateLevelInfoViewer.cpp \
                 states/StateLevelPackViewer.h   \
                 states/StateLevelPackViewer.cpp \
                 states/StatePlaying.h           \
                 states/StatePlaying.cpp         \
                 states/StatePlayingLocal.h           \
                 states/StatePlayingLocal.cpp         \
                 states/StatePlayingNet.h        \
                 states/StatePlayingNet.cpp      \
                 states/StatePreplaying.h        \
                 states/StatePreplaying.cpp      \
                 states/StatePreplayingGame.h    \
                 states/StatePreplayingGame.cpp  \
                 states/StatePreplayingNet.h    \
                 states/StatePreplayingNet.cpp  \
                 states/StatePreplayingReplay.h  \
                 states/StatePreplayingReplay.cpp  \
                 states/StatePreplayingCredits.h   \
                 states/StatePreplayingCredits.cpp \
                 states/StateReplaying.h         \
                 states/StateReplaying.cpp       \
		 states/AttractMode.cpp          \
		 states/AttractMode.h            \
		 states/StateUpdate.cpp          \
		 states/StateUpdate.h            \
		 states/StateWaiting.cpp          \
		 states/StateWaiting.h            \
		 states/StateUpdateDb.cpp        \
		 states/StateUpdateDb.h          \
		 states/StateUpdateThemesList.cpp\
		 states/StateUpdateThemesList.h  \
		 states/StateUpdateTheme.cpp     \
		 states/StateUpdateTheme.h       \
		 states/StateUpdateRoomsList.cpp \
		 states/StateUpdateRoomsList.h   \
		 states/StateUploadHighscore.cpp \
		 states/StateUploadHighscore.h   \
		 states/StateUploadAllHighscores.cpp \
		 states/StateUploadAllHighscores.h   \
		 states/StateRequestKey.cpp      \
		 states/StateRequestKey.h        \
		 states/StateMultiUpdate.cpp     \
		 states/StateMultiUpdate.h       \
		 states/StateCheckWww.cpp        \
		 states/StateCheckWww.h          \
		 states/StateSendVote.cpp        \
		 states/StateSendVote.h          \
		 states/StateSendReport.cpp        \
		 states/StateSendReport.h          \
		 states/StateSync.cpp            \
		 states/StateSync.h              \
		 states/StateWaitServerInstructions.h   \
		 states/StateWaitServerInstructions.cpp \
		 states/StateUpgradeLevels.cpp   \
		 states/StateUpgradeLevels.h     \
		 states/StateViewHighscore.cpp   \
		 states/StateViewHighscore.h

xdgbasedir_SOURCES = xdgbasedir/include/basedir.h xdgbasedir/include/basedir_fs.h xdgbasedir/src/basedir.c

bz2_SOURCES = bzip2/blocksort.c     \
              bzip2/bzlib.c         \
              bzip2/bzlib.h         \
              bzip2/bzlib_private.h \
              bzip2/compress.c      \
              bzip2/crctable.c      \
              bzip2/decompress.c    \
              bzip2/huffman.c       \
              bzip2/randtable.c

thread_SOURCES = thread/XMThread.h                 \
		 thread/XMThread.cpp               \
		 thread/UpdateDbThread.h           \
		 thread/UpdateDbThread.cpp         \
		 thread/UpdateThemesListThread.h   \
		 thread/UpdateThemesListThread.cpp \
		 thread/UpdateThemeThread.h        \
		 thread/UpdateThemeThread.cpp      \
		 thread/UpdateRoomsListThread.h    \
		 thread/UpdateRoomsListThread.cpp  \
		 thread/UploadHighscoreThread.h    \
		 thread/UploadHighscoreThread.cpp  \
		 thread/UploadAllHighscoresThread.h    \
		 thread/UploadAllHighscoresThread.cpp  \
		 thread/XMThreads.cpp              \
		 thread/XMThreads.h                \
		 thread/CheckWwwThread.cpp         \
		 thread/CheckWwwThread.h           \
		 thread/SendVoteThread.cpp         \
		 thread/SendVoteThread.h           \
		 thread/SendReportThread.cpp         \
		 thread/SendReportThread.h           \
		 thread/SyncThread.cpp             \
		 thread/SyncThread.h               \
		 thread/UpgradeLevelsThread.cpp    \
		 thread/UpgradeLevelsThread.h      \
		 thread/LevelsPacksCountUpdateThread.h   \
		 thread/LevelsPacksCountUpdateThread.cpp \
		 thread/XMThreadStats.h \
		 thread/XMThreadStats.cpp \
		 thread/DownloadReplaysThread.h \
		 thread/DownloadReplaysThread.cpp \
		 net/thread/ServerThread.cpp             \
		 net/thread/ServerThread.h       \
		 net/NetClient.cpp             \
		 net/NetClient.h               \
		 net/NetServer.cpp             \
		 net/NetServer.h               \
		 net/ServerRules.cpp             \
		 net/ServerRules.h               \
		 net/helpers/Net.cpp             \
		 net/helpers/Net.h   \
		 net/BasicStructures.h    \
		 net/NetActions.h    \
		 net/NetActions.cpp  \
		 net/ActionReader.h    \
		 net/ActionReader.cpp \
		 net/extSDL_net.h     \
		 net/extSDL_net.cpp \
		 net/VirtualNetLevelsList.h \
		 net/VirtualNetLevelsList.cpp

if USE_INTERNAL_BZ2
common_SOURCES += $(bz2_SOURCES)
endif

if ! WIN32
if USE_INTERNAL_XDG
common_SOURCES += $(xdgbasedir_SOURCES)
endif
endif

common_SOURCES += $(gui_basic_SOURCES)
common_SOURCES += $(gui_specific_SOURCES)
common_SOURCES += $(drawlib_SOURCES)
common_SOURCES += $(states_SOURCES)
common_SOURCES += $(thread_SOURCES)

common_LDADD = $(GL_LIBS) $(SDL_LIBS) $(LIBINTL)
if WIN32
common_LDADD += $(MINGW32_LIBS)
endif

bin_PROGRAMS = xmoto

xmoto_SOURCES = $(common_SOURCES) BSP.cpp Game.cpp \
  GUIBestTimes.cpp \
  Renderer.cpp Replay.cpp Sound.cpp \
  GeomsManager.h GeomsManager.cpp \
  LuaLibGame.h LuaLibGame.cpp \
  LuaLibBase.h LuaLibBase.cpp \
  UserConfig.cpp \
  BSP.h \
  Game.h \
  GameText.h \
  LevelsText.h \
  PhysSettings.h \
  Renderer.h \
  Replay.h \
  Sound.h \
  UserConfig.h \
  Trainer.cpp \
  Trainer.h \
  Input.cpp \
  Input.h \
  XMKey.cpp \
  XMKey.h \
  VirtualLevelsList.h \
  VirtualLevelsList.cpp \
  VideoRecorder.h \
  VideoRecorder.cpp \
  Collision.cpp \
  Collision.h \
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
  LevelsManager.h \
  LevelsManager.cpp \
  XMDemo.h \
  XMDemo.cpp \
  SysMessage.h \
  SysMessage.cpp \
  sqlqueries.h \
  db/xmDatabase.h \
  db/xmDatabase.cpp \
  db/xmDatabase_stats.cpp \
  db/xmDatabase_levels.cpp \
  db/xmDatabase_profiles.cpp \
  db/xmDatabase_replays.cpp \
  db/xmDatabase_web.cpp \
  db/xmDatabase_srv.cpp \
  db/xmDatabase_themes.cpp \
  db/xmDatabase_config.cpp \
  db/xmDatabase_sync.cpp \
  db/xmDatabase_votes.cpp \
  db/xmDatabase_fixes.cpp \
  db/xmDatabaseUpdateInterface.h \
  Universe.h   \
  Universe.cpp \
  xmscene/BasicSceneStructs.h \
  xmscene/BasicSceneStructs.cpp \
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
  xmscene/Serializer.cpp \
  xmscene/PhysicsSettings.h   \
  xmscene/PhysicsSettings.cpp \
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
  xmscene/GhostTrail.h\
  xmscene/GhostTrail.cpp\
  xmscene/Camera.h \
  xmscene/Camera.cpp \
  xmscene/ChipmunkWorld.cpp \
  xmscene/ChipmunkWorld.h \
	xmscene/ScriptTimer.cpp \
	xmscene/ScriptTimer.h


if WIN32
xmoto_SOURCES += xmoto_icone.ico
.ico.o:
	echo "1 ICON \"""$<""\"" > tmp.rc && i586-mingw32msvc-windres tmp.rc "$@"
endif

xmoto_LDADD = $(common_LDADD)

icndatadir = $(datadir)/@PACKAGE@
icndata_DATA = xmoto_icone_x.ico

EXTRA_DIST = $(icndata_DATA)

# Force svn_version.o to always be remade if necessary
all: svnCheck
	@echo svnCheck

svnCheck:
	if test -d $(srcdir)/.svn; then \
	  if ! test -f svnVersion -a "`cat svnVersion`" = "`svnversion -n $(srcdir)`"; then \
	    svnversion -n $(srcdir) > svnVersion; \
	  fi; \
	fi

svnVersion:
	if test -d $(srcdir)/.svn; then \
	  svnversion -n $(srcdir) > svnVersion; \
	else \
	  echo "exported" > svnVersion; \
	fi

svn_version.$(OBJEXT): svnVersion
