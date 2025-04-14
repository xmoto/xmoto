/*=============================================================================
XMOTO

This file is part of XMOTO.

XMOTO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

XMOTO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XMOTO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
=============================================================================*/

#include "XMSession.h"
#include "WWW.h"
#include "XMArgs.h"
#include "db/xmDatabase.h"
#include "helpers/VMath.h"
#include "xmoto/UserConfig.h"
#include "xmoto/VideoRecorder.h"
#include <curl/curl.h>
#include <sstream>

#define PROPAGATE(A, B, C, D) A::propagate(this, new TFunctor1A<A, D>(&A::B, C))
#define PROPAGATE_REF(A, B, C, D) \
  A::propagate(this, new TFunctor1ARef<A, D>(&A::B, C))

XMSession::XMSession() {
  setToDefault();

  /* don't set them by default */
  m_webConfAtInit = DEFAULT_WEBCONFATINIT;
  m_profile = DEFAULT_PROFILE;

  m_bSafemodeActive = false;
}

void XMSession::setToDefault() {
  // Initialise propagation in MultiSingleton
  setIsPropagator(false);

  m_language = DEFAULT_LANGUAGE;
  m_verbose = DEFAULT_VERBOSE;
  m_resolutionWidth = DEFAULT_RESOLUTION_WIDTH;
  m_resolutionHeight = DEFAULT_RESOLUTION_HEIGHT;
  if (m_maxRenderFps == DEFAULT_MAXRENDERFPS) {
    m_maxRenderFps = DEFAULT_MAXRENDERFPS;
  }
  m_windowed = DEFAULT_WINDOWED;
  m_useThemeCursor = DEFAULT_USETHEMECURSOR;
  m_glExts = DEFAULT_GLEXTS;
  m_glVOBS = DEFAULT_GLVOBS;
  m_drawlib = DEFAULT_DRAWLIB;
  m_www = DEFAULT_WWW;
  m_www_password = DEFAULT_WWW_PASSWORD;
  m_benchmark = DEFAULT_BENCHMARK;
  m_debug = DEFAULT_DEBUG;
  m_sqlTrace = DEFAULT_SQLTRACE;
  m_gdebug = DEFAULT_GDEBUG;
  m_timedemo = DEFAULT_TIMEDEMO;
  m_fps = DEFAULT_FPS;
  m_ugly = DEFAULT_UGLY;
  m_hideSpritesUgly = DEFAULT_HIDESPRITESUGLY;
  m_hideSpritesMinimap = DEFAULT_HIDESPRITESMINIMAP;
  m_uglyOver = DEFAULT_UGLYOVER;
  m_testTheme = DEFAULT_TESTTHEME;
  m_noLog = DEFAULT_NOLOG;
  m_ghostStrategy_MYBEST = DEFAULT_GHOST_MYBEST;
  m_ghostStrategy_THEBEST = DEFAULT_GHOST_THEBEST;
  m_ghostStrategy_BESTOFREFROOM = DEFAULT_GHOSTBESTREFROOM;
  m_ghostStrategy_BESTOFOTHERROOMS = DEFAULT_GHOSTBESTOTHERROOMS;
  m_autosaveHighscoreReplays = DEFAULT_AUTOSAVEHIGHSCORESREPLAYS;
  m_disableAnimations = DEFAULT_DISABLEANIMATIONS;
  m_enableGhosts = DEFAULT_ENABLEGHOSTS;
  m_enableEngineSound = DEFAULT_ENABLEENGINESOUND;
  m_showEngineCounter = DEFAULT_SHOWENGINECOUNTER;
  m_showMinimap = DEFAULT_SHOWMINIMAP;
  m_multiStopWhenOneFinishes = DEFAULT_MULTISTOPWHENONEFINISHES;
  m_enableMenuMusic = DEFAULT_ENABLEMENUMUSIC;
  m_enableGameMusic = DEFAULT_ENABLEGAMEMUSIC;
  m_enableDeadAnimation = DEFAULT_ENABLEDEADANIMATION;
  m_enablePermanentConsole = DEFAULT_PERMANENTCONSOLE;
  m_showGameInformationInConsole = DEFAULT_SHOWGAMEINFORMATIONINCONSOLE;
  m_consoleSize = DEFAULT_CONSOLESIZE;
  m_menuGraphics = DEFAULT_MENUGRAPHICS;
  m_gameGraphics = DEFAULT_GAMEGRAPHICS;
  m_quickStartQualityMIN = DEFAULT_QUICKSTARTQUALITYMIN;
  m_quickStartQualityMAX = DEFAULT_QUICKSTARTQUALITYMAX;
  m_quickStartDifficultyMIN = DEFAULT_QUICKSTARTDIFFICULTYMIN;
  m_quickStartDifficultyMAX = DEFAULT_QUICKSTARTDIFFICULTYMAX;
  m_multiNbPlayers = DEFAULT_MULTINBPLAYERS;
  m_multiGameMode = DEFAULT_MULTIGAMEMODE;
  m_multiScenes = DEFAULT_MULTISCENES;
  m_enableContextHelp = DEFAULT_ENABLECONTEXTHELP;
  m_theme = XMDefault::DefaultTheme;
  m_enableAudio = DEFAULT_ENABLEAUDIO;
  m_audioSampleRate = DEFAULT_AUDIOSAMPLERATE;
  m_audioSampleBits = DEFAULT_AUDIOSAMPLEBITS;
  m_audioChannels = DEFAULT_AUDIOCHANNELS;
  m_enableAudioEngine = DEFAULT_ENABLEAUDIOENGINE;
  m_checkNewLevelsAtStartup = DEFAULT_CHECKNEWLEVELSATSTARTUP;
  m_checkNewHighscoresAtStartup = DEFAULT_CHECKNEWHIGHSCORESATSTARTUP;
  m_showHighscoreInGame = DEFAULT_SHOWHIGHSCOREINGAME;
  m_showNextMedalInGame = DEFAULT_SHOWNEXTMEDALINGAME;
  for (unsigned int i = 0; i < ROOMS_NB_MAX; i++) {
    m_idRoom[i] = DEFAULT_WEBROOM_ID;
  }
  m_nbRoomsEnabled = DEFAULT_NBROOMSENABLED;
  m_showGhostTimeDifference = DEFAULT_SHOWGHOSTTIMEDIFFERENCE;
  m_ghostMotionBlur = DEFAULT_GHOSTMOTIONBLUR;
  m_showGhostsInfos = DEFAULT_SHOWGHOSTSINFOS;
  m_showBikersArrows = DEFAULT_SHOWBIKERSARROWS;
  m_hideGhosts = DEFAULT_HIDEGHOSTS;
  m_replayFrameRate = DEFAULT_REPLAYFRAMERATE;
  m_webThemesURL = DEFAULT_WEBTHEMES_URL;
  m_webThemesURLBase = DEFAULT_WEBTHEMES_SPRITESURLBASE;
  m_webRoomsURL = DEFAULT_WEBROOMS_URL;
  m_storeReplays = DEFAULT_STOREREPLAYS;
  m_enableReplayInterpolation = DEFAULT_ENABLEREPLAYINTERPOLATION;
  m_uploadHighscoreUrl = DEFAULT_UPLOADREPLAY_URL;
  m_screenshotFormat = DEFAULT_SCREENSHOTFORMAT;
  m_notifyAtInit = DEFAULT_NOTIFYATINIT;
  m_webLevelsUrl = DEFAULT_WEBLEVELS_URL;
  m_uploadDbSyncUrl = DEFAULT_UPLOADDBSYNC_URL;
  m_mirrorMode = DEFAULT_MIRRORMODE;
  m_useCrappyPack = DEFAULT_USECRAPPYPACK;
  m_useChildrenCompliant = DEFAULT_USECHILDRENCOMPLIANT;
  m_forceChildrenCompliant = DEFAULT_FORCECHILDRENCOMPLIANT;
  m_enableVideoRecording = DEFAULT_ENABLEVIDEORECORDING;
  m_videoRecordingDivision = VR_DEFAULT_DIVISION;
  m_videoRecordingFramerate = VR_DEFAULT_FRAMERATE;
  m_videoRecordingStartTime = DEFAULT_VIDEORECORDINGSTARTTIME;
  m_videoRecordingEndTime = DEFAULT_VIDEORECORDINGENDTIME;
  m_hidePlayingInformation = DEFAULT_HIDEPLAYINGINFORMATION;
  m_enableInitZoom = DEFAULT_ENABLEINITZOOM;
  m_enableActiveZoom = DEFAULT_ENABLEACTIVEZOOM;
  m_enableTrailCam = DEFAULT_ENABLETRAILCAM;
  m_renderGhostTrail = DEFAULT_GHOSTTRAILRENDERING;
  m_dbsynchronizeOnQuit = DEFAULT_DBSYNCHRONIZEONQUIT;
  m_enableJoysticks = DEFAULT_ENABLEJOYSTICKS;
  m_adminMode = DEFAULT_ADMINMODE;
  m_beatingMode = DEFAULT_BEATINGMODE;
  m_webForms = DEFAULT_WEBFORMS;
  m_serverStartAtStartup = DEFAULT_SERVERSTARTATSTARTUP;
  m_clientConnectAtStartup = DEFAULT_CLIENTCONNECTATSTARTUP;
  m_serverPort = DEFAULT_SERVERPORT;
  m_serverMaxClients = DEFAULT_SERVERMAXCLIENTS;
  m_clientServerName = DEFAULT_CLIENTSERVERNAME;
  m_clientGhostMode = DEFAULT_CLIENTGHOSTMODE;
  m_clientServerPort = DEFAULT_CLIENTSERVERPORT;
  m_clientFramerateUpload = DEFAULT_CLIENTFRAMERATEUPLOAD;
  m_musicOnAllLevels = DEFAULT_MUSICONALLLEVELS;
  m_proxySettings.setDefault();
}

void XMSession::loadArgs(const XMArguments *i_xmargs) {
  if (i_xmargs->isOptVerbose()) {
    m_verbose = true;
  }
  if (i_xmargs->isOptRes()) {
    m_resolutionWidth = i_xmargs->getOpt_res_dispWidth();
    m_resolutionHeight = i_xmargs->getOpt_res_dispHeight();
  }

  if (i_xmargs->isOptWindowed()) {
    m_windowed = true;
  }

  if (i_xmargs->isOptFs()) {
    m_windowed = false;
  }

  if (i_xmargs->isOptNoExts()) {
    m_glExts = false;
  }

  if (i_xmargs->isOptNoVOBS()) {
    m_glVOBS = false;
  }

  if (i_xmargs->isOptDrawlib()) {
    m_drawlib = i_xmargs->getOpt_drawlib_lib();
  }

  if (i_xmargs->isOptNoWWW()) {
    m_www = false;
  }

  if (i_xmargs->isOptBenchmark()) {
    m_benchmark = true;
  }

  if (i_xmargs->isOptDebug()) {
    m_debug = true;
  }

  if (i_xmargs->isOptSqlTrace()) {
    m_sqlTrace = true;
  }

  if (i_xmargs->isOptProfile()) {
    m_profile = i_xmargs->getOpt_profile_value();
  }

  if (i_xmargs->isOptGDebug()) {
    m_gdebug = true;
    m_gdebug_file = i_xmargs->getOpt_gdebug_file();
  }

  if (i_xmargs->isOptTimedemo()) {
    m_timedemo = true;
  }

  if (i_xmargs->isOptFps()) {
    m_fps = true;
  }

  if (i_xmargs->isOptUgly()) {
    m_ugly = true;
  }

  if (i_xmargs->isOptNoLog()) {
    m_noLog = true;
  }

  if (i_xmargs->isOptTestTheme()) {
    m_testTheme = true;
  }

  if (i_xmargs->isOptNoSound()) {
    m_enableAudio = false;
  }

  if (i_xmargs->isOptVideoRecording()) {
    m_enableVideoRecording = true;
    m_videoRecordName = i_xmargs->getOptVideoRecording_name();
  }

  if (i_xmargs->isOptVideoRecordingDivision()) {
    m_videoRecordingDivision = i_xmargs->getOptVideoRecordingDivision_value();
  } else {
    m_videoRecordingDivision = VR_DEFAULT_DIVISION;
  }

  if (i_xmargs->isOptVideoRecordingFramerate()) {
    m_videoRecordingFramerate = i_xmargs->getOptVideoRecordingFramerate_value();
  } else {
    m_videoRecordingFramerate = VR_DEFAULT_FRAMERATE;
  }

  if (i_xmargs->isOptVideoRecordingStartTime()) {
    m_videoRecordingStartTime = i_xmargs->getOptVideoRecordingStartTime_value();
  }

  if (i_xmargs->isOptVideoRecordingEndTime()) {
    m_videoRecordingEndTime = i_xmargs->getOptVideoRecordingEndTime_value();
  }

  if (i_xmargs->isOptHidePlayingInformation()) {
    m_hidePlayingInformation = true;
  }

  if (i_xmargs->isOptForceChildrenCompliant()) {
    m_forceChildrenCompliant = true;
  }

  if (i_xmargs->isOptClientConnectAtStartup()) {
    m_clientConnectAtStartup = true;
  }

  if (i_xmargs->isOptAdminMode()) {
    m_adminMode = true;
  }
}

void XMSession::loadConfig(UserConfig *config, bool loadProfile) {
  if (loadProfile)
    m_profile = config->getString("DefaultProfile");

  m_resolutionWidth = config->getInteger("DisplayWidth");
  m_resolutionHeight = config->getInteger("DisplayHeight");
  m_bpp = config->getInteger("DisplayBPP");
  m_maxRenderFps = config->getInteger("DisplayMaxRenderFPS");
  m_windowed = config->getBool("DisplayWindowed");
  m_drawlib = config->getString("DrawLib");
  m_useThemeCursor = config->getBool("UseThemeCursor");

  m_screenshotFormat = config->getString("ScreenshotFormat");
  m_storeReplays = config->getBool("StoreReplays");
  m_replayFrameRate = config->getFloat("ReplayFrameRate");

  m_uploadHighscoreUrl = config->getString("WebHighscoreUploadURL");
  m_webThemesURL = config->getString("WebThemesURL");
  m_webThemesURLBase = config->getString("WebThemesURLBase");
  m_webLevelsUrl = config->getString("WebLevelsURL");
  m_uploadDbSyncUrl = config->getString("WebDbSyncUploadURL");
}

void XMSession::loadProfile(const std::string &i_id_profile, xmDatabase *pDb) {
  m_sitekey = pDb->getXmDbSiteKey();
  m_www = pDb->config_getBool(i_id_profile, "WebHighscores", m_www);
  m_www_password =
    pDb->config_getString(i_id_profile, "WWWPassword", m_www_password);
  m_theme = pDb->config_getString(i_id_profile, "Theme", m_theme);
  m_language = pDb->config_getString(i_id_profile, "Language", m_language);
  m_quickStartQualityMIN = pDb->config_getInteger(
    i_id_profile, "QSQualityMIN", m_quickStartQualityMIN);
  m_quickStartQualityMAX = pDb->config_getInteger(
    i_id_profile, "QSQualityMAX", m_quickStartQualityMAX);
  m_quickStartDifficultyMIN = pDb->config_getInteger(
    i_id_profile, "QSDifficultyMIN", m_quickStartDifficultyMIN);
  m_quickStartDifficultyMAX = pDb->config_getInteger(
    i_id_profile, "QSDifficultyMAX", m_quickStartDifficultyMAX);
  m_enableAudio =
    pDb->config_getBool(i_id_profile, "AudioEnable", m_enableAudio);
  m_audioSampleRate =
    pDb->config_getInteger(i_id_profile, "AudioSampleRate", m_audioSampleRate);
  m_audioSampleBits =
    pDb->config_getInteger(i_id_profile, "AudioSampleBits", m_audioSampleBits);
  m_audioChannels = pDb->config_getString(
                      i_id_profile,
                      "AudioChannels",
                      DEFAULT_AUDIOCHANNELS == 1 ? "Mono" : "Stereo") == "Mono"
                      ? 1
                      : 2;
  m_enableAudioEngine =
    pDb->config_getBool(i_id_profile, "EngineSoundEnable", m_enableAudioEngine);
  m_autosaveHighscoreReplays = pDb->config_getBool(
    i_id_profile, "AutosaveHighscoreReplays", m_autosaveHighscoreReplays);
  m_notifyAtInit =
    pDb->config_getBool(i_id_profile, "NotifyAtInit", m_notifyAtInit);

  m_showMinimap =
    pDb->config_getBool(i_id_profile, "ShowMiniMap", m_showMinimap);
  m_showEngineCounter =
    pDb->config_getBool(i_id_profile, "ShowEngineCounter", m_showEngineCounter);
  m_enableContextHelp =
    pDb->config_getBool(i_id_profile, "ContextHelp", m_enableContextHelp);
  m_enableMenuMusic =
    pDb->config_getBool(i_id_profile, "MenuMusic", m_enableMenuMusic);
  m_enableGameMusic =
    pDb->config_getBool(i_id_profile, "GameMusic", m_enableGameMusic);
  m_enableInitZoom =
    pDb->config_getBool(i_id_profile, "InitZoom", m_enableInitZoom);
  m_enableActiveZoom =
    pDb->config_getBool(i_id_profile, "CameraActiveZoom", m_enableActiveZoom);
  m_enableTrailCam =
    pDb->config_getBool(i_id_profile, "CameraTrailCam", m_enableTrailCam);
  m_enableDeadAnimation =
    pDb->config_getBool(i_id_profile, "DeathAnim", m_enableDeadAnimation);
  m_checkNewLevelsAtStartup = pDb->config_getBool(
    i_id_profile, "CheckNewLevelsAtStartup", m_checkNewLevelsAtStartup);
  m_checkNewHighscoresAtStartup = pDb->config_getBool(
    i_id_profile, "CheckHighscoresAtStartup", m_checkNewHighscoresAtStartup);
  m_showHighscoreInGame = pDb->config_getBool(
    i_id_profile, "ShowInGameWorldRecord", m_showHighscoreInGame);
  m_showNextMedalInGame = pDb->config_getBool(
    i_id_profile, "ShowInGameNextMedal", m_showNextMedalInGame);
  m_webConfAtInit =
    pDb->config_getBool(i_id_profile, "WebConfAtInit", m_webConfAtInit);
  m_useCrappyPack =
    pDb->config_getBool(i_id_profile, "UseCrappyPack", m_useCrappyPack);
  m_useChildrenCompliant = pDb->config_getBool(
    i_id_profile, "UseChildrenCompliant", m_useChildrenCompliant);
  m_enablePermanentConsole = pDb->config_getBool(
    i_id_profile, "enablePermanentConsole", m_enablePermanentConsole);
  m_showGameInformationInConsole =
    pDb->config_getBool(i_id_profile,
                        "showGameInformationInConsole",
                        m_showGameInformationInConsole);
  m_consoleSize =
    pDb->config_getInteger(i_id_profile, "consoleSize", m_consoleSize);
  m_enableGhosts =
    pDb->config_getBool(i_id_profile, "EnableGhost", m_enableGhosts);
  m_disableAnimations =
    pDb->config_getBool(i_id_profile, "disableAnimations", m_disableAnimations);
  m_ghostStrategy_MYBEST = pDb->config_getBool(
    i_id_profile, "GhostStrategy_MYBEST", m_ghostStrategy_MYBEST);
  m_ghostStrategy_THEBEST = pDb->config_getBool(
    i_id_profile, "GhostStrategy_THEBEST", m_ghostStrategy_THEBEST);
  m_ghostStrategy_BESTOFREFROOM = pDb->config_getBool(
    i_id_profile, "GhostStrategy_BESTOFREFROOM", m_ghostStrategy_BESTOFREFROOM);
  m_ghostStrategy_BESTOFOTHERROOMS =
    pDb->config_getBool(i_id_profile,
                        "GhostStrategy_BESTOFOTHERROOMS",
                        m_ghostStrategy_BESTOFOTHERROOMS);
  m_showGhostTimeDifference = pDb->config_getBool(
    i_id_profile, "ShowGhostTimeDiff", m_showGhostTimeDifference);
  m_showGhostsInfos =
    pDb->config_getBool(i_id_profile, "DisplayGhostInfo", m_showGhostsInfos);
  m_showBikersArrows =
    pDb->config_getBool(i_id_profile, "DisplayBikerArrow", m_showBikersArrows);
  m_ghostMotionBlur =
    pDb->config_getBool(i_id_profile, "GhostMotionBlur", m_ghostMotionBlur);
  m_hideGhosts = pDb->config_getBool(i_id_profile, "HideGhosts", m_hideGhosts);
  m_multiStopWhenOneFinishes = pDb->config_getBool(
    i_id_profile, "MultiStopWhenOneFinishes", m_multiStopWhenOneFinishes);
  m_dbsynchronizeOnQuit = pDb->config_getBool(
    i_id_profile, "DbSynchronizeOnQuit", m_dbsynchronizeOnQuit);
  m_enableJoysticks =
    pDb->config_getBool(i_id_profile, "EnableJoysticks", m_enableJoysticks);
  m_beatingMode =
    pDb->config_getBool(i_id_profile, "BeatingMode", m_beatingMode);
  m_hideSpritesUgly =
    pDb->config_getBool(i_id_profile, "HideSpritesUgly", m_hideSpritesUgly);
  m_hideSpritesMinimap = pDb->config_getBool(
    i_id_profile, "HideSpritesMinimap", m_hideSpritesMinimap);
  m_webForms = pDb->config_getBool(i_id_profile, "WebForms", m_webForms);

  m_serverStartAtStartup = pDb->config_getBool(
    i_id_profile, "ServerStartAtStartup", m_serverStartAtStartup);
  m_clientConnectAtStartup = pDb->config_getBool(
    i_id_profile, "ClientConnectAtStartup", m_clientConnectAtStartup);
  m_serverPort =
    pDb->config_getInteger(i_id_profile, "ServerPort", m_serverPort);
  m_serverMaxClients = pDb->config_getInteger(
    i_id_profile, "ServerMaxClients", m_serverMaxClients);
  m_clientServerName =
    pDb->config_getString(i_id_profile, "ClientServerName", m_clientServerName);
  m_clientServerPort = pDb->config_getInteger(
    i_id_profile, "ClientServerPort", m_clientServerPort);
  m_clientFramerateUpload = pDb->config_getInteger(
    i_id_profile, "ClientFramerateUpload", m_clientFramerateUpload);
  m_musicOnAllLevels =
    pDb->config_getBool(i_id_profile, "MusicOnAllLevels", m_musicOnAllLevels);
  m_clientGhostMode =
    pDb->config_getBool(i_id_profile, "ClientGhostMode", m_clientGhostMode);

  m_nbRoomsEnabled = pDb->config_getInteger(
    i_id_profile, "WebHighscoresNbRooms", m_nbRoomsEnabled);
  if (m_nbRoomsEnabled < 1) {
    m_nbRoomsEnabled = 1;
  }
  if (m_nbRoomsEnabled > ROOMS_NB_MAX) {
    m_nbRoomsEnabled = ROOMS_NB_MAX;
  }
  for (unsigned int i = 0; i < ROOMS_NB_MAX; i++) {
    if (i == 0) {
      m_idRoom[i] =
        pDb->config_getString(i_id_profile, "WebHighscoresIdRoom", m_idRoom[i]);
    } else {
      std::ostringstream v_strRoom;
      v_strRoom << i;
      m_idRoom[i] = pDb->config_getString(
        i_id_profile, "WebHighscoresIdRoom" + v_strRoom.str(), m_idRoom[i]);
    }
  }

  m_proxySettings.setPort(
    pDb->config_getInteger(i_id_profile, "ProxyPort", DEFAULT_PROXY_PORT));
  m_proxySettings.setType(
    pDb->config_getString(i_id_profile, "ProxyType", DEFAULT_PROXY_TYPE));
  m_proxySettings.setServer(
    pDb->config_getString(i_id_profile, "ProxyServer", DEFAULT_PROXY_SERVER));
  m_proxySettings.setAuthentification(
    pDb->config_getString(
      i_id_profile, "ProxyAuthUser", DEFAULT_PROXY_AUTHUSER),
    pDb->config_getString(i_id_profile, "ProxyAuthPwd", DEFAULT_PROXY_AUTHPWD));

  std::string v_menuGraphics =
    pDb->config_getString(i_id_profile, "MenuGraphics", "High");
  if (v_menuGraphics == "Low")
    m_menuGraphics = GFX_LOW;
  if (v_menuGraphics == "Medium")
    m_menuGraphics = GFX_MEDIUM;
  if (v_menuGraphics == "High")
    m_menuGraphics = GFX_HIGH;

  std::string v_gameGraphics = pDb->config_getString(
    i_id_profile,
    "GameGraphics",
    DEFAULT_MENUGRAPHICS == GFX_HIGH
      ? "High"
      : (DEFAULT_MENUGRAPHICS == GFX_MEDIUM ? "Medium" : "Low"));
  if (v_gameGraphics == "Low")
    m_gameGraphics = GFX_LOW;
  if (v_gameGraphics == "Medium")
    m_gameGraphics = GFX_MEDIUM;
  if (v_gameGraphics == "High")
    m_gameGraphics = GFX_HIGH;
}

int XMSession::dbSync(xmDatabase *pDb, const std::string &i_id_profile) {
  return pDb->config_getInteger(i_id_profile, "dbSync", 0);
}

void XMSession::setDbSync(xmDatabase *pDb,
                          const std::string &i_id_profile,
                          int i_dbSync) {
  XMSession::propagate(
    this,
    new TFunctor1A2ARef3A<XMSession, xmDatabase *, std::string, int>(
      &XMSession::setDbSync, pDb, i_id_profile, i_dbSync));
  pDb->config_setInteger(i_id_profile, "dbSync", i_dbSync);
}

int XMSession::dbSyncServer(xmDatabase *pDb, const std::string &i_id_profile) {
  return pDb->config_getInteger(i_id_profile, "dbSyncServer", 0);
}

void XMSession::setDbSyncServer(xmDatabase *pDb,
                                const std::string &i_id_profile,
                                int i_dbSyncServer) {
  XMSession::propagate(
    this,
    new TFunctor1A2ARef3A<XMSession, xmDatabase *, std::string, int>(
      &XMSession::setDbSyncServer, pDb, i_id_profile, i_dbSyncServer));
  pDb->config_setInteger(i_id_profile, "dbSyncServer", i_dbSyncServer);
}

void XMSession::save(UserConfig *v_config, xmDatabase *pDb) {
  v_config->setString("DefaultProfile", m_profile);

  v_config->setInteger("DisplayWidth", m_resolutionWidth);
  v_config->setInteger("DisplayHeight", m_resolutionHeight);
  v_config->setInteger("DisplayBPP", m_bpp);
  v_config->setInteger("DisplayMaxRenderFPS", m_maxRenderFps);
  v_config->setBool("DisplayWindowed", m_windowed);
  v_config->setBool("UseThemeCursor", m_useThemeCursor);

  v_config->setString("WebThemesURL", m_webThemesURL);
  v_config->setString("WebThemesURLBase", m_webThemesURLBase);
  v_config->setString("WebHighscoreUploadURL", m_uploadHighscoreUrl);
  v_config->setString("WebLevelsURL", m_webLevelsUrl);
  v_config->setString("WebDbSyncUploadURL", m_uploadDbSyncUrl);

  v_config->setFloat("ReplayFrameRate", m_replayFrameRate);
  v_config->setBool("StoreReplays", m_storeReplays);

  saveProfile(pDb);
}

void XMSession::saveProfile(xmDatabase *pDb) {
  if (m_profile == "") { /* don't save */
    return;
  }

  pDb->config_setValue_begin();

  pDb->config_setBool(m_profile, "WebHighscores", m_www);
  pDb->config_setString(m_profile, "WWWPassword", m_www_password);
  pDb->config_setString(m_profile, "Theme", m_theme);
  pDb->config_setString(m_profile, "Language", m_language);
  pDb->config_setInteger(m_profile, "QSQualityMIN", m_quickStartQualityMIN);
  pDb->config_setInteger(m_profile, "QSQualityMAX", m_quickStartQualityMAX);
  pDb->config_setInteger(
    m_profile, "QSDifficultyMIN", m_quickStartDifficultyMIN);
  pDb->config_setInteger(
    m_profile, "QSDifficultyMAX", m_quickStartDifficultyMAX);
  pDb->config_setBool(m_profile, "AudioEnable", m_enableAudio);
  pDb->config_setInteger(m_profile, "AudioSampleRate", m_audioSampleRate);
  pDb->config_setInteger(m_profile, "AudioSampleBits", m_audioSampleBits);
  pDb->config_setString(
    m_profile, "AudioChannels", m_audioChannels == 1 ? "Mono" : "Stereo");
  pDb->config_setBool(m_profile, "EngineSoundEnable", m_enableAudioEngine);
  pDb->config_setBool(
    m_profile, "AutosaveHighscoreReplays", m_autosaveHighscoreReplays);
  pDb->config_setBool(m_profile, "NotifyAtInit", m_notifyAtInit);
  pDb->config_setBool(m_profile, "ShowMiniMap", m_showMinimap);
  pDb->config_setBool(m_profile, "ShowEngineCounter", m_showEngineCounter);
  pDb->config_setBool(m_profile, "ContextHelp", m_enableContextHelp);
  pDb->config_setBool(m_profile, "MenuMusic", m_enableMenuMusic);
  pDb->config_setBool(m_profile, "GameMusic", m_enableGameMusic);
  pDb->config_setBool(m_profile, "InitZoom", m_enableInitZoom);
  pDb->config_setBool(m_profile, "CameraActiveZoom", m_enableActiveZoom);
  pDb->config_setBool(m_profile, "CameraTrailCam", m_enableTrailCam);
  pDb->config_setBool(m_profile, "DeathAnim", m_enableDeadAnimation);
  pDb->config_setBool(
    m_profile, "enablePermanentConsole", m_enablePermanentConsole);
  pDb->config_setBool(
    m_profile, "showGameInformationInConsole", m_showGameInformationInConsole);
  pDb->config_setInteger(m_profile, "consoleSize", m_consoleSize);
  pDb->config_setBool(
    m_profile, "CheckNewLevelsAtStartup", m_checkNewLevelsAtStartup);
  pDb->config_setBool(
    m_profile, "CheckHighscoresAtStartup", m_checkNewHighscoresAtStartup);
  pDb->config_setBool(
    m_profile, "ShowInGameWorldRecord", m_showHighscoreInGame);
  pDb->config_setBool(m_profile, "ShowInGameNextMedal", m_showNextMedalInGame);
  pDb->config_setBool(m_profile, "WebConfAtInit", m_webConfAtInit);
  pDb->config_setBool(m_profile, "UseCrappyPack", m_useCrappyPack);
  pDb->config_setBool(
    m_profile, "UseChildrenCompliant", m_useChildrenCompliant);
  pDb->config_setBool(m_profile, "EnableGhost", m_enableGhosts);
  pDb->config_setBool(m_profile, "disableAnimations", m_disableAnimations);
  pDb->config_setBool(
    m_profile, "GhostStrategy_MYBEST", m_ghostStrategy_MYBEST);
  pDb->config_setBool(
    m_profile, "GhostStrategy_THEBEST", m_ghostStrategy_THEBEST);
  pDb->config_setBool(
    m_profile, "GhostStrategy_BESTOFREFROOM", m_ghostStrategy_BESTOFREFROOM);
  pDb->config_setBool(m_profile,
                      "GhostStrategy_BESTOFOTHERROOMS",
                      m_ghostStrategy_BESTOFOTHERROOMS);
  pDb->config_setBool(
    m_profile, "ShowGhostTimeDiff", m_showGhostTimeDifference);
  pDb->config_setBool(m_profile, "DisplayGhostInfo", m_showGhostsInfos);
  pDb->config_setBool(m_profile, "DisplayBikerArrow", m_showBikersArrows);
  pDb->config_setBool(m_profile, "HideGhosts", m_hideGhosts);
  pDb->config_setBool(m_profile, "GhostMotionBlur", m_ghostMotionBlur);
  pDb->config_setBool(
    m_profile, "MultiStopWhenOneFinishes", m_multiStopWhenOneFinishes);
  pDb->config_setBool(m_profile, "DbSynchronizeOnQuit", m_dbsynchronizeOnQuit);
  pDb->config_setBool(m_profile, "EnableJoysticks", m_enableJoysticks);
  pDb->config_setBool(m_profile, "BeatingMode", m_beatingMode);
  pDb->config_setBool(m_profile, "HideSpritesUgly", m_hideSpritesUgly);
  pDb->config_setBool(m_profile, "HideSpritesMinimap", m_hideSpritesMinimap);
  pDb->config_setBool(m_profile, "WebForms", m_webForms);

  pDb->config_setBool(
    m_profile, "ServerStartAtStartup", m_serverStartAtStartup);
  pDb->config_setBool(
    m_profile, "ClientConnectAtStartup", m_clientConnectAtStartup);
  pDb->config_setInteger(m_profile, "ServerPort", m_serverPort);
  pDb->config_setInteger(m_profile, "ServerMaxClients", m_serverMaxClients);
  pDb->config_setString(m_profile, "ClientServerName", m_clientServerName);
  pDb->config_setInteger(m_profile, "ClientServerPort", m_clientServerPort);
  pDb->config_setInteger(
    m_profile, "ClientFramerateUpload", m_clientFramerateUpload);
  pDb->config_setBool(m_profile, "MusicOnAllLevels", m_musicOnAllLevels);
  pDb->config_setBool(m_profile, "ClientGhostMode", m_clientGhostMode);

  pDb->config_setString(m_profile,
                        "MenuGraphics",
                        m_menuGraphics == GFX_LOW      ? "Low"
                        : m_menuGraphics == GFX_MEDIUM ? "Medium"
                                                       : "High");
  pDb->config_setString(m_profile,
                        "GameGraphics",
                        m_gameGraphics == GFX_LOW      ? "Low"
                        : m_gameGraphics == GFX_MEDIUM ? "Medium"
                                                       : "High");
  pDb->config_setString(m_profile, "ProxyType", proxySettings()->getTypeStr());
  pDb->config_setString(m_profile, "ProxyServer", proxySettings()->getServer());
  pDb->config_setString(
    m_profile, "ProxyAuthUser", proxySettings()->getAuthentificationUser());
  pDb->config_setString(
    m_profile, "ProxyAuthPwd", proxySettings()->getAuthentificationPassword());
  pDb->config_setInteger(m_profile, "ProxyPort", proxySettings()->getPort());

  pDb->config_setInteger(m_profile, "WebHighscoresNbRooms", m_nbRoomsEnabled);
  for (unsigned int i = 0; i < ROOMS_NB_MAX; i++) {
    if (i == 0) {
      pDb->config_setString(m_profile, "WebHighscoresIdRoom", m_idRoom[i]);
    } else {
      std::ostringstream v_strRoom;
      v_strRoom << i;
      pDb->config_setString(
        m_profile, "WebHighscoresIdRoom" + v_strRoom.str(), m_idRoom[i]);
    }
  }

  pDb->config_setValue_end();
}

bool XMSession::isVerbose() const {
  return m_verbose;
}

int XMSession::resolutionWidth() const {
  return m_resolutionWidth;
}

int XMSession::resolutionHeight() const {
  return m_resolutionHeight;
}

int XMSession::maxRenderFps() const {
  return m_maxRenderFps;
}

bool XMSession::windowed() const {
  return m_windowed;
}

void XMSession::setResolutionWidth(int i_value) {
  PROPAGATE(XMSession, setResolutionWidth, i_value, int);
  m_resolutionWidth = i_value;
}

void XMSession::setResolutionHeight(int i_value) {
  PROPAGATE(XMSession, setResolutionHeight, i_value, int);
  m_resolutionHeight = i_value;
}

void XMSession::setMaxRenderFps(int i_value) {
  PROPAGATE(XMSession, setMaxRenderFps, i_value, int);
  m_maxRenderFps = i_value;
}

void XMSession::setWindowed(bool i_value) {
  PROPAGATE(XMSession, setWindowed, i_value, bool);
  m_windowed = i_value;
}

bool XMSession::useThemeCursor() const {
  return m_useThemeCursor;
}

void XMSession::setUseThemeCursor(bool i_value) {
  PROPAGATE(XMSession, setUseThemeCursor, i_value, bool);
  m_useThemeCursor = i_value;
}

bool XMSession::glExts() const {
  return m_glExts;
}

bool XMSession::glVOBS() const {
  return m_glVOBS;
}

std::string XMSession::drawlib() const {
  return m_drawlib;
}

bool XMSession::www() const {
  return m_www;
}

void XMSession::setWWW(bool i_value) {
  PROPAGATE(XMSession, setWWW, i_value, bool);
  m_www = i_value;
}

bool XMSession::benchmark() const {
  return m_benchmark;
}

bool XMSession::debug() const {
  return m_debug;
}

bool XMSession::sqlTrace() const {
  return m_sqlTrace;
}

std::string XMSession::profile() const {
  return m_profile;
}

void XMSession::setProfile(const std::string &i_profile) {
  PROPAGATE_REF(XMSession, setProfile, i_profile, std::string);
  m_profile = i_profile;
}

std::string XMSession::sitekey() const {
  return m_sitekey;
}

std::string XMSession::wwwPassword() const {
  return m_www_password;
}

void XMSession::setWwwPassword(const std::string &i_password) {
  PROPAGATE_REF(XMSession, setWwwPassword, i_password, std::string);
  m_www_password = i_password;
}

bool XMSession::gDebug() const {
  return m_gdebug;
}

std::string XMSession::gDebugFile() const {
  return m_gdebug_file;
}

bool XMSession::timedemo() const {
  return m_timedemo;
}

bool XMSession::fps() const {
  return m_fps;
}

void XMSession::setFps(bool i_value) {
  PROPAGATE(XMSession, setFps, i_value, bool);
  m_fps = i_value;
}

bool XMSession::ugly() const {
  return m_ugly;
}

bool XMSession::uglyOver() const {
  return m_uglyOver;
}

bool XMSession::noLog() const {
  return m_noLog;
}

bool XMSession::testTheme() const {
  return m_testTheme;
}

void XMSession::setUgly(bool i_value) {
  PROPAGATE(XMSession, setUgly, i_value, bool);
  m_ugly = i_value;
}

void XMSession::setUglyOver(bool i_value) {
  PROPAGATE(XMSession, setUglyOver, i_value, bool);
  m_uglyOver = i_value;
}

void XMSession::setTestTheme(bool i_value) {
  PROPAGATE(XMSession, setTestTheme, i_value, bool);
  m_testTheme = i_value;
}

bool XMSession::hideSpritesUgly() const {
  return m_hideSpritesUgly;
}

void XMSession::setHideSpritesUgly(bool i_value) {
  PROPAGATE(XMSession, setHideSpritesUgly, i_value, bool);
  m_hideSpritesUgly = i_value;
}

bool XMSession::hideSpritesMinimap() const {
  return m_hideSpritesMinimap;
}

void XMSession::setHideSpritesMinimap(bool i_value) {
  PROPAGATE(XMSession, setHideSpritesMinimap, i_value, bool);
  m_hideSpritesMinimap = i_value;
}

bool XMSession::ghostStrategy_MYBEST() const {
  return m_ghostStrategy_MYBEST;
}

void XMSession::setGhostStrategy_MYBEST(bool i_value) {
  PROPAGATE(XMSession, setGhostStrategy_MYBEST, i_value, bool);
  m_ghostStrategy_MYBEST = i_value;
}

bool XMSession::ghostStrategy_THEBEST() const {
  return m_ghostStrategy_THEBEST;
}

void XMSession::setGhostStrategy_THEBEST(bool i_value) {
  PROPAGATE(XMSession, setGhostStrategy_THEBEST, i_value, bool);
  m_ghostStrategy_THEBEST = i_value;
}

bool XMSession::ghostStrategy_BESTOFREFROOM() const {
  return m_ghostStrategy_BESTOFREFROOM;
}

void XMSession::setGhostStrategy_BESTOFREFROOM(bool i_value) {
  PROPAGATE(XMSession, setGhostStrategy_BESTOFREFROOM, i_value, bool);
  m_ghostStrategy_BESTOFREFROOM = i_value;
}

bool XMSession::ghostStrategy_BESTOFOTHERROOMS() const {
  return m_ghostStrategy_BESTOFOTHERROOMS;
}

void XMSession::setGhostStrategy_BESTOFOTHERROOMS(bool i_value) {
  PROPAGATE(XMSession, setGhostStrategy_BESTOFOTHERROOMS, i_value, bool);
  m_ghostStrategy_BESTOFOTHERROOMS = i_value;
}

bool XMSession::autosaveHighscoreReplays() const {
  return m_autosaveHighscoreReplays;
}

void XMSession::setAutosaveHighscoreReplays(bool i_value) {
  PROPAGATE(XMSession, setAutosaveHighscoreReplays, i_value, bool);
  m_autosaveHighscoreReplays = i_value;
}

void XMSession::setEnableGhosts(bool i_value) {
  PROPAGATE(XMSession, setEnableGhosts, i_value, bool);
  m_enableGhosts = i_value;
}

bool XMSession::enableGhosts() const {
  return m_enableGhosts;
}

bool XMSession::disableAnimations() const {
  return m_disableAnimations;
}

bool XMSession::permanentConsole() const {
  return m_enablePermanentConsole;
}

bool XMSession::showGameInformationInConsole() const {
  return m_showGameInformationInConsole;
}

unsigned int XMSession::consoleSize() const {
  return m_consoleSize;
}

void XMSession::setEnableEngineSound(bool i_value) {
  PROPAGATE(XMSession, setEnableEngineSound, i_value, bool);
  m_enableEngineSound = i_value;
}

bool XMSession::enableEngineSound() const {
  return m_enableEngineSound;
}

bool XMSession::enableVideoRecording() const {
  return m_enableVideoRecording;
}

std::string XMSession::videoRecordName() const {
  return m_videoRecordName;
}

int XMSession::videoRecordingDivision() const {
  return m_videoRecordingDivision;
}

int XMSession::videoRecordingFramerate() const {
  return m_videoRecordingFramerate;
}

int XMSession::videoRecordingStartTime() {
  return m_videoRecordingStartTime;
}

int XMSession::videoRecordingEndTime() {
  return m_videoRecordingEndTime;
}

bool XMSession::hidePlayingInformation() {
  return m_hidePlayingInformation;
}

void XMSession::setShowEngineCounter(bool i_value) {
  PROPAGATE(XMSession, setShowEngineCounter, i_value, bool);
  m_showEngineCounter = i_value;
}

bool XMSession::showEngineCounter() const {
  return m_showEngineCounter;
}

void XMSession::setShowMinimap(bool i_value) {
  PROPAGATE(XMSession, setShowMinimap, i_value, bool);
  m_showMinimap = i_value;
}

bool XMSession::showMinimap() const {
  return m_showMinimap;
}

void XMSession::setMultiStopWhenOneFinishes(bool i_value) {
  PROPAGATE(XMSession, setMultiStopWhenOneFinishes, i_value, bool);
  m_multiStopWhenOneFinishes = i_value;
}

bool XMSession::MultiStopWhenOneFinishes() const {
  return m_multiStopWhenOneFinishes;
}

void XMSession::setEnableMenuMusic(bool i_value) {
  PROPAGATE(XMSession, setEnableMenuMusic, i_value, bool);
  m_enableMenuMusic = i_value;
}

bool XMSession::enableMenuMusic() const {
  return m_enableMenuMusic;
}

void XMSession::setEnableGameMusic(bool i_value) {
  PROPAGATE(XMSession, setEnableGameMusic, i_value, bool);
  m_enableGameMusic = i_value;
}

bool XMSession::enableGameMusic() const {
  return m_enableGameMusic;
}

void XMSession::setEnableInitZoom(bool i_value) {
  PROPAGATE(XMSession, setEnableInitZoom, i_value, bool);
  m_enableInitZoom = i_value;
}

bool XMSession::enableInitZoom() const {
  return m_enableInitZoom;
}

void XMSession::setEnableActiveZoom(bool i_value) {
  PROPAGATE(XMSession, setEnableActiveZoom, i_value, bool);
  m_enableActiveZoom = i_value;
}

bool XMSession::enableActiveZoom() const {
  return m_enableActiveZoom;
}

void XMSession::setEnableTrailCam(bool i_value) {
  PROPAGATE(XMSession, setEnableTrailCam, i_value, bool);
  m_enableTrailCam = i_value;
}

bool XMSession::enableTrailCam() const {
  return m_enableTrailCam;
}

void XMSession::setRenderGhostTrail(bool i_value) {
  PROPAGATE(XMSession, setRenderGhostTrail, i_value, bool);
  m_renderGhostTrail = i_value;
}

bool XMSession::renderGhostTrail() const {
  return m_renderGhostTrail;
}

void XMSession::setEnableDeadAnimation(bool i_value) {
  PROPAGATE(XMSession, setEnableDeadAnimation, i_value, bool);
  m_enableDeadAnimation = i_value;
}

bool XMSession::enableDeadAnimation() const {
  return m_enableDeadAnimation;
}

void XMSession::setMenuGraphics(GraphicsLevel i_value) {
  PROPAGATE(XMSession, setMenuGraphics, i_value, GraphicsLevel);
  m_menuGraphics = i_value;
}

GraphicsLevel XMSession::menuGraphics() const {
  return m_menuGraphics;
}

void XMSession::setGameGraphics(GraphicsLevel i_value) {
  PROPAGATE(XMSession, setGameGraphics, i_value, GraphicsLevel);
  m_gameGraphics = i_value;
}

GraphicsLevel XMSession::gameGraphics() const {
  return m_gameGraphics;
}

void XMSession::setQuickStartQualityMIN(int i_value) {
  PROPAGATE(XMSession, setQuickStartQualityMIN, i_value, int);
  m_quickStartQualityMIN = i_value;
}

int XMSession::quickStartQualityMIN() const {
  return m_quickStartQualityMIN;
}

void XMSession::setQuickStartQualityMAX(int i_value) {
  PROPAGATE(XMSession, setQuickStartQualityMAX, i_value, int);
  m_quickStartQualityMAX = i_value;
}

int XMSession::quickStartQualityMAX() const {
  return m_quickStartQualityMAX;
}

void XMSession::setQuickStartDifficultyMIN(int i_value) {
  PROPAGATE(XMSession, setQuickStartDifficultyMIN, i_value, int);
  m_quickStartDifficultyMIN = i_value;
}

int XMSession::quickStartDifficultyMIN() const {
  return m_quickStartDifficultyMIN;
}

void XMSession::setQuickStartDifficultyMAX(int i_value) {
  PROPAGATE(XMSession, setQuickStartDifficultyMAX, i_value, int);
  m_quickStartDifficultyMAX = i_value;
}

int XMSession::quickStartDifficultyMAX() const {
  return m_quickStartDifficultyMAX;
}

void XMSession::setMultiNbPlayers(int i_value) {
  PROPAGATE(XMSession, setMultiNbPlayers, i_value, int);
  m_multiNbPlayers = i_value;
}

int XMSession::multiNbPlayers() const {
  return m_multiNbPlayers;
}

void XMSession::setMultiGameMode(MultiGameMode i_value) {
  PROPAGATE(XMSession, setMultiGameMode, i_value, MultiGameMode);
  m_multiGameMode = i_value;
}

MultiGameMode XMSession::multiGameMode() const {
  return m_multiGameMode;
}

void XMSession::setMultiScenes(bool i_value) {
  PROPAGATE(XMSession, setMultiScenes, i_value, bool);
  m_multiScenes = i_value;
}

bool XMSession::multiScenes() const {
  return m_multiScenes;
}

void XMSession::setEnableContextHelp(bool i_value) {
  PROPAGATE(XMSession, setEnableContextHelp, i_value, bool);
  m_enableContextHelp = i_value;
}

bool XMSession::enableContextHelp() const {
  return m_enableContextHelp;
}

void XMSession::setTheme(const std::string &i_value) {
  PROPAGATE_REF(XMSession, setTheme, i_value, std::string);
  m_theme = i_value;
}

std::string XMSession::theme() const {
  return m_theme;
}

void XMSession::setEnableAudio(bool i_value) {
  PROPAGATE(XMSession, setEnableAudio, i_value, bool);
  m_enableAudio = i_value;
}

bool XMSession::enableAudio() const {
  return m_enableAudio;
}

void XMSession::setAudioSampleRate(int i_value) {
  PROPAGATE(XMSession, setAudioSampleRate, i_value, int);
  m_audioSampleRate = i_value;
}

int XMSession::audioSampleRate() const {
  return m_audioSampleRate;
}

void XMSession::setAudioSampleBits(int i_value) {
  PROPAGATE(XMSession, setAudioSampleBits, i_value, int);
  m_audioSampleBits = i_value;
}

int XMSession::audioSampleBits() const {
  return m_audioSampleBits;
}

void XMSession::setAudioChannels(int i_value) {
  PROPAGATE(XMSession, setAudioChannels, i_value, int);
  m_audioChannels = i_value;
}

int XMSession::audioChannels() const {
  return m_audioChannels;
}

void XMSession::setEnableAudioEngine(bool i_value) {
  PROPAGATE(XMSession, setEnableAudioEngine, i_value, bool);
  m_enableAudioEngine = i_value;
}

bool XMSession::enableAudioEngine() const {
  return m_enableAudioEngine;
}

void XMSession::setCheckNewLevelsAtStartup(bool i_value) {
  PROPAGATE(XMSession, setCheckNewLevelsAtStartup, i_value, bool);
  m_checkNewLevelsAtStartup = i_value;
}

bool XMSession::checkNewLevelsAtStartup() const {
  return m_checkNewLevelsAtStartup;
}

void XMSession::setCheckNewHighscoresAtStartup(bool i_value) {
  PROPAGATE(XMSession, setCheckNewHighscoresAtStartup, i_value, bool);
  m_checkNewHighscoresAtStartup = i_value;
}

bool XMSession::checkNewHighscoresAtStartup() const {
  return m_checkNewHighscoresAtStartup;
}

void XMSession::setShowHighscoreInGame(bool i_value) {
  PROPAGATE(XMSession, setShowHighscoreInGame, i_value, bool);
  m_showHighscoreInGame = i_value;
}

void XMSession::setNextMedalInGame(bool i_value) {
  PROPAGATE(XMSession, setNextMedalInGame, i_value, bool);
  m_showNextMedalInGame = i_value;
}

bool XMSession::showHighscoreInGame() const {
  return m_showHighscoreInGame;
}

bool XMSession::showNextMedalInGame() const {
  return m_showNextMedalInGame;
}

unsigned int XMSession::nbRoomsEnabled() const {
  return m_nbRoomsEnabled;
}

void XMSession::setNbRoomsEnabled(unsigned int i_value) {
  PROPAGATE(XMSession, setNbRoomsEnabled, i_value, unsigned int);
  m_nbRoomsEnabled = i_value;
}

void XMSession::setIdRoom(unsigned int i_number, const std::string &i_value) {
  XMSession::propagate(
    this,
    new TFunctor1A2ARef<XMSession, unsigned int, std::string>(
      &XMSession::setIdRoom, i_number, i_value));
  m_idRoom[i_number] = i_value;
}

std::string XMSession::idRoom(unsigned int i_number) const {
  return m_idRoom[i_number];
}

void XMSession::setShowGhostTimeDifference(bool i_value) {
  PROPAGATE(XMSession, setShowGhostTimeDifference, i_value, bool);
  m_showGhostTimeDifference = i_value;
}

bool XMSession::showGhostTimeDifference() const {
  return m_showGhostTimeDifference;
}

void XMSession::setGhostMotionBlur(bool i_value) {
  PROPAGATE(XMSession, setGhostMotionBlur, i_value, bool);
  m_ghostMotionBlur = i_value;
}

bool XMSession::ghostMotionBlur() const {
  return m_ghostMotionBlur;
}

void XMSession::setShowGhostsInfos(bool i_value) {
  PROPAGATE(XMSession, setShowGhostsInfos, i_value, bool);
  m_showGhostsInfos = i_value;
}

bool XMSession::showGhostsInfos() const {
  return m_showGhostsInfos;
}

void XMSession::setShowBikersArrows(bool i_value) {
  PROPAGATE(XMSession, setShowBikersArrows, i_value, bool);
  m_showBikersArrows = i_value;
}

bool XMSession::showBikersArrows() const {
  return m_showBikersArrows;
}

void XMSession::setHideGhosts(bool i_value) {
  PROPAGATE(XMSession, setHideGhosts, i_value, bool);
  m_hideGhosts = i_value;
}

bool XMSession::hideGhosts() const {
  return m_hideGhosts;
}

void XMSession::setDbsynchronizeOnQuit(bool i_value) {
  PROPAGATE(XMSession, setDbsynchronizeOnQuit, i_value, bool);
  m_dbsynchronizeOnQuit = i_value;
}

bool XMSession::dbsynchronizeOnQuit() const {
  return m_dbsynchronizeOnQuit;
}

float XMSession::replayFrameRate() const {
  return m_replayFrameRate;
}

std::string XMSession::webThemesURL() const {
  return m_webThemesURL;
}

std::string XMSession::webThemesURLBase() const {
  return m_webThemesURLBase;
}

std::string XMSession::webRoomsURL() const {
  return m_webRoomsURL;
}

void XMSession::setWebConfAtInit(bool i_value) {
  PROPAGATE(XMSession, setWebConfAtInit, i_value, bool);
  m_webConfAtInit = i_value;
}

bool XMSession::webConfAtInit() const {
  return m_webConfAtInit;
}

bool XMSession::storeReplays() const {
  return m_storeReplays;
}

bool XMSession::enableReplayInterpolation() const {
  return m_enableReplayInterpolation;
}

void XMSession::setEnableReplayInterpolation(bool i_value) {
  PROPAGATE(XMSession, setEnableReplayInterpolation, i_value, bool);
  m_enableReplayInterpolation = i_value;
}

std::string XMSession::uploadHighscoreUrl() const {
  return m_uploadHighscoreUrl;
}

std::string XMSession::screenshotFormat() const {
  return m_screenshotFormat;
}

std::string XMSession::language() const {
  return m_language;
}

void XMSession::setLanguage(const std::string &i_value) {
  PROPAGATE_REF(XMSession, setLanguage, i_value, std::string);
  m_language = i_value;
}

void XMSession::setNotifyAtInit(bool i_value) {
  PROPAGATE(XMSession, setNotifyAtInit, i_value, bool);
  m_notifyAtInit = i_value;
}

bool XMSession::notifyAtInit() const {
  return m_notifyAtInit;
}

std::string XMSession::webLevelsUrl() const {
  return m_webLevelsUrl;
}

std::string XMSession::uploadDbSyncUrl() const {
  return m_uploadDbSyncUrl;
}

bool XMSession::mirrorMode() const {
  return m_mirrorMode;
}

void XMSession::setMirrorMode(bool i_value) {
  PROPAGATE(XMSession, setMirrorMode, i_value, bool);
  m_mirrorMode = i_value;
}

bool XMSession::useCrappyPack() const {
  return m_useCrappyPack;
}

void XMSession::setUseCrappyPack(bool i_value) {
  PROPAGATE(XMSession, setUseCrappyPack, i_value, bool);
  m_useCrappyPack = i_value;
}

bool XMSession::useChildrenCompliant() const {
  if (m_forceChildrenCompliant) {
    return true;
  }

  return m_useChildrenCompliant;
}

void XMSession::setChildrenCompliant(bool i_value) {
  PROPAGATE(XMSession, setChildrenCompliant, i_value, bool);
  m_useChildrenCompliant = i_value;
}

bool XMSession::forceChildrenCompliant() const {
  return m_forceChildrenCompliant;
}

bool XMSession::adminMode() const {
  return m_adminMode;
}

void XMSession::setAdminMode(bool i_value) {
  m_adminMode = i_value;
}

bool XMSession::enableJoysticks() const {
  return m_enableJoysticks;
}

void XMSession::setEnableJoysticks(bool i_value) {
  PROPAGATE(XMSession, setEnableJoysticks, i_value, bool);
  m_enableJoysticks = i_value;
}

void XMSession::setBeatingMode(bool i_value) {
  PROPAGATE(XMSession, setBeatingMode, i_value, bool);
  m_beatingMode = i_value;
}

void XMSession::setDisableAnimations(bool i_value) {
  PROPAGATE(XMSession, setDisableAnimations, i_value, bool);
  m_disableAnimations = i_value;
}

void XMSession::setPermanentConsole(bool i_value) {
  PROPAGATE(XMSession, setPermanentConsole, i_value, bool);
  m_enablePermanentConsole = i_value;
}

void XMSession::setShowGameInformationInConsole(bool i_value) {
  PROPAGATE(XMSession, setShowGameInformationInConsole, i_value, bool);
  m_showGameInformationInConsole = i_value;
}

void XMSession::setConsoleSize(unsigned int i_value) {
  PROPAGATE(XMSession, setConsoleSize, i_value, unsigned int);
  m_consoleSize = i_value;
}

bool XMSession::beatingMode() const {
  return m_beatingMode;
}

void XMSession::setWebForms(bool i_value) {
  PROPAGATE(XMSession, setWebForms, i_value, bool);
  m_webForms = i_value;
}

bool XMSession::webForms() const {
  return m_webForms;
}

bool XMSession::serverStartAtStartup() const {
  return m_serverStartAtStartup;
}

void XMSession::setServerStartAtStartup(bool i_value) {
  PROPAGATE(XMSession, setServerStartAtStartup, i_value, bool);
  m_serverStartAtStartup = i_value;
}

bool XMSession::clientConnectAtStartup() const {
  return m_clientConnectAtStartup;
}

void XMSession::setClientConnectAtStartup(bool i_value) {
  PROPAGATE(XMSession, setClientConnectAtStartup, i_value, bool);
  m_clientConnectAtStartup = i_value;
}

int XMSession::serverPort() const {
  return m_serverPort;
}

void XMSession::setServerPort(int i_value) {
  PROPAGATE(XMSession, setServerPort, i_value, int);
  m_serverPort = i_value;
}

unsigned int XMSession::serverMaxClients() const {
  return m_serverMaxClients;
}

void XMSession::setServerMaxClients(unsigned int i_value) {
  PROPAGATE(XMSession, setServerMaxClients, i_value, unsigned int);
  m_serverMaxClients = i_value;
}

std::string XMSession::clientServerName() const {
  return m_clientServerName;
}

void XMSession::setClientServerName(const std::string &i_value) {
  PROPAGATE_REF(XMSession, setClientServerName, i_value, std::string);
  m_clientServerName = i_value;
}

bool XMSession::clientGhostMode() const {
  return m_clientGhostMode;
}

void XMSession::setClientGhostMode(bool i_value) {
  PROPAGATE(XMSession, setClientGhostMode, i_value, bool);
  m_clientGhostMode = i_value;
}

int XMSession::clientServerPort() const {
  return m_clientServerPort;
}

void XMSession::setClientServerPort(int i_value) {
  PROPAGATE(XMSession, setClientServerPort, i_value, int);
  m_clientServerPort = i_value;
}

int XMSession::clientFramerateUpload() const {
  return m_clientFramerateUpload;
}

void XMSession::setClientFramerateUpload(int i_value) {
  PROPAGATE(XMSession, setClientFramerateUpload, i_value, int);
  m_clientFramerateUpload = i_value;
}

bool XMSession::musicOnAllLevels() const {
  return m_musicOnAllLevels;
}

void XMSession::setMusicOnAllLevels(bool i_value) {
  PROPAGATE(XMSession, setMusicOnAllLevels, i_value, bool);
  m_musicOnAllLevels = i_value;
}

ProxySettings *XMSession::proxySettings() {
  return &m_proxySettings;
}

void XMSession::setProxySettings(const ProxySettings &i_value) {
  m_proxySettings = i_value;
}

void XMSession::markProxyUpdated() {
  PROPAGATE_REF(XMSession, setProxySettings, m_proxySettings, ProxySettings);
}

ProxySettings::ProxySettings() {
  setDefault();
}

void ProxySettings::operator=(const ProxySettings &i_copy) {
  m_useProxy = i_copy.m_useProxy;
  m_server = i_copy.m_server;
  m_port = i_copy.m_port;
  m_type = i_copy.m_type;
  m_authUser = i_copy.m_authUser;
  m_authPassword = i_copy.m_authPassword;
}

void ProxySettings::setServer(std::string p_server) {
  m_server = p_server;
}

void ProxySettings::setPort(long p_port) {
  m_port = p_port;
}

void ProxySettings::setType(const std::string &p_type) {
  m_useProxy = true;

  if (p_type == "SOCKS4") {
    m_type = CURLPROXY_SOCKS4;
  } else if (p_type == "SOCKS5") {
    m_type = CURLPROXY_SOCKS5;
  } else if (p_type == "HTTP") {
    m_type = CURLPROXY_HTTP;
  } else {
    m_useProxy = false;
  }
}

void ProxySettings::setAuthentification(std::string p_user,
                                        std::string p_password) {
  m_authUser = p_user;
  m_authPassword = p_password;
}

void ProxySettings::setDefault() {
  setDefaultServer();
  setDefaultPort();
  setDefaultType();
  setDefaultAuthentification();
}

void ProxySettings::setDefaultServer() {
  m_server = "";
}

void ProxySettings::setDefaultPort() {
  m_port = -1;
}

void ProxySettings::setDefaultType() {
  m_useProxy = false;
  m_type = CURLPROXY_HTTP;
}

void ProxySettings::setDefaultAuthentification() {
  m_authUser = "";
  m_authPassword = "";
}

std::string ProxySettings::getServer() const {
  return m_server;
}

long ProxySettings::getPort() const {
  return m_port;
}

long ProxySettings::getType() const {
  return m_type;
}

std::string ProxySettings::getTypeStr() const {
  if (m_useProxy == false) {
    return "";
  }

  switch (m_type) {
    case CURLPROXY_SOCKS4:
      return "SOCKS4";
      break;

    case CURLPROXY_SOCKS5:
      return "SOCKS5";
      break;

    case CURLPROXY_HTTP:
      return "HTTP";
      break;

    default:
      return "";
  }
}

std::string ProxySettings::getAuthentificationUser() const {
  return m_authUser;
}

std::string ProxySettings::getAuthentificationPassword() const {
  return m_authPassword;
}

bool ProxySettings::useDefaultServer() const {
  return m_server == "";
}

bool ProxySettings::useDefaultPort() const {
  return m_port == -1;
}

bool ProxySettings::useDefaultAuthentification() const {
  return m_authUser == "";
}

void XMSession::createDefaultConfig(UserConfig *v_config) {
  /* Display */
  /* don't store theses values in the database */
  /* so that in case of pb, people can change them easyly */
  /* moreover, dbinitialisation require win to be initialized (this can be
   * changed) */
  v_config->createVar("DisplayWidth", "800");
  v_config->createVar("DisplayHeight", "600");
  v_config->createVar("DisplayBPP", "32");
  v_config->createVar("DisplayWindowed", "true");
  v_config->createVar("DisplayMaxRenderFPS", "50");
  v_config->createVar("DrawLib", DEFAULT_DRAWLIB);
  v_config->createVar("UseThemeCursor", "true");

  /* option not easy to change (not in the options tab) ; keep them here */
  v_config->createVar("DefaultProfile", DEFAULT_PROFILE);
  v_config->createVar("ScreenshotFormat", DEFAULT_SCREENSHOTFORMAT);
  v_config->createVar("StoreReplays", "true");
  v_config->createVar("ReplayFrameRate", "25");

  /* server url, keep them easy to modify */
  v_config->createVar("WebLevelsURL", DEFAULT_WEBLEVELS_URL);
  v_config->createVar("WebDbSyncUploadURL", DEFAULT_UPLOADDBSYNC_URL);
  v_config->createVar("WebThemesURL", DEFAULT_WEBTHEMES_URL);
  v_config->createVar("WebThemesURLBase", DEFAULT_WEBTHEMES_SPRITESURLBASE);
  v_config->createVar("WebHighscoreUploadURL", DEFAULT_UPLOADREPLAY_URL);
}
