option(STATIC_BUILD "Build a static executable" OFF)
option(USE_OPENGL "Build with OpenGL for graphics" ON)
option(USE_SDLGFX "Build with SDL for graphics" OFF)
option(USE_GETTEXT "Build with Gettext for internationalization" ON)
option(PREFER_SYSTEM_BZip2 "Prefer system BZip2" ON)
option(PREFER_SYSTEM_Lua "Prefer system Lua" ON)
option(PREFER_SYSTEM_ODE "Prefer system ODE" ON)
option(PREFER_SYSTEM_XDG "Prefer system XDG" ON)
option(ALLOW_DEV "Enable some development/debug features" OFF)
option(BUILD_MACOS_BUNDLE "Build xmoto as a macOS Bundle" OFF)

if(CMAKE_GENERATOR MATCHES "Ninja")
  message("-- Using Ninja, try to colorize")
  check_cxx_compiler_flag(-fdiagnostics-color FLAG_CXX_DIAGNOSTICS_COLOR)
  if(FLAG_CXX_DIAGNOSTICS_COLOR)
    add_compile_options("-fdiagnostics-color")
  endif()
else()
  message("-- Using " ${CMAKE_GENERATOR})
endif()

# find dependencies
find_package(BZip2)
set(USE_SYSTEM_BZip2 $<AND:$<BOOL:${PREFER_SYSTEM_BZip2}>,$<BOOL:${BZIP2_FOUND}>,$<NOT:$<BOOL:${WIN32}>>>)

find_package(CURL REQUIRED)
if(USE_GETTEXT)
  find_package(Gettext REQUIRED)
  find_package(Intl REQUIRED)
endif()
find_package(JPEG REQUIRED)

find_package(LibLZMA)
find_package(LibXml2 REQUIRED)

find_package(ODE)
set(USE_SYSTEM_ODE $<AND:$<BOOL:${PREFER_SYSTEM_ODE}>,$<BOOL:${ODE_FOUND}>>)
if(USE_OPENGL)
  find_package(OpenGL REQUIRED)
endif()
find_package(PNG REQUIRED)

find_package(Lua)
set(USE_SYSTEM_Lua $<AND:$<BOOL:${PREFER_SYSTEM_Lua}>,$<BOOL:${LUA_FOUND}>,$<NOT:$<BOOL:${WIN32}>>>)
if (NOT LUA_VERSION_STRING VERSION_LESS 5.2 AND LUA_VERSION_STRING VERSION_LESS 5.3)
    add_definitions("-DLUA_COMPAT_ALL")
elseif (LUA_VERSION_STRING VERSION_GREATER_EQUAL "5.3")
    add_definitions("-DLUA_COMPAT_5_1")
endif()

# Can't disable yet
if(ON OR USE_SDLGFX)
  find_package(SDL REQUIRED)
  find_package(SDL_mixer REQUIRED)
  find_package(SDL_net REQUIRED)
  find_package(SDL_ttf REQUIRED)
endif()

find_package(SQLITE3 REQUIRED)

find_package(XDG)
set(USE_SYSTEM_XDG $<AND:$<BOOL:${PREFER_SYSTEM_XDG}>,$<BOOL:${XDG_FOUND}>>)

find_package(ZLIB REQUIRED)

# sources

add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/bzip2" bzip2)
add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/libccd" libccd)
add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/chipmunk" chipmunk)
add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/lua" lua)
add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/md5sum" md5sum)
add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/ode" ode)
add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/xdgbasedir" xdgbasedir)
add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/glad" glad)

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

set(xmincludes_src
  include/xm_OpenGL.h
  include/xm_SDL.h
  include/xm_SDL_mixer.h
  include/xm_SDL_net.h
  include/xm_SDL_ttf.h
  include/xm_endian.h
  include/xm_hashmap.h
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

set(XMOTO_MACOS_ICON ${PROJECT_SOURCE_DIR}/extra/X-Moto.icns)

set_source_files_properties(
  ${XMOTO_MACOS_ICON} PROPERTIES
  MACOSX_PACKAGE_LOCATION Resources
)

#if(STATIC_BUILD)
  #set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  #set(BUILD_SHARED_LIBRARIES OFF)
  #set(CMAKE_EXE_LINKER_FLAGS "-static")
#endif()

add_executable(xmoto
  ${common_src}
  ${db_src}
  ${drawlib_src}
  ${gui_src}
  ${helpers_src}
  ${image_src}
  ${include_src}
  ${net_src}
  ${states_src}
  ${thread_src}
  ${xmincludes_src}
  ${xmoto_src}
  ${xmscene_src}
  $<$<PLATFORM_ID:Windows>:${PROJECT_SOURCE_DIR}/extra/xmoto.rc>
  $<$<PLATFORM_ID:Darwin>:${XMOTO_MACOS_ICON}>
)

if(NOT APPLE AND BUILD_MACOS_BUNDLE)
  message(FATAL_ERROR "Can't build a macOS bundle for non-macOS platforms")
elseif(APPLE AND BUILD_MACOS_BUNDLE)
  set_property(TARGET xmoto PROPERTY MACOSX_BUNDLE TRUE)
endif()

target_include_directories(xmoto
  PRIVATE

  "${PROJECT_SOURCE_DIR}/src"
  "${LIBXML2_INCLUDE_DIR}"
  "${SDL_INCLUDE_DIR}"
  "${SDL_MIXER_INCLUDE_DIRS}"
  "${SDL_NET_INCLUDE_DIRS}"
  "${SDL_TTF_INCLUDE_DIRS}"
  "$<${USE_SYSTEM_Lua}:${LUA_INCLUDE_DIR}>"
  "$<$<NOT:${USE_SYSTEM_Lua}>:${PROJECT_SOURCE_DIR}/vendor/lua/lua>"
)

target_link_libraries(xmoto PUBLIC
  $<$<PLATFORM_ID:Windows>:userenv>

  "$<${USE_SYSTEM_BZip2}:${BZIP2_LIBRARIES}>"
  "$<$<NOT:${USE_SYSTEM_BZip2}>:bzip2>"
  chipmunk
  ${CURL_LIBRARIES}
    $<$<BOOL:${STATIC_BUILD}>:gnutls>
      #$<$<BOOL:${STATIC_BUILD}>:idn>
      #$<$<BOOL:${STATIC_BUILD}>:unistring>
    $<$<BOOL:${STATIC_BUILD}>:idn2>
      $<$<BOOL:${STATIC_BUILD}>:unistring>
    $<$<BOOL:${STATIC_BUILD}>:ssh2>
      $<$<BOOL:${STATIC_BUILD}>:gcrypt>
      $<$<BOOL:${STATIC_BUILD}>:gpg-error>
    $<$<BOOL:${STATIC_BUILD}>:wldap32>
  ${Intl_LIBRARIES}
  ${JPEG_LIBRARIES}
  ${LIBXML2_LIBRARIES}
    "$<$<BOOL:${STATIC_BUILD}>:${LIBLZMA_LIBRARIES}>"
  "$<${USE_SYSTEM_Lua}:${LUA_LIBRARIES}>"
  $<$<NOT:${USE_SYSTEM_Lua}>:lua>
  md5sum
  $<${USE_SYSTEM_ODE}:${ODE_LIBRARY}>
  $<$<NOT:${USE_SYSTEM_ODE}>:ode>
  glad
  ${CMAKE_DL_LIBS}
  ${OPENGL_LIBRARIES}
  ${PNG_LIBRARY}
  ${SDL_LIBRARY}
  ${SDL_MIXER_LIBRARIES}
  ${SDL_NET_LIBRARIES}
  ${SDL_TTF_LIBRARIES}
  ${SQLITE3_LIBRARIES}
  $<${USE_SYSTEM_XDG}:${XDG_LIBRARY}>
  $<$<NOT:${USE_SYSTEM_XDG}>:xdgbasedir>
  ${ZLIB_LIBRARIES}
)

set_property(TARGET xmoto PROPERTY CXX_STANDARD 11)
# strip release builds
set_target_properties(xmoto PROPERTIES LINK_FLAGS_RELEASE -s)

check_symbol_exists(putenv stdlib.h HAVE_PUTENV)
check_symbol_exists(setenv stdlib.h HAVE_SETENV)
target_compile_definitions(xmoto PUBLIC HAVE_PUTENV=$<BOOL:${HAVE_PUTENV}>)
target_compile_definitions(xmoto PUBLIC HAVE_SETENV=$<BOOL:${HAVE_SETENV}>)
check_prototype_definition(mkdir
  "int mkdir(const char *_Path)"
  "0"
  "sys/types.h;sys/stat.h"
  MS_MKDIR
)
target_compile_definitions(xmoto PUBLIC MS_MKDIR=$<BOOL:${MS_MKDIR}>)

target_compile_definitions(xmoto PUBLIC USE_OPENGL=$<BOOL:${USE_OPENGL}>)
target_compile_definitions(xmoto PUBLIC USE_SDLGFX=$<BOOL:${USE_SDLGFX}>)
target_compile_definitions(xmoto PUBLIC USE_GETTEXT=$<BOOL:${USE_GETTEXT}>)
target_compile_definitions(xmoto PUBLIC ALLOW_DEV=$<BOOL:${ALLOW_DEV}>)
target_compile_definitions(xmoto PUBLIC BUILD_MACOS_BUNDLE=$<BOOL:${BUILD_MACOS_BUNDLE}>)

target_compile_definitions(xmoto PUBLIC $<$<BOOL:${STATIC_BUILD}>:CURL_STATICLIB>)
target_compile_definitions(xmoto PUBLIC $<$<BOOL:${STATIC_BUILD}>:LIBXML_STATIC>)

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

set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
  "/usr/share/applications"
  "/usr/share/pixmaps"
  "/usr/share/locale"
)

# Handle localization
# Based on Dolphin Emulator's Source/Core/DolphinWX/CMakeLists.txt (831ae6a)
if(USE_GETTEXT AND GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGFMT_EXECUTABLE)
  set(pot_file "${CMAKE_SOURCE_DIR}/i18n/po/xmoto.pot")
  file(GLOB LINGUAS "${CMAKE_SOURCE_DIR}/i18n/po/*.po")

  target_sources(xmoto PRIVATE "${pot_file}" ${LINGUAS})
  source_group("i18n" FILES ${LINGUAS})
  source_group("i18n\\\\generated" FILES "${pot_file}")

  foreach(po ${LINGUAS})
    get_filename_component(lang ${po} NAME_WE)
    set(mo_dir "${CMAKE_CURRENT_BINARY_DIR}/${lang}")
    set(mo "${mo_dir}/xmoto.mo")

    target_sources(xmoto PRIVATE "${mo}")
    source_group("i18n\\\\generated" FILES "${mo}")

    if(BUILD_MACOS_BUNDLE AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")
      set_source_files_properties("${mo}" PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources/locale/${lang}/LC_MESSAGES"
      )
    else()
      install(FILES "${mo}" DESTINATION share/locale/${lang}/LC_MESSAGES)

      # work around a problem where cpack thinks it's a good
      # idea to throw in the parent directories of files in
      # the rpm that will be installed in the filesystem...
      set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
        ${CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION}
        "/usr/share/locale/${lang}"
        "/usr/share/locale/${lang}/LC_MESSAGES"
      )
    endif()

    add_custom_command(OUTPUT "${mo}"
      COMMAND mkdir -p "${mo_dir}"
      COMMAND "${GETTEXT_MSGMERGE_EXECUTABLE}" --quiet --update --backup=none -s ${po} "${pot_file}"
      COMMAND "${GETTEXT_MSGFMT_EXECUTABLE}" -o "${mo}" ${po}
      DEPENDS ${po}
    )
  endforeach()
endif()

# make this visible to its parent (and therefore to cpack/)
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
  ${CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION}
  PARENT_SCOPE
)

#target_compile_options(xmoto PRIVATE -Wall)
#target_compile_options(xmoto PRIVATE -Werror)
# chipmunk on Windows
target_compile_options(xmoto PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>)

function(set_win32_compat_path COMPAT REAL)
  # stupid win32 paths..
  string(REGEX REPLACE "^C:(/|\\\\)?" "/c/" _COMPAT "${REAL}")
  set(${COMPAT} "${_COMPAT}" PARENT_SCOPE)
endfunction()

function(win32_run_script _CMD _ARGS _IS_MSYS2)
  if (IS_MSYS2)
    # msys2's cmake is a native win32 program, and it can't run bash scripts directly
    install(CODE "execute_process(COMMAND bash -l -c '${_CMD} ${_ARGS}' \n\
      RESULT_VARIABLE ret ERROR_VARIABLE err)")
  else()
    install(CODE "execute_process(COMMAND ${_CMD} ${_ARGS} \n\
      RESULT_VARIABLE ret ERROR_VARIABLE err)")
  endif()

  get_filename_component(_SCRIPT_NAME "${_CMD}" NAME)
  install(CODE "\
    if(NOT \"\${ret}\" STREQUAL \"0\") \n\
      message(FATAL_ERROR \"${_SCRIPT_NAME} failed:\n\${err}\") \n\
    endif() \n\
  ")
endfunction()

if(WIN32)
  # MXE exports MSYS=1 so we can't use that to
  # differentiate between it and MSYS2
  if(DEFINED ENV{MSYSTEM_PREFIX})
    set(IS_MSYS2 TRUE)
  else()
    set(IS_MSYS2 FALSE)
  endif()
  message("IS_MSYS2: ${IS_MSYS2}")

  if(IS_MSYS2)
    # use msys prefix path as the target
    set_win32_compat_path(DLLCOPY_TARGET $ENV{MSYSTEM_PREFIX})
  else()
    set(DLLCOPY_TARGET "i686-w64-mingw32.shared")
  endif()
  message("DLLCOPY_TARGET: ${DLLCOPY_TARGET}")

  if(NOT IS_MSYS2)
    # this is required to get rid of the console window
    set(CMAKE_EXE_LINKER_FLAGS "-mwindows")
  endif()

  if(IS_MSYS2)
    set_win32_compat_path(WIN32_COMPAT_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
    set_win32_compat_path(WIN32_COMPAT_CURRENT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    set_win32_compat_path(WIN32_COMPAT_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
  else()
    set(WIN32_COMPAT_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
    set(WIN32_COMPAT_CURRENT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    set(WIN32_COMPAT_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
  endif()

  install(
    FILES
      "${PROJECT_SOURCE_DIR}/README.md"
      "${PROJECT_SOURCE_DIR}/COPYING"
      "${PROJECT_SOURCE_DIR}/ChangeLog"
    DESTINATION "./"
  )
  set(WIN32_DLL_DEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/dll")
  set_win32_compat_path(WIN32_COMPAT_DLL_DEST_DIR "${WIN32_DLL_DEST_DIR}")
  set(COPY_DLLS_CMD "${WIN32_COMPAT_SOURCE_DIR}/tools/win32_copy_dlls.sh")
  string(CONCAT COPY_DLLS_ARGS
    " -t \"${DLLCOPY_TARGET}\""
    " -o \"${WIN32_COMPAT_DLL_DEST_DIR}\""
    " -e \"${WIN32_COMPAT_CURRENT_BINARY_DIR}/xmoto.exe\""
    " -r"
  )
  win32_run_script("${COPY_DLLS_CMD}" "${COPY_DLLS_ARGS}" IS_MSYS2)

  install(TARGETS xmoto RUNTIME DESTINATION "./")

  install(
    # NOTE: don't use WIN32_COMPAT_DLL_DEST_DIR here!
    DIRECTORY "${WIN32_DLL_DEST_DIR}/"
    DESTINATION "./"
    FILES_MATCHING PATTERN "*.dll"
  )

  if (CMAKE_BUILD_TYPE STREQUAL "Release")
    # strip the dlls
    set(STRIP_DLLS_CMD "${WIN32_COMPAT_SOURCE_DIR}/tools/win32_strip_dlls.sh")
    string(CONCAT
      STRIP_DLLS_ARGS
      " \"${CMAKE_STRIP}\"" # (path to) strip program
      " \"${WIN32_COMPAT_INSTALL_PREFIX}\"" # DLL folder
    )
    win32_run_script("${STRIP_DLLS_CMD}" "${STRIP_DLLS_ARGS}" IS_MSYS2)
  endif()
elseif(NOT BUILD_MACOS_BUNDLE)
  # for macOS bundles, this is set in cpack/macos.cmake
  # because otherwise fixup_bundle() would complain
  install(TARGETS xmoto RUNTIME DESTINATION bin)
endif()

message("BZip2     libraries: ${BZIP2_LIBRARIES}")
message("Curl      libraries: ${CURL_LIBRARIES}")
message("Intl      libraries: ${Intl_LIBRARIES}")
message("Jpeg      libraries: ${JPEG_LIBRARIES}")
message("LibXml2   libraries: ${LIBXML2_LIBRARIES}")
message("LibLZMA   libraries: ${LIBLZMA_LIBRARIES}")
message("Lua       libraries: ${LUA_LIBRARIES}")
message("Ode       libraries: ${ODE_LIBRARY}")
message("OpenGL    libraries: ${OPENGL_LIBRARIES}")
message("Png       libraries: ${PNG_LIBRARY}")
message("SDL       libraries: ${SDL_LIBRARY}")
message("SDL_mixer libraries: ${SDL_MIXER_LIBRARIES}")
message("SDL_net   libraries: ${SDL_NET_LIBRARIES}")
message("SDL_ttf   libraries: ${SDL_TTF_LIBRARIES}")
message("Sqlite3   libraries: ${SQLITE3_LIBRARIES}")
message("Xdg       libraries: ${XDG_LIBRARY}")
message("Zlib      libraries: ${ZLIB_LIBRARIES}")
message("")

message("Installation path: ${CMAKE_INSTALL_PREFIX}")
message("Flags:")
if(${ALLOW_DEV})
  message("-- Dev enabled")
endif()
