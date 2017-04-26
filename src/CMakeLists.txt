option(USE_OPENGL "Build with OpenGL for graphics" ON)
option(USE_SDLGFX "Build with SDL for graphics" OFF)
option(ALLOW_DEV "Enable some development/debug features." OFF)

if(CMAKE_GENERATOR MATCHES "Ninja")
  message("-- Using Ninja, try to colorize")
  check_cxx_compiler_flag(-fdiagnostics-color FLAG_CXX_DIAGNOSTICS_COLOR)
  if (FLAG_CXX_DIAGNOSTICS_COLOR)
    add_compile_options("-fdiagnostics-color")
  endif()
else()
  message("-- Using " ${CMAKE_GENERATOR})
endif()

# find dependencies
if(CMAKE_SYSTEM_NAME MATCHES Windows)
 set(bzip2internal_src
   bzip2/blocksort.c
   bzip2/bzlib.c
   bzip2/bzlib.h
   bzip2/bzlib_private.h
   bzip2/compress.c
   bzip2/crctable.c
   bzip2/decompress.c
   bzip2/huffman.c
   bzip2/randtable.c
 )
else()
 set(bzip2internal_src)
 find_package(BZip2 REQUIRED)
endif()

find_package(CURL REQUIRED)
find_package(Gettext)
find_package(JPEG REQUIRED)

find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})

find_package(Lua51 REQUIRED)
message("--    Including from: " ${LUA_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR})

find_package(ODE REQUIRED)
if(USE_OPENGL)
  find_package(OpenGL REQUIRED)
endif()
find_package(PNG REQUIRED)

# Can't disable yet
if(ON OR USE_SDLGFX)
  find_package(SDL REQUIRED)
  find_package(SDL_mixer REQUIRED)
  find_package(SDL_net REQUIRED)
  find_package(SDL_ttf REQUIRED)
endif()

find_package(SQLITE3 REQUIRED)
find_package(XDG REQUIRED)
find_package(ZLIB REQUIRED)

set(DATADIR ${CMAKE_INSTALL_PREFIX}/share)
set(GAMEDATADIR ${DATADIR}/${CMAKE_PROJECT_NAME})

# include languages
if(GETTEXT_FOUND)
# GETTEXT_CREATE_TRANSLATIONS(../po/xmoto.pot ../po/fr_FR.po ../po/de_DE.po)
endif()

# sources

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

set(drawlib_src
  drawlib/DrawLib.cpp drawlib/DrawLib.h
)
if(USE_OPENGL)
  list(APPEND drawlib_src
    drawlib/DrawLibOpenGL.cpp drawlib/DrawLibOpenGL.h
  )
endif()
if(USE_SDLGFX)
  list(APPEND drawlib_src
    drawlib/DrawLibSDLgfx.cpp drawlib/DrawLibSDLgfx.h
  )
endif()

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

set(helpers_src
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

set(net_src
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

set(thread_src
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

set(xdgbasedir_src
  xdgbasedir/include/basedir.h
  xdgbasedir/include/basedir_fs.h
  xdgbasedir/src/basedir.c
)

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

include_directories(${PROJECT_SOURCE_DIR}/src)

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
  ${xdgbasedir_src}
  ${xmincludes_src}
  ${xmoto_src}
  ${xmscene_src}
)

target_link_libraries(xmoto
  ${BZIP2_LIBRARIES}
  ${CURL_LIBRARIES}
  ${JPEG_LIBRARIES}
  ${LIBXML2_LIBRARIES}
  ${LUA_LIBRARIES}
  ${ODE_LIBRARY}
  ${OPENGL_LIBRARIES}
  ${PNG_LIBRARY}
  ${SDL_LIBRARY}
  ${SDLMIXER_LIBRARY}
  ${SDLNET_LIBRARY}
  ${SDLTTF_LIBRARY}
  ${SQLITE3_LIBRARIES}
  ${XDG_LIBRARY}
  ${ZLIB_LIBRARIES}
)

target_compile_definitions(xmoto PUBLIC CMAKE_LUA_H)
target_compile_definitions(xmoto PUBLIC SVN_REV=\"git\")

# options
target_compile_definitions(xmoto PRIVATE CONFIGURE_DEFAULT_THEME=\"Modern\")

if(CMAKE_SYSTEM_NAME MATCHES Windows)
  target_compile_definitions(xmoto PRIVATE
    DEFAULT_ASIAN_TTF_FILE=\"Textures/Fonts/bkai00mp.ttf\")
else()
  target_compile_definitions(xmoto PRIVATE
    DEFAULT_ASIAN_TTF_FILE=\"/usr/share/fonts/truetype/arphic/bkai00mp.ttf\")
endif()

#target_compile_options(xmoto PRIVATE -Wall)
#target_compile_options(xmoto PRIVATE -Werror)

install(TARGETS xmoto
  RUNTIME DESTINATION bin
)

message("BZip2     libraries: " ${BZIP2_LIBRARIES})
message("Curl      libraries: " ${CURL_LIBRARIES})
message("Jpeg      libraries: " ${JPEG_LIBRARIES})
message("LibXml2   libraries: " ${LIBXML2_LIBRARIES})
message("Lua       libraries: " ${LUA_LIBRARIES})
message("Ode       libraries: " ${ODE_LIBRARY})
message("OpenGL    libraries: " ${OPENGL_LIBRARIES})
message("Png       libraries: " ${PNG_LIBRARY})
message("SDL       libraries: " ${SDL_LIBRARY})
message("SDL_mixer libraries: " ${SDLMIXER_LIBRARY})
message("SDL_net   libraries: " ${SDLNET_LIBRARY})
message("SDL_ttf   libraries: " ${SDLTTF_LIBRARY})
message("Sqlite3   libraries: " ${SQLITE3_LIBRARIES})
message("Xdg       libraries: " ${XDG_LIBRARY})
message("Zlib      libraries: " ${ZLIB_LIBRARIES})
message("")

message("Installation path: " ${CMAKE_INSTALL_PREFIX})
message("Flags:")
if(${ALLOW_DEV})
  message("-- Dev enabled")
endif()
