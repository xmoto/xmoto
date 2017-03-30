CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)

# cmake -D CMAKE_GENERATOR="Unix Makefiles" ../trunk/src
SET(CMAKE_GENERATOR "Unix Makefiles")
PROJECT(xmoto)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake_modules)

# the name of the target operating system
#SET(CMAKE_SYSTEM_NAME Windows)
#SET(CMAKE_SYSTEM_NAME Linux)

# cross compilation on windows
MESSAGE("System: "${CMAKE_SYSTEM_NAME})
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
INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR})
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_ttf REQUIRED)
FIND_PACKAGE(SDL_mixer REQUIRED)
FIND_PACKAGE(SDL_net REQUIRED)

IF(CMAKE_SYSTEM_NAME MATCHES Windows)
 set(bzip2internal_src
     bzip2/crctable.c bzip2/bzlib_private.h bzip2/blocksort.c bzip2/bzlib.c bzip2/decompress.c
     bzip2/huffman.c bzip2/bzlib.h bzip2/compress.c bzip2/randtable.c
 )
ELSE(CMAKE_SYSTEM_NAME MATCHES Windows)
 set(bzip2internal_src)
 FIND_PACKAGE(BZip2 REQUIRED)
ENDIF(CMAKE_SYSTEM_NAME MATCHES Windows)

FIND_PACKAGE(ZLIB REQUIRED)
FIND_PACKAGE(PNG REQUIRED)
FIND_PACKAGE(JPEG REQUIRED)
FIND_PACKAGE(CURL REQUIRED)
FIND_PACKAGE(SQLITE3 REQUIRED)
FIND_PACKAGE(ODE REQUIRED)
FIND_PACKAGE(Gettext)
FIND_PACKAGE(XDG REQUIRED)

SET(DATADIR ${CMAKE_INSTALL_PREFIX}/share)
SET(GAMEDATADIR ${DATADIR}/${CMAKE_PROJECT_NAME})

# include languages
IF(GETTEXT_FOUND)
# GETTEXT_CREATE_TRANSLATIONS(../po/xmoto.pot ../po/fr_FR.po ../po/de_DE.po)
ENDIF(GETTEXT_FOUND)

# include svn revision
SET(SVN_REV exported)
FIND_PACKAGE(Subversion)
IF(Subversion_FOUND)
 IF(EXISTS ${PROJECT_SOURCE_DIR}/.svn)
  Subversion_WC_INFO(${PROJECT_SOURCE_DIR} Project)
  SET(SVN_REV ${Project_WC_REVISION})
 ENDIF(EXISTS ${PROJECT_SOURCE_DIR}/.svn)
ENDIF(Subversion_FOUND)
ADD_DEFINITIONS(-DSVN_REV=\"${SVN_REV}\")

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
 include/xm_SDL_mixer.h include/xm_ode.h include/xm_endian.h include/xm_SDL_ttf.h
 include/xm_hashmap.h include/xm_lua.h include/xm_SDL.h include/xm_OpenGL.h include/xm_SDL_net.h
)

set(common_src
GeomsManager.h
GeomsManager.cpp
SomersaultCounter.cpp
Universe.cpp
PolyDraw.cpp
VideoRecorder.h
GameEvents.h
GameSerializer.cpp
Trainer.cpp
VBezier.cpp
Renderer.h
WWWAppInterface.h
svn_version.h
DBuffer.h
Locales.cpp
SysMessage.cpp
WWW.cpp
VTexture.cpp
Sound.cpp
XMSession_default.cpp
RendererFBO.cpp
CameraAnimation.h
XMSession.h
LuaLibGame.cpp
SomersaultCounter.h
UserConfig.cpp
LevelsText.h
Universe.h
Trainer.h
VBezier.h
Languages.h
svn_version.cpp
DBuffer.cpp
VCommon.h
BSP.cpp
iqsort.h
Locales.h
glext.h
WWW.h
LevelsManager.h
VTexture.h
Replay.cpp
Sound.h
XMSession_default.h
Theme.cpp
GUIBestTimes.cpp
Input.h
XMKey.h
XMBuild.cpp
XMArgs.h
LuaLibGame.h
XMotoLoadLevelsInterface.h
UserConfig.h
PhysSettings.h
Game.h
CRCHash.h
Packager.h
Image.cpp
LevelsManager.cpp
VXml.cpp
ScriptDynamicObjects.h
BSP.h
VideoRecorder.cpp
GameEvents.cpp
Input.cpp
XMKey.cpp
Collision.h
Replay.h
XMArgs.cpp
Credits.h
VFileIO.h
Renderer.cpp
PolyDraw.h
Theme.h
Game.cpp
XMBuild.h
CRCHash.cpp
Packager.cpp
ScriptDynamicObjects.cpp
GameText.h
XMBuild_info.h
Image.h
SysMessage.h
CameraAnimation.cpp
XMSession.cpp
BuildConfig.h
Collision.cpp
VXml.h
XMotoLoadReplaysInterface.h
VFileIO.cpp
Credits.cpp
GameInit.cpp
VirtualLevelsList.h
VirtualLevelsList.cpp
Game.h
)

set(image_src
 image/tim.h image/tim_jpeg.cpp image/tim_io_stdio.cpp image/tim.cpp image/tim_memory_crt.cpp image/tim_png.cpp
)

set (helpers_src
helpers/SwapEndian.h
helpers/FileCompression.cpp
helpers/Environment.cpp
helpers/FileCompression.h
helpers/Singleton.h
helpers/Environment.h
helpers/VMath.cpp
helpers/Random.cpp
helpers/Log.cpp
helpers/MultiSingleton.h
helpers/Color.h
helpers/CmdArgumentParser.cpp
helpers/VExcept.h
helpers/System.cpp
helpers/Text.cpp
helpers/VMath.h
helpers/Random.h
helpers/Log.h
helpers/utf8.cpp
helpers/CmdArgumentParser.h
helpers/SwapEndian.cpp
helpers/System.h
helpers/Text.h
helpers/HashTbl.h
helpers/utf8.h
helpers/RenderSurface.h
helpers/RenderSurface.cpp
)

set(thread_src
thread/UpgradeLevelsThread.h
thread/XMThreads.cpp
thread/UpdateThemeThread.h
thread/SyncThread.cpp
thread/XMThreads.h
thread/UploadHighscoreThread.cpp
thread/SyncThread.h
thread/CheckWwwThread.cpp
thread/UpdateThemesListThread.cpp
thread/XMThread.cpp
thread/UploadHighscoreThread.h
thread/CheckWwwThread.h
thread/UpdateDbThread.cpp
thread/UpdateThemesListThread.h 
thread/XMThread.h 
thread/UploadAllHighscoresThread.cpp
thread/UpdateDbThread.h
thread/UpdateRoomsListThread.cpp
thread/UploadAllHighscoresThread.h
thread/UpgradeLevelsThread.cpp 
thread/UpdateRoomsListThread.h
thread/UpdateThemeThread.cpp
thread/LevelsPacksCountUpdateThread.h
thread/LevelsPacksCountUpdateThread.cpp
thread/XMThreadStats.h
thread/XMThreadStats.cpp
thread/SendVoteThread.h
thread/SendVoteThread.cpp
thread/DownloadReplaysThread.h
thread/DownloadReplaysThread.cpp
)

set(gui_src
gui/basic/GUIFrame.cpp
gui/basic/GUIButton.cpp
gui/basic/GUIList.cpp
gui/basic/GUI.cpp
gui/basic/GUITabView.cpp
gui/basic/GUI.h
gui/basic/GUIEdit.cpp
gui/basic/GUIButtonDrawn.cpp
gui/basic/GUIStatic.cpp
gui/specific/GUIXMoto.h
gui/specific/GUIXMoto.cpp
gui/basic/GUIConsole.h
gui/basic/GUIConsole.cpp
)

set(xmscene_src
xmscene/BikeParameters.cpp
xmscene/Block.h
xmscene/Scene.h
xmscene/BikeParameters.h
xmscene/Level.cpp
xmscene/Bike.cpp
xmscene/Zone.cpp
xmscene/Entity.cpp
xmscene/Level.h
xmscene/BikeController.cpp
xmscene/BikeGhost.cpp
xmscene/Bike.h
xmscene/Zone.h
xmscene/Entity.h
xmscene/BikeController.h
xmscene/BikeGhost.h
xmscene/SkyApparence.cpp
xmscene/ChipmunkWorld.cpp
xmscene/BikePlayer.cpp
xmscene/Camera.cpp
xmscene/BikeAnchors.cpp
xmscene/Block.cpp
xmscene/SkyApparence.h
xmscene/ChipmunkWorld.h
xmscene/BikePlayer.h
xmscene/BasicSceneStructs.h
xmscene/BikeAnchors.h
xmscene/Camera.h
xmscene/Scene.cpp
xmscene/ScriptTimer.h
xmscene/ScriptTimer.cpp
xmscene/GhostTrail.h
xmscene/GhostTrail.cpp
xmscene/BasicSceneStructs.h
xmscene/BasicSceneStructs.cpp
xmscene/PhysicsSettings.cpp
xmscene/PhysicsSettings.h
)

set(tinyxml_src
tinyxml/tinyxmlerror.cpp
tinyxml/tinystr.h
tinyxml/tinyxmlparser.cpp
tinyxml/tinyxml.cpp
tinyxml/tinyxml.h
tinyxml/tinystr.cpp
)

set(md5sum_src
md5sum/md5.c
md5sum/md5file.cpp
md5sum/md5.h
md5sum/md5file.h
)

set(db_src
db/xmDatabase.h
db/xmDatabase_config.cpp
db/xmDatabase_themes.cpp
db/xmDatabaseUpdateInterface.h
db/xmDatabase_levels.cpp
db/xmDatabase_sync.cpp
db/xmDatabase_web.cpp
db/xmDatabase_fixes.cpp
db/xmDatabase_stats.cpp
db/xmDatabase_replays.cpp
db/xmDatabase.cpp
db/xmDatabase_profiles.cpp
db/xmDatabase_votes.cpp
db/xmDatabase_srv.cpp
)

set(drawlib_src
drawlib/DrawLibOpenGL.h
drawlib/DrawLib.cpp
drawlib/DrawLib.h
drawlib/DrawLibOpenGL.cpp
)

set(states_src
states/StatePlayingLocal.h
states/StatePlayingLocal.h
states/StateMultiUpdate.cpp
states/StateUpdateRoomsList.cpp
states/StateFinished.h
states/StateScene.h
states/StateDeadJust.cpp
states/StateUpdateTheme.cpp
states/StateRequestKey.h
states/AttractMode.cpp
states/StateMenu.h
states/GameState.h
states/StateMainMenu.h
states/StateUpdateThemesList.cpp
states/StateUpdate.h
states/StatePreplayingReplay.cpp
states/StateOptions.cpp
states/StateReplaying.h
states/StateMultiUpdate.h
states/StateMenu.cpp
states/StateLevelInfoViewer.cpp
states/StateCreditsMode.h
states/StateUpgradeLevels.h
states/StateDeadJust.h
states/StatePreplayingGame.h
states/StateEditWebConfig.cpp
states/StateUpdateTheme.h
states/StatePreplaying.h
states/StateUpdate.cpp
states/StateMessageBoxReceiver.h
states/StateCheckWww.h
states/StateReplaying.cpp
states/StatePreplayingReplay.h
states/StateUpdateDb.h
states/StatePreplayingCredits.h
states/StateMessageBox.cpp
states/StateLevelPackViewer.h
states/StateOptions.h
states/StateCreditsMode.cpp
states/StateUpgradeLevels.cpp
states/StatePlaying.cpp
states/StatePreplayingGame.cpp
states/StatePreplaying.cpp
states/StateLevelInfoViewer.h
states/StateEditProfile.h
states/StatePause.cpp
states/StateEditWebConfig.h
states/StateManager.h
states/StateSync.h
states/StateCheckWww.cpp
states/StateUploadHighscore.h
states/StateDeadMenu.h
states/StateUpdateDb.cpp
states/StatePreplayingCredits.cpp
states/StateLevelPackViewer.cpp
states/StateFinished.cpp
states/StateScene.cpp
states/StateHelp.h
states/StateRequestKey.cpp
states/StateUploadAllHighscores.h
states/StateMessageBox.h
states/StateUpdateRoomsList.h
states/StateEditProfile.cpp
states/StatePlaying.h
states/GameState.cpp
states/StateMainMenu.cpp
states/StateManager.cpp
states/AttractMode.h
states/StateSync.cpp
states/StateUploadHighscore.cpp
states/StatePause.h
states/StateDeadMenu.cpp
states/StateHelp.cpp
states/StateUpdateThemesList.h
states/StateUploadAllHighscores.cpp
states/StateVote.h
states/StateVote.cpp
states/StateWaiting.cpp
states/StateWaiting.h
states/StateViewHighscore.cpp
states/StateViewHighscore.h
states/StateWaitServerInstructions.cpp
states/StateWaitServerInstructions.h
states/StatePlayingLocal.h
states/StatePlayingLocal.cpp
states/StateSendVote.h
states/StateSendVote.cpp
states/StateServerConsole.h
states/StateServerConsole.cpp
states/StatePreplayingNet.h
states/StatePreplayingNet.cpp
states/StatePlayingNet.cpp
states/StatePlayingNet.h
)

set(chipmunk_src
chipmunk/chipmunk.h
chipmunk/cpArray.c
chipmunk/cpJoint.c
chipmunk/cpBB.c
chipmunk/cpArray.h
chipmunk/cpJoint.h
chipmunk/cpBB.h
chipmunk/cpArbiter.c
chipmunk/cpCollision.c
chipmunk/chipmunk.c
chipmunk/cpSpace.c
chipmunk/cpBody.c
chipmunk/cpArbiter.h
chipmunk/prime.h
chipmunk/cpHashSet.c
chipmunk/cpSpaceHash.c
chipmunk/cpCollision.h
chipmunk/cpShape.c
chipmunk/cpSpace.h
chipmunk/cpVect.c
chipmunk/cpBody.h
chipmunk/cpHashSet.h
chipmunk/cpPolyShape.c
chipmunk/cpSpaceHash.h
chipmunk/cpShape.h
chipmunk/cpVect.h
chipmunk/cpPolyShape.h
)

set(net_src
net/NetServer.cpp
net/NetServer.h
net/NetClient.cpp
net/NetClient.h
net/NetActions.cpp
net/NetActions.h
net/extSDL_net.cpp
net/extSDL_net.h
net/VirtualNetLevelsList.h
net/thread/ClientListenerThread.h
net/thread/ClientListenerThread.cpp
net/thread/ServerThread.h
net/thread/ServerThread.cpp
net/ActionReader.cpp
net/ActionReader.h
net/VirtualNetLevelsList.h
net/VirtualNetLevelsList.cpp
net/helpers/Net.cpp
net/helpers/Net.h
)

add_executable(xmoto
${xmincludes_src}
${common_src}
${image_src}
${helpers_src}
${thread_src}
${include_src}
${gui_src}
${xmscene_src}
${tinyxml_src}
${md5sum_src}
${db_src}
${drawlib_src}
${bzip2internal_src}
${states_src}
${chipmunk_src}
${net_src}
)

TARGET_LINK_LIBRARIES(xmoto
 ${OPENGL_LIBRARIES} ${SDL_LIBRARY}       ${SDLTTF_LIBRARY}  ${SDLMIXER_LIBRARY}
 ${PNG_LIBRARY}      ${JPEG_LIBRARIES}    ${BZIP2_LIBRARIES} ${CURL_LIBRARIES}
 ${LUA_LIBRARIES}    ${SQLITE3_LIBRARIES} ${ODE_LIBRARY}     ${ZLIB_LIBRARIES}
 ${XDG_LIBRARY}      ${SDLNET_LIBRARY} 
)


INSTALL(TARGETS xmoto RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

MESSAGE("OpenGL    libraries: "${OPENGL_LIBRARIES})
MESSAGE("SDL       libraries: "${SDL_LIBRARY})
MESSAGE("SDL_ttf   libraries: "${SDLTTF_LIBRARY})
MESSAGE("SDM_mixer libraries: "${SDLMIXER_LIBRARY})
MESSAGE("Png       libraries: "${PNG_LIBRARY})
MESSAGE("Jpeg      libraries: "${JPEG_LIBRARIES})
MESSAGE("Bzip2     libraries: "${BZIP2_LIBRARIES})
MESSAGE("Curl      libraries: "${CURL_LIBRARIES})
MESSAGE("Lua       libraries: "${LUA_LIBRARIES})
MESSAGE("Sqlite3   libraries: "${SQLITE3_LIBRARIES})
MESSAGE("Ode       libraries: "${ODE_LIBRARY})
MESSAGE("Zlib      libraries: "${ZLIB_LIBRARIES})
MESSAGE("Xdg       libraries: "${XDG_LIBRARY})
MESSAGE("SDL_net   libraries: "${SDLNET_LIBRARY})

MESSAGE("Installation path: "${CMAKE_INSTALL_PREFIX})
MESSAGE("Game data path: "${GAMEDATADIR})