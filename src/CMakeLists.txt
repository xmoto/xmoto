CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)

# cmake -D CMAKE_GENERATOR="Unix Makefiles" ../trunk/src
SET(CMAKE_GENERATOR "Unix Makefiles")
PROJECT(xmoto)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake_modules)

# the name of the target operating system
#SET(CMAKE_SYSTEM_NAME Windows)
#SET(CMAKE_SYSTEM_NAME Linux)

# cross compilation on windows
MESSAGE("System: " ${CMAKE_SYSTEM_NAME})
IF(CMAKE_SYSTEM_NAME MATCHES Windows)

 IF(CMAKE_HOST_UNIX) # cross compiling windows version from unix system
  # which compilers to use for C and C++
  SET(CMAKE_C_COMPILER i586-mingw32msvc-gcc)
  SET(CMAKE_CXX_COMPILER i586-mingw32msvc-g++)
  # here is the target environment located
  SET(CMAKE_FIND_ROOT_PATH  /usr/i586-mingw32msvc)

  # adjust the default behaviour of the FIND_XXX() commands:
  # search headers and libraries in the target environment, search 
  # programs in the host environment
  set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
 ENDIF(CMAKE_HOST_UNIX)

 ADD_DEFINITIONS(-mthreads -MD)
 link_libraries (mingw32 SDLmain SDL opengl32 glu32 -mwindows userenv intl)
ENDIF(CMAKE_SYSTEM_NAME MATCHES Windows)

# find dependencies
FIND_PACKAGE(Lua51 REQUIRED)
ADD_DEFINITIONS(-DCMAKE_LUA_H)
MESSAGE("--    Including from: " ${LUA_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR})
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_mixer REQUIRED)
FIND_PACKAGE(SDL_net REQUIRED)
FIND_PACKAGE(SDL_ttf REQUIRED)

IF(CMAKE_SYSTEM_NAME MATCHES Windows)
 set(bzip2internal_src
     bzip2/crctable.c bzip2/bzlib_private.h bzip2/blocksort.c bzip2/bzlib.c bzip2/decompress.c
     bzip2/huffman.c bzip2/bzlib.h bzip2/compress.c bzip2/randtable.c
 )
ELSE(CMAKE_SYSTEM_NAME MATCHES Windows)
 set(bzip2internal_src)
 FIND_PACKAGE(BZip2 REQUIRED)
ENDIF(CMAKE_SYSTEM_NAME MATCHES Windows)

FIND_PACKAGE(CURL REQUIRED)
FIND_PACKAGE(Gettext)
FIND_PACKAGE(JPEG REQUIRED)
FIND_PACKAGE(LibXml2 REQUIRED)
INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
FIND_PACKAGE(ODE REQUIRED)
FIND_PACKAGE(PNG REQUIRED)
FIND_PACKAGE(SQLITE3 REQUIRED)
FIND_PACKAGE(XDG REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)

SET(DATADIR ${CMAKE_INSTALL_PREFIX}/share)
SET(GAMEDATADIR ${DATADIR}/${CMAKE_PROJECT_NAME})

# include languages
IF(GETTEXT_FOUND)
# GETTEXT_CREATE_TRANSLATIONS(../po/xmoto.pot ../po/fr_FR.po ../po/de_DE.po)
ENDIF(GETTEXT_FOUND)

ADD_DEFINITIONS(-DSVN_REV=\"git\")

# options
#ADD_DEFINITIONS(-O2) # g++ optimisations
ADD_DEFINITIONS(-DGAMEDATADIR=\"${GAMEDATADIR}\")
ADD_DEFINITIONS(-DCONFIGURE_DEFAULT_THEME=\"Modern\")

IF(CMAKE_SYSTEM_NAME MATCHES Windows)
  ADD_DEFINITIONS(-DDEFAULT_ASIAN_TTF_FILE=\"Textures/Fonts/bkai00mp.ttf\")
ELSE(CMAKE_SYSTEM_NAME MATCHES Windows)
  ADD_DEFINITIONS(-DDEFAULT_ASIAN_TTF_FILE=\"/usr/share/fonts/truetype/arphic/bkai00mp.ttf\")
ENDIF(CMAKE_SYSTEM_NAME MATCHES Windows)

set(xmincludes_src
  include/xm_OpenGL.h
  include/xm_SDL.h
  include/xm_SDL_mixer.h
  include/xm_SDL_net.h
  include/xm_SDL_ttf.h
  include/xm_endian.h
  include/xm_hashmap.h
  include/xm_lua.h
  include/xm_ode.h
)

set(image_src
  image/tim.cpp image/tim.h
  image/tim_io_stdio.cpp
  image/tim_jpeg.cpp
  image/tim_memory_crt.cpp
  image/tim_png.cpp
)

set(md5sum_src
  md5sum/md5.c md5sum/md5.h
  md5sum/md5file.cpp md5sum/md5file.h
)

set(gui_src
  gui/basic/GUI.cpp gui/basic/GUI.h
  gui/basic/GUIButton.cpp
  gui/basic/GUIButtonDrawn.cpp
  gui/basic/GUIConsole.cpp gui/basic/GUIConsole.h
  gui/basic/GUIEdit.cpp
  gui/basic/GUIFrame.cpp
  gui/basic/GUIList.cpp
  gui/basic/GUIStatic.cpp
  gui/basic/GUITabView.cpp

  gui/specific/GUIXMoto.cpp gui/specific/GUIXMoto.h
)

set(drawlib_src
  drawlib/DrawLib.cpp drawlib/DrawLib.h

  # Needs proper library integration
  drawlib/DrawLibOpenGL.cpp drawlib/DrawLibOpenGL.h
# drawlib/DrawLibSDLgfx.cpp drawlib/DrawLibSDLgfx.h
)

set(chipmunk_src
  chipmunk/chipmunk.c chipmunk/chipmunk.h
  chipmunk/cpArbiter.c chipmunk/cpArbiter.h
  chipmunk/cpArray.c chipmunk/cpArray.h
  chipmunk/cpBB.c chipmunk/cpBB.h
  chipmunk/cpBody.c chipmunk/cpBody.h
  chipmunk/cpCollision.c chipmunk/cpCollision.h
  chipmunk/cpHashSet.c chipmunk/cpHashSet.h
  chipmunk/cpJoint.c chipmunk/cpJoint.h
  chipmunk/cpPolyShape.c chipmunk/cpPolyShape.h
  chipmunk/cpShape.c chipmunk/cpShape.h
  chipmunk/cpSpace.c chipmunk/cpSpace.h
  chipmunk/cpSpaceHash.c chipmunk/cpSpaceHash.h
  chipmunk/cpVect.c chipmunk/cpVect.h
  chipmunk/prime.h
)

set (helpers_src
  helpers/CmdArgumentParser.cpp helpers/CmdArgumentParser.h
  helpers/Color.h
  helpers/Environment.cpp helpers/Environment.h
  helpers/FileCompression.cpp helpers/FileCompression.h
  helpers/HighPrecisionTimer.h
  helpers/Log.cpp helpers/Log.h
  helpers/MultiSingleton.h
  helpers/Random.cpp helpers/Random.h
  helpers/RenderSurface.cpp helpers/RenderSurface.h
  helpers/Singleton.h
  helpers/SwapEndian.cpp helpers/SwapEndian.h
  helpers/System.cpp helpers/System.h
  helpers/TFunctor.h
  helpers/Text.cpp helpers/Text.h
  helpers/VExcept.h
  helpers/VMath.cpp helpers/VMath.h
  helpers/iqsort.h
  helpers/utf8.cpp helpers/utf8.h
)

set(common_src
  common/BuildConfig.h
  common/CRCHash.cpp
  common/CRCHash.h
  common/CameraAnimation.cpp
  common/CameraAnimation.h
  common/DBuffer.cpp
  common/DBuffer.h
  common/Image.cpp
  common/Image.h
  common/Languages.h
  common/Locales.cpp
  common/Locales.h
  common/Packager.cpp
  common/Packager.h
  common/PolyDraw.cpp
  common/PolyDraw.h
  common/Theme.cpp
  common/Theme.h
  common/VBezier.cpp
  common/VBezier.h
  common/VCommon.h
  common/VFileIO.cpp
  common/VFileIO.h
  common/VFileIO_types.h
  common/VTexture.cpp
  common/VTexture.h
  common/VXml.cpp
  common/VXml.h
  common/WWW.cpp
  common/WWW.h
  common/WWWAppInterface.h
  common/XMArgs.cpp
  common/XMArgs.h
  common/XMBuild.cpp
  common/XMBuild.h
  common/XMBuild_info.h
  common/XMSession.cpp
  common/XMSession.h
  common/XMSession_default.cpp
  common/XMSession_default.h
  common/XMotoLoadLevelsInterface.h
  common/XMotoLoadReplaysInterface.h
  common/svn_version.cpp
  common/svn_version.h
)

set(states_src
  states/AttractMode.cpp states/AttractMode.h
  states/GameState.cpp states/GameState.h
  states/StateCheckWww.cpp states/StateCheckWww.h
  states/StateCreditsMode.cpp states/StateCreditsMode.h
  states/StateDeadJust.cpp states/StateDeadJust.h
  states/StateDeadMenu.cpp states/StateDeadMenu.h
  states/StateEditProfile.cpp states/StateEditProfile.h
  states/StateEditWebConfig.cpp states/StateEditWebConfig.h
  states/StateFinished.cpp states/StateFinished.h
  states/StateHelp.cpp states/StateHelp.h
  states/StateLevelInfoViewer.cpp states/StateLevelInfoViewer.h
  states/StateLevelPackViewer.cpp states/StateLevelPackViewer.h
  states/StateMainMenu.cpp states/StateMainMenu.h
  states/StateManager.cpp states/StateManager.h
  states/StateMenu.cpp states/StateMenu.h
  states/StateMessageBox.cpp states/StateMessageBox.h
  states/StateMessageBoxReceiver.h
  states/StateMultiUpdate.cpp states/StateMultiUpdate.h
  states/StateOptions.cpp states/StateOptions.h
  states/StatePause.cpp states/StatePause.h
  states/StatePlaying.cpp states/StatePlaying.h
  states/StatePlayingLocal.cpp states/StatePlayingLocal.h
  states/StatePlayingNet.cpp states/StatePlayingNet.h
  states/StatePreplaying.cpp states/StatePreplaying.h
  states/StatePreplayingCredits.cpp states/StatePreplayingCredits.h
  states/StatePreplayingGame.cpp states/StatePreplayingGame.h
  states/StatePreplayingNet.cpp states/StatePreplayingNet.h
  states/StatePreplayingReplay.cpp states/StatePreplayingReplay.h
  states/StateReplaying.cpp states/StateReplaying.h
  states/StateRequestKey.cpp states/StateRequestKey.h
  states/StateScene.cpp states/StateScene.h
  states/StateSendReport.cpp states/StateSendReport.h
  states/StateSendVote.cpp states/StateSendVote.h
  states/StateServerConsole.cpp states/StateServerConsole.h
  states/StateSync.cpp states/StateSync.h
  states/StateUpdate.cpp states/StateUpdate.h
  states/StateUpdateDb.cpp states/StateUpdateDb.h
  states/StateUpdateRoomsList.cpp states/StateUpdateRoomsList.h
  states/StateUpdateTheme.cpp states/StateUpdateTheme.h
  states/StateUpdateThemesList.cpp states/StateUpdateThemesList.h
  states/StateUpgradeLevels.cpp states/StateUpgradeLevels.h
  states/StateUploadAllHighscores.cpp states/StateUploadAllHighscores.h
  states/StateUploadHighscore.cpp states/StateUploadHighscore.h
  states/StateViewHighscore.cpp states/StateViewHighscore.h
  states/StateVote.cpp states/StateVote.h
  states/StateWaitServerInstructions.cpp states/StateWaitServerInstructions.h
  states/StateWaiting.cpp states/StateWaiting.h
)

set(xdgbasedir_src
  xdgbasedir/include/basedir.h
  xdgbasedir/include/basedir_fs.h
  xdgbasedir/src/basedir.c
)

set(thread_net_src
  net/ActionReader.cpp net/ActionReader.h
  net/BasicStructures.h
  net/NetActions.cpp net/NetActions.h
  net/NetClient.cpp net/NetClient.h
  net/NetServer.cpp net/NetServer.h
  net/ServerRules.cpp net/ServerRules.h
  net/VirtualNetLevelsList.cpp net/VirtualNetLevelsList.h
  net/extSDL_net.cpp net/extSDL_net.h

  net/helpers/Net.cpp net/helpers/Net.h

  net/thread/ServerThread.cpp net/thread/ServerThread.h
)

set(thread_src
  ${thread_net_src}

  thread/CheckWwwThread.cpp thread/CheckWwwThread.h
  thread/DownloadReplaysThread.cpp thread/DownloadReplaysThread.h
  thread/LevelsPacksCountUpdateThread.cpp thread/LevelsPacksCountUpdateThread.h
  thread/SendReportThread.cpp thread/SendReportThread.h
  thread/SendVoteThread.cpp thread/SendVoteThread.h
  thread/SyncThread.cpp thread/SyncThread.h
  thread/UpdateDbThread.cpp thread/UpdateDbThread.h
  thread/UpdateRoomsListThread.cpp thread/UpdateRoomsListThread.h
  thread/UpdateThemeThread.cpp thread/UpdateThemeThread.h
  thread/UpdateThemesListThread.cpp thread/UpdateThemesListThread.h
  thread/UpgradeLevelsThread.cpp thread/UpgradeLevelsThread.h
  thread/UploadAllHighscoresThread.cpp thread/UploadAllHighscoresThread.h
  thread/UploadHighscoreThread.cpp thread/UploadHighscoreThread.h
  thread/XMThread.cpp thread/XMThread.h
  thread/XMThreadStats.cpp thread/XMThreadStats.h
  thread/XMThreads.cpp thread/XMThreads.h
)

set(db_src
  db/xmDatabase.cpp db/xmDatabase.h
  db/xmDatabaseUpdateInterface.h
  db/xmDatabase_config.cpp
  db/xmDatabase_fixes.cpp
  db/xmDatabase_levels.cpp
  db/xmDatabase_profiles.cpp
  db/xmDatabase_replays.cpp
  db/xmDatabase_srv.cpp
  db/xmDatabase_stats.cpp
  db/xmDatabase_sync.cpp
  db/xmDatabase_themes.cpp
  db/xmDatabase_votes.cpp
  db/xmDatabase_web.cpp
)

set(xmscene_src
  xmscene/BasicSceneStructs.cpp
  xmscene/BasicSceneStructs.h
  xmscene/Bike.cpp
  xmscene/Bike.h
  xmscene/BikeAnchors.cpp
  xmscene/BikeAnchors.h
  xmscene/BikeController.cpp
  xmscene/BikeController.h
  xmscene/BikeGhost.cpp
  xmscene/BikeGhost.h
  xmscene/BikeParameters.cpp
  xmscene/BikeParameters.h
  xmscene/BikePlayer.cpp
  xmscene/BikePlayer.h
  xmscene/Block.cpp
  xmscene/Block.h
  xmscene/Camera.cpp
  xmscene/Camera.h
  xmscene/ChipmunkWorld.cpp
  xmscene/ChipmunkWorld.h
  xmscene/Entity.cpp
  xmscene/Entity.h
  xmscene/GhostTrail.cpp
  xmscene/GhostTrail.h
  xmscene/Level.cpp
  xmscene/Level.h
  xmscene/PhysicsSettings.cpp
  xmscene/PhysicsSettings.h
  xmscene/Scene.cpp
  xmscene/Scene.h
  xmscene/ScriptTimer.cpp
  xmscene/ScriptTimer.h
  xmscene/Serializer.cpp
  xmscene/SkyApparence.cpp
  xmscene/SkyApparence.h
  xmscene/Zone.cpp
  xmscene/Zone.h
)

set(xmoto_src
  xmoto/BSP.cpp xmoto/BSP.h
  xmoto/Collision.cpp xmoto/Collision.h
  xmoto/Credits.cpp xmoto/Credits.h
  xmoto/GUIBestTimes.cpp
  xmoto/Game.cpp xmoto/Game.h
  xmoto/GameEvents.cpp xmoto/GameEvents.h
  xmoto/GameInit.cpp xmoto/GameText.h
  xmoto/GeomsManager.cpp xmoto/GeomsManager.h
  xmoto/Input.cpp xmoto/Input.h
  xmoto/LevelsManager.cpp xmoto/LevelsManager.h
  xmoto/LevelsText.h
  xmoto/LuaLibBase.cpp xmoto/LuaLibBase.h
  xmoto/LuaLibGame.cpp xmoto/LuaLibGame.h
  xmoto/PhysSettings.h
  xmoto/Renderer.cpp xmoto/Renderer.h
  xmoto/RendererFBO.cpp
  xmoto/Replay.cpp xmoto/Replay.h
  xmoto/ScriptDynamicObjects.cpp xmoto/ScriptDynamicObjects.h
  xmoto/SomersaultCounter.cpp xmoto/SomersaultCounter.h
  xmoto/Sound.cpp xmoto/Sound.h
  xmoto/SysMessage.cpp xmoto/SysMessage.h
  xmoto/Trainer.cpp xmoto/Trainer.h
  xmoto/Universe.cpp xmoto/Universe.h
  xmoto/UserConfig.cpp xmoto/UserConfig.h
  xmoto/VideoRecorder.cpp xmoto/VideoRecorder.h
  xmoto/VirtualLevelsList.cpp xmoto/VirtualLevelsList.h
  xmoto/XMDemo.cpp xmoto/XMDemo.h
  xmoto/XMKey.cpp xmoto/XMKey.h
  xmoto/glext.h
  xmoto/sqlqueries.h
)

include_directories("${PROJECT_SOURCE_DIR}")

add_executable(xmoto
  ${bzip2internal_src}
  ${chipmunk_src}
  ${common_src}
  ${db_src}
  ${drawlib_src}
  ${gui_src}
  ${helpers_src}
  ${image_src}
  ${include_src}
  ${md5sum_src}
  ${net_src}
  ${states_src}
  ${thread_src}
  ${tinyxml_src}
  ${xdgbasedir_src}
  ${xmincludes_src}
  ${xmoto_src}
  ${xmscene_src}
)

TARGET_LINK_LIBRARIES(xmoto
  ${BZIP2_LIBRARIES}
  ${CURL_LIBRARIES}
  ${JPEG_LIBRARIES}
  ${LIBXML2_LIBRARIES}
  ${LUA_LIBRARIES}
  ${ODE_LIBRARY}
  ${OPENGL_LIBRARIES}
  ${PNG_LIBRARY}
  ${SDLMIXER_LIBRARY}
  ${SDLNET_LIBRARY}
  ${SDLTTF_LIBRARY}
  ${SDL_LIBRARY}
  ${SQLITE3_LIBRARIES}
  ${XDG_LIBRARY}
  ${ZLIB_LIBRARIES}
)


INSTALL(TARGETS xmoto RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

MESSAGE("Bzip2     libraries: " ${BZIP2_LIBRARIES})
MESSAGE("Curl      libraries: " ${CURL_LIBRARIES})
MESSAGE("Jpeg      libraries: " ${JPEG_LIBRARIES})
MESSAGE("LibXml2   libraries: " ${LIBXML2_LIBRARIES})
MESSAGE("Lua       libraries: " ${LUA_LIBRARIES})
MESSAGE("Ode       libraries: " ${ODE_LIBRARY})
MESSAGE("OpenGL    libraries: " ${OPENGL_LIBRARIES})
MESSAGE("Png       libraries: " ${PNG_LIBRARY})
MESSAGE("SDL       libraries: " ${SDL_LIBRARY})
MESSAGE("SDL_net   libraries: " ${SDLNET_LIBRARY})
MESSAGE("SDL_ttf   libraries: " ${SDLTTF_LIBRARY})
MESSAGE("SDM_mixer libraries: " ${SDLMIXER_LIBRARY})
MESSAGE("Sqlite3   libraries: " ${SQLITE3_LIBRARIES})
MESSAGE("Xdg       libraries: " ${XDG_LIBRARY})
MESSAGE("Zlib      libraries: " ${ZLIB_LIBRARIES})
MESSAGE("")

MESSAGE("Installation path: " ${CMAKE_INSTALL_PREFIX})
MESSAGE("Game data path: " ${GAMEDATADIR})
