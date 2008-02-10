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
#include "XMArgs.h"
#include "UserConfig.h"
#include "WWW.h"
#include <curl/curl.h>
#include "VideoRecorder.h"

XMSession::XMSession() {
  setToDefault();

  /* don't set them by default */
  m_webConfAtInit = true;
  m_profile       = "";
}

void XMSession::setToDefault() {
  m_verbose          	          = false;
  m_resolutionWidth  	          = 800;
  m_resolutionHeight 	          = 600;
  m_bpp              	          = 32;
  m_windowed         	          = true;
  m_glExts           	          = true;
  m_drawlib          	          = "OPENGL";
  m_www              	          = true;
  m_benchmark 	     	          = false;
  m_debug     	     	          = false;
  m_sqlTrace  	     	          = false;
  m_gdebug    	     	          = false;
  m_timedemo  	     	          = false;
  m_fps       	     	          = false;
  m_ugly      	     	          = false;
  m_uglyOver         	          = false;
  m_testTheme 	     	          = false;
  m_ghostStrategy_MYBEST          = true;
  m_ghostStrategy_THEBEST         = false;
  m_ghostStrategy_BESTOFROOM      = false;
  m_autosaveHighscoreReplays      = true;
  m_enableGhosts                  = true;
  m_enableEngineSound             = true;
  m_showEngineCounter             = true;
  m_showMinimap                   = true;
  m_multiStopWhenOneFinishes      = true;
  m_enableMenuMusic               = true;
  m_enableDeadAnimation           = true;
  m_menuGraphics                  = GFX_HIGH;
  m_gameGraphics                  = GFX_HIGH;
  m_quickStartQualityMIN          = 1;
  m_quickStartQualityMAX          = 5;
  m_quickStartDifficultyMIN       = 1;
  m_quickStartDifficultyMAX       = 5;
  m_multiNbPlayers                = 1;
  m_multiScenes                   = false;
  m_multiEnableStopWheNoneFinishes = true;
  m_enableContextHelp             = true;
  m_theme                         = THEME_DEFAULT_THEMENAME;
  m_enableAudio                   = true;
  m_audioSampleRate               = 22050;
  m_audioSampleBits               = 16;
  m_audioChannels                 = 1;
  m_enableAudioEngine             = true;
  m_checkNewLevelsAtStartup       = true;
  m_checkNewHighscoresAtStartup   = true;
  m_showHighscoreInGame           = true;
  m_idRoom                        = DEFAULT_WEBROOM_ID;
  m_showGhostTimeDifference       = true;
  m_ghostMotionBlur               = true;
  m_showGhostsInfos               = false;
  m_hideGhosts                    = false;
  m_replayFrameRate               = 25.0;
  m_webThemesURL                  = DEFAULT_WEBTHEMES_URL;
  m_webThemesURLBase              = DEFAULT_WEBTHEMES_SPRITESURLBASE;
  m_webRoomsURL                   = DEFAULT_WEBROOMS_URL;
  m_storeReplays                  = true;
  m_compressReplays               = true;
  m_enableReplayInterpolation     = true;
  m_uploadHighscoreUrl            = DEFAULT_UPLOADREPLAY_URL;
  m_screenshotFormat              = "png";
  m_notifyAtInit                  = true;
  m_webLevelsUrl                  = DEFAULT_WEBLEVELS_URL;
  m_mirrorMode                    = false;
  m_useCrappyPack                 = true;
  m_enableVideoRecording          = false;
  m_videoRecordingDivision        = VR_DEFAULT_DIVISION;
  m_videoRecordingFramerate       = VR_DEFAULT_FRAMERATE;
  m_videoRecordingStartTime       = -1;
  m_videoRecordingEndTime         = -1;
  m_hidePlayingInformation        = false;
}

void XMSession::load(const XMArguments* i_xmargs) {
  if(i_xmargs->isOptVerbose()) {
    m_verbose = true;
  }
  if(i_xmargs->isOptRes()) {
    m_resolutionWidth  = i_xmargs->getOpt_res_dispWidth();
    m_resolutionHeight = i_xmargs->getOpt_res_dispHeight();
  }

  if(i_xmargs->isOptBpp()) {
    m_bpp = i_xmargs->getOpt_bpp_value();
  }

  if(i_xmargs->isOptWindowed()) {
    m_windowed = true;
  }

  if(i_xmargs->isOptFs()) {
    m_windowed = false;
  }

  if(i_xmargs->isOptNoExts()) {
    m_glExts = false;
  }

  if(i_xmargs->isOptDrawlib()) {
    m_drawlib = i_xmargs->getOpt_drawlib_lib();
  }

  if(i_xmargs->isOptNoWWW()) {
    m_www = false;
  }

  if(i_xmargs->isOptBenchmark()) {
    m_benchmark = true;
  }

  if(i_xmargs->isOptDebug()) {
    m_debug = true;
  }

  if(i_xmargs->isOptSqlTrace()) {
    m_sqlTrace = true;
  }

  if(i_xmargs->isOptProfile()) {
    m_profile = i_xmargs->getOpt_profile_value();
  }

  if(i_xmargs->isOptGDebug()) {
    m_gdebug = true;
    m_gdebug_file = i_xmargs->getOpt_gdebug_file();
  }

  if(i_xmargs->isOptTimedemo()) {
    m_timedemo = true;
  }

  if(i_xmargs->isOptFps()) {
    m_fps = true;
  }

  if(i_xmargs->isOptUgly()) {
    m_ugly = true;
  }

  if(i_xmargs->isOptTestTheme()) {
    m_testTheme = true;
  }

  if(i_xmargs->isOptNoSound()) {
    m_enableAudio = false;
  }

  if(i_xmargs->isOptVideoRecording()) {
    m_enableVideoRecording = true;
    m_videoRecordName = i_xmargs->getOptVideoRecording_name();
  }

  if(i_xmargs->isOptVideoRecordingDivision()) {
    m_videoRecordingDivision = i_xmargs->getOptVideoRecordingDivision_value();
  } else {
    m_videoRecordingDivision = VR_DEFAULT_DIVISION;
  }

  if(i_xmargs->isOptVideoRecordingFramerate()) {
    m_videoRecordingFramerate = i_xmargs->getOptVideoRecordingFramerate_value();
  } else {
    m_videoRecordingFramerate = VR_DEFAULT_FRAMERATE;
  }

  if(i_xmargs->isOptVideoRecordingStartTime()) {
    m_videoRecordingStartTime = i_xmargs->getOptVideoRecordingStartTime_value();
  }

  if(i_xmargs->isOptVideoRecordingEndTime()) {
    m_videoRecordingEndTime = i_xmargs->getOptVideoRecordingEndTime_value();
  }

  if(i_xmargs->isOptHidePlayingInformation()) {
    m_hidePlayingInformation = true;
  }

}

void XMSession::load(UserConfig* m_Config) {
  m_resolutionWidth  	     = m_Config->getInteger("DisplayWidth");
  m_resolutionHeight 	     = m_Config->getInteger("DisplayHeight");
  m_bpp              	     = m_Config->getInteger("DisplayBPP");
  m_windowed         	     = m_Config->getBool("DisplayWindowed");
  m_drawlib          	     = m_Config->getString("DrawLib");
  m_www              	     = m_Config->getBool("WebHighscores");
  m_profile          	     = m_Config->getString("DefaultProfile");
  m_ghostStrategy_MYBEST     = m_Config->getBool("GhostStrategy_MYBEST");
  m_ghostStrategy_THEBEST    = m_Config->getBool("GhostStrategy_THEBEST");
  m_ghostStrategy_BESTOFROOM = m_Config->getBool("GhostStrategy_BESTOFROOM");
  m_autosaveHighscoreReplays = m_Config->getBool("AutosaveHighscoreReplays");
  m_enableGhosts             = m_Config->getBool("EnableGhost");
  m_enableEngineSound        = m_Config->getBool("EngineSoundEnable");
  m_showEngineCounter        = m_Config->getBool("ShowEngineCounter");
  m_showMinimap              = m_Config->getBool("ShowMiniMap");
  m_multiStopWhenOneFinishes = m_Config->getBool("MultiStopWhenOneFinishes");
  m_enableMenuMusic          = m_Config->getBool("MenuMusic");
  m_enableInitZoom           = m_Config->getBool("InitZoom");
  m_enableDeadAnimation      = m_Config->getBool("DeathAnim");

  std::string v_menuGraphics = m_Config->getString("MenuGraphics");
  if(v_menuGraphics == "Low")    m_menuGraphics = GFX_LOW;
  if(v_menuGraphics == "Medium") m_menuGraphics = GFX_MEDIUM;
  if(v_menuGraphics == "High")   m_menuGraphics = GFX_HIGH;

  std::string v_gameGraphics = m_Config->getString("GameGraphics");
  if(v_gameGraphics == "Low")    m_gameGraphics = GFX_LOW;
  if(v_gameGraphics == "Medium") m_gameGraphics = GFX_MEDIUM;
  if(v_gameGraphics == "High")   m_gameGraphics = GFX_HIGH;

  m_quickStartQualityMIN     	= m_Config->getInteger("QSQualityMIN");
  m_quickStartQualityMAX     	= m_Config->getInteger("QSQualityMAX");
  m_quickStartDifficultyMIN  	= m_Config->getInteger("QSDifficultyMIN");
  m_quickStartDifficultyMAX  	= m_Config->getInteger("QSDifficultyMAX");
  m_multiStopWhenOneFinishes 	= m_Config->getBool("MultiStopWhenOneFinishes");
  m_enableContextHelp        	= m_Config->getBool("ContextHelp");
  m_theme                    	= m_Config->getString("Theme");
  m_enableAudio              	= m_Config->getBool("AudioEnable");
  m_audioSampleRate          	= m_Config->getInteger("AudioSampleRate");
  m_audioSampleBits          	= m_Config->getInteger("AudioSampleBits");
  m_audioChannels            	= m_Config->getString("AudioChannels") == "Mono" ? 1 : 2;
  m_enableAudioEngine        	= m_Config->getBool("EngineSoundEnable");
  m_enableMenuMusic          	= m_Config->getBool("MenuMusic");
  m_checkNewLevelsAtStartup     = m_Config->getBool("CheckNewLevelsAtStartup");
  m_checkNewHighscoresAtStartup = m_Config->getBool("CheckHighscoresAtStartup");
  m_showHighscoreInGame         = m_Config->getBool("ShowInGameWorldRecord");
  m_uploadLogin                 = m_Config->getString("WebHighscoreUploadLogin");
  m_uploadPassword              = m_Config->getString("WebHighscoreUploadPassword");
  m_uploadHighscoreUrl          = m_Config->getString("WebHighscoreUploadURL");
  m_idRoom                      = m_Config->getString("WebHighscoresIdRoom");
  m_showGhostTimeDifference     = m_Config->getBool("ShowGhostTimeDiff");
  m_ghostMotionBlur             = m_Config->getBool("GhostMotionBlur");
  m_showGhostsInfos             = m_Config->getBool("DisplayGhostInfo");
  m_hideGhosts                  = m_Config->getBool("HideGhosts");
  m_replayFrameRate          	= m_Config->getFloat("ReplayFrameRate");
  m_webThemesURL                = m_Config->getString("WebThemesURL");
  m_webThemesURLBase            = m_Config->getString("WebThemesURLBase");
  m_proxySettings.setType(            m_Config->getString("ProxyType"));
  m_proxySettings.setServer(          m_Config->getString("ProxyServer"));
  m_proxySettings.setPort(            m_Config->getInteger("ProxyPort"));
  m_proxySettings.setAuthentification(m_Config->getString("ProxyAuthUser"),
				      m_Config->getString("ProxyAuthPwd"));
  m_webConfAtInit               = m_Config->getBool("WebConfAtInit");
  m_storeReplays                = m_Config->getBool("StoreReplays");
  m_compressReplays             = m_Config->getBool("CompressReplays");
  m_screenshotFormat            = m_Config->getString("ScreenshotFormat");
  m_language                    = m_Config->getString("Language");
  m_notifyAtInit                = m_Config->getBool("NotifyAtInit");
  m_webLevelsUrl                = m_Config->getString("WebLevelsURL");
  m_useCrappyPack               = m_Config->getBool("UseCrappyPack");
}

void XMSession::save(UserConfig* v_config) {
  v_config->setString("DefaultProfile",             m_profile);
  v_config->setInteger("QSQualityMIN",              m_quickStartQualityMIN);
  v_config->setInteger("QSQualityMAX",              m_quickStartQualityMAX);
  v_config->setInteger("QSDifficultyMIN",           m_quickStartDifficultyMIN);
  v_config->setInteger("QSDifficultyMAX",           m_quickStartDifficultyMAX);
  v_config->setBool("MultiStopWhenOneFinishes",     m_multiStopWhenOneFinishes);
  v_config->setBool("ContextHelp",                  m_enableContextHelp);
  v_config->setString("Theme",                      m_theme);
  v_config->setBool("WebHighscores",                m_www);
  v_config->setBool("CheckNewLevelsAtStartup",      m_checkNewLevelsAtStartup);
  v_config->setBool("CheckHighscoresAtStartup",     m_checkNewHighscoresAtStartup);
  v_config->setBool("ShowInGameWorldRecord",        m_showHighscoreInGame);
  v_config->setString("WebHighscoreUploadLogin",    m_uploadLogin);
  v_config->setString("WebHighscoreUploadPassword", m_uploadPassword);
  v_config->setString("WebHighscoresIdRoom",        m_idRoom);
  v_config->setBool("EnableGhost",           	    m_enableGhosts);
  v_config->setBool("GhostStrategy_MYBEST",  	    m_ghostStrategy_MYBEST);
  v_config->setBool("GhostStrategy_THEBEST", 	    m_ghostStrategy_THEBEST);
  v_config->setBool("GhostStrategy_BESTOFROOM",     m_ghostStrategy_BESTOFROOM);
  v_config->setBool("ShowGhostTimeDiff",            m_showGhostTimeDifference);
  v_config->setBool("GhostMotionBlur",              m_ghostMotionBlur);
  v_config->setBool("DisplayGhostInfo",             m_showGhostsInfos);
  v_config->setBool("HideGhosts",                   m_hideGhosts);
  v_config->setBool("ShowMiniMap",                  m_showMinimap);
  v_config->setBool("ShowEngineCounter",            m_showEngineCounter);
  v_config->setBool("InitZoom",                     m_enableInitZoom);
  v_config->setBool("DeathAnim",                    m_enableDeadAnimation);
  v_config->setBool("AutosaveHighscoreReplays",     m_autosaveHighscoreReplays);
  v_config->setInteger("DisplayWidth",              m_resolutionWidth);
  v_config->setInteger("DisplayHeight",             m_resolutionHeight);
  v_config->setInteger("DisplayBPP",                m_bpp);
  v_config->setBool("DisplayWindowed",              m_windowed);
  v_config->setString("MenuGraphics", m_menuGraphics == GFX_LOW ? "Low" : m_menuGraphics == GFX_MEDIUM ? "Medium" : "High");
  v_config->setString("GameGraphics", m_gameGraphics == GFX_LOW ? "Low" : m_gameGraphics == GFX_MEDIUM ? "Medium" : "High");
  v_config->setBool("AudioEnable",                  m_enableAudio);
  v_config->setInteger("AudioSampleRate",           m_audioSampleRate);
  v_config->setInteger("AudioSampleBits",           m_audioSampleBits);
  v_config->setString("AudioChannels",              m_audioChannels == 1 ? "Mono" : "Stereo");
  v_config->setBool("EngineSoundEnable",            m_enableAudioEngine);
  v_config->setBool("MenuMusic",                    m_enableMenuMusic);
  v_config->setFloat("ReplayFrameRate",             m_replayFrameRate);
  v_config->setString("WebThemesURL",               m_webThemesURL);
  v_config->setString("WebThemesURLBase",           m_webThemesURLBase);
  v_config->setString("ProxyType",     		    proxySettings()->getTypeStr());
  v_config->setString("ProxyServer",   		    proxySettings()->getServer());
  v_config->setInteger("ProxyPort",    		    proxySettings()->getPort());
  v_config->setString("ProxyAuthUser", 		    proxySettings()->getAuthentificationUser());
  v_config->setString("ProxyAuthPwd" , 		    proxySettings()->getAuthentificationPassword());
  v_config->setBool("WebConfAtInit",                m_webConfAtInit);
  v_config->setBool("StoreReplays",                 m_storeReplays);
  v_config->setBool("CompressReplays",              m_compressReplays);
  v_config->setString("WebHighscoreUploadURL",      m_uploadHighscoreUrl);
  v_config->setString("Language",                   m_language);
  v_config->setBool("NotifyAtInit",                 m_notifyAtInit);
  v_config->setString("WebLevelsURL",               m_webLevelsUrl);
  v_config->setBool("UseCrappyPack",                m_useCrappyPack);
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

int XMSession::bpp() const {
  return m_bpp;
}

bool XMSession::windowed() const {
  return m_windowed;
}

void XMSession::setResolutionWidth(int i_value) {
  m_resolutionWidth = i_value;
}

void XMSession::setResolutionHeight(int i_value) {
  m_resolutionHeight = i_value;
}

void XMSession::setBpp(int i_value) {
  m_bpp = i_value;
}

void XMSession::setWindowed(bool i_value) {
  m_windowed = i_value;
}

bool XMSession::glExts() const {
  return m_glExts;
}

std::string XMSession::drawlib() const {
  return m_drawlib;
}

bool XMSession::www() const {
  return m_www;
}

void XMSession::setWWW(bool i_value) {
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

void XMSession::setProfile(const std::string& i_profile) {
  m_profile = i_profile;
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
  m_fps = i_value;
}

bool XMSession::ugly() const {
  return m_ugly;
}

bool XMSession::uglyOver() const {
  return m_uglyOver;
}

bool XMSession::testTheme() const {
  return m_testTheme;
}

void XMSession::setUgly(bool i_value) {
  m_ugly = i_value;
}

void XMSession::setUglyOver(bool i_value) {
  m_uglyOver = i_value;
}

void XMSession::setTestTheme(bool i_value) {
  m_testTheme = i_value;
}

bool XMSession::ghostStrategy_MYBEST() const {
  return m_ghostStrategy_MYBEST;
}

void XMSession::setGhostStrategy_MYBEST(bool i_value) {
  m_ghostStrategy_MYBEST = i_value;
}

bool XMSession::ghostStrategy_THEBEST() const {
  return m_ghostStrategy_THEBEST;
}

void XMSession::setGhostStrategy_THEBEST(bool i_value) {
  m_ghostStrategy_THEBEST = i_value;
}

bool XMSession::ghostStrategy_BESTOFROOM() const {
  return m_ghostStrategy_BESTOFROOM;
}

void XMSession::setGhostStrategy_BESTOFROOM(bool i_value) {
  m_ghostStrategy_BESTOFROOM = i_value;
}

bool XMSession::autosaveHighscoreReplays() const {
  return m_autosaveHighscoreReplays;
}

void XMSession::setAutosaveHighscoreReplays(bool i_value) {
  m_autosaveHighscoreReplays = i_value;
}

void XMSession::setEnableGhosts(bool i_value) {
  m_enableGhosts = i_value;
}
 
bool XMSession::enableGhosts() const {
  return m_enableGhosts;
}

void XMSession::setEnableEngineSound(bool i_value) {
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
  m_showEngineCounter = i_value;
}

bool XMSession::showEngineCounter() const {
  return m_showEngineCounter;
}

void XMSession::setShowMinimap(bool i_value) {
  m_showMinimap = i_value;
}

bool XMSession::showMinimap() const {
  return m_showMinimap;
}

void XMSession::setMultiStopWhenOneFinishes(bool i_value) {
  m_multiStopWhenOneFinishes = i_value;
}

bool XMSession::MultiStopWhenOneFinishes() const {
  return m_multiStopWhenOneFinishes;
}

void XMSession::setEnableMenuMusic(bool i_value) {
  m_enableMenuMusic = i_value;
}

bool XMSession::enableMenuMusic() const {
  return m_enableMenuMusic;
}

void XMSession::setEnableInitZoom(bool i_value) {
  m_enableInitZoom = i_value;
}

bool XMSession::enableInitZoom() const {
  return m_enableInitZoom;
}

void XMSession::setEnableDeadAnimation(bool i_value) {
  m_enableDeadAnimation = i_value;
}

bool XMSession::enableDeadAnimation() const {
  return m_enableDeadAnimation;
}

void XMSession::setMenuGraphics(GraphicsLevel i_value) {
  m_menuGraphics = i_value;
}

GraphicsLevel XMSession::menuGraphics() const {
  return m_menuGraphics;
}

void XMSession::setGameGraphics(GraphicsLevel i_value) {
  m_gameGraphics = i_value;
}

GraphicsLevel XMSession::gameGraphics() const {
  return m_gameGraphics;
}


void XMSession::setQuickStartQualityMIN(int i_value) {
  m_quickStartQualityMIN = i_value;
}

int XMSession::quickStartQualityMIN() const {
  return m_quickStartQualityMIN;
}

void XMSession::setQuickStartQualityMAX(int i_value) {
  m_quickStartQualityMAX = i_value;
}

int XMSession::quickStartQualityMAX() const {
  return m_quickStartQualityMAX;
}

void XMSession::setQuickStartDifficultyMIN(int i_value) {
  m_quickStartDifficultyMIN = i_value;
}

int XMSession::quickStartDifficultyMIN() const {
  return m_quickStartDifficultyMIN;
}

void XMSession::setQuickStartDifficultyMAX(int i_value) {
  m_quickStartDifficultyMAX = i_value;
}

int XMSession::quickStartDifficultyMAX() const {
  return m_quickStartDifficultyMAX;
}

void XMSession::setMultiNbPlayers(int i_value) {
  m_multiNbPlayers = i_value;
}

int XMSession::multiNbPlayers() const {
  return m_multiNbPlayers;
}

void XMSession::setMultiScenes(bool i_value) {
  m_multiScenes = i_value;
}

bool XMSession::multiScenes() const {
  return m_multiScenes;
}

void XMSession::setMultiEnableStopWheNoneFinishes(bool i_value) {
  m_multiStopWhenOneFinishes = i_value;
}

bool XMSession::multiEnableStopWheNoneFinishes() const {
  return m_multiStopWhenOneFinishes;
}

void XMSession::setEnableContextHelp(bool i_value) {
  m_enableContextHelp = i_value;
}

bool XMSession::enableContextHelp() const {
  return m_enableContextHelp;
}

void XMSession::setTheme(const std::string& i_value) {
  m_theme = i_value;
}

std::string XMSession::theme() const {
  return m_theme;
}

void XMSession::setEnableAudio(bool i_value) {
  m_enableAudio = i_value;
}

bool XMSession::enableAudio() const {
  return m_enableAudio;
}

void XMSession::setAudioSampleRate(int i_value) {
  m_audioSampleRate = i_value;
}

int XMSession::audioSampleRate() const {
  return m_audioSampleRate;
}

void XMSession::setAudioSampleBits(int i_value) {
  m_audioSampleBits = i_value;
}

int XMSession::audioSampleBits() const {
  return m_audioSampleBits;
}

void XMSession::setAudioChannels(int i_value) {
  m_audioChannels = i_value;
}

int XMSession::audioChannels() const {
  return m_audioChannels;
}

void XMSession::setEnableAudioEngine(bool i_value) {
  m_enableAudioEngine = i_value;
}

bool XMSession::enableAudioEngine() const {
  return m_enableAudioEngine;
}

void XMSession::setCheckNewLevelsAtStartup(bool i_value) {
  m_checkNewLevelsAtStartup = i_value;
}

bool XMSession::checkNewLevelsAtStartup() const {
  return m_checkNewLevelsAtStartup;
}

void XMSession::setCheckNewHighscoresAtStartup(bool i_value) {
  m_checkNewHighscoresAtStartup = i_value;
}

bool XMSession::checkNewHighscoresAtStartup() const {
  return m_checkNewHighscoresAtStartup;
}

void XMSession::setShowHighscoreInGame(bool i_value) {
  m_showHighscoreInGame = i_value;
}

bool XMSession::showHighscoreInGame() const {
  return m_showHighscoreInGame;
}

void XMSession::setUploadLogin(const std::string& i_value) {
  m_uploadLogin = i_value;
}

std::string XMSession::uploadLogin() const {
  return m_uploadLogin;
}

void XMSession::setUploadPassword(const std::string& i_value) {
  m_uploadPassword = i_value;
}

std::string XMSession::uploadPassword() const {
  return m_uploadPassword;
}

void XMSession::setIdRoom(const std::string& i_value) {
  m_idRoom = i_value;
}

std::string XMSession::idRoom() const {
  return m_idRoom;
}

void XMSession::setShowGhostTimeDifference(bool i_value) {
  m_showGhostTimeDifference = i_value;
}

bool XMSession::showGhostTimeDifference() const {
  return m_showGhostTimeDifference;
}

void XMSession::setGhostMotionBlur(bool i_value) {
  m_ghostMotionBlur = i_value;
}

bool XMSession::ghostMotionBlur() const {
  return m_ghostMotionBlur;
}

void XMSession::setShowGhostsInfos(bool i_value) {
  m_showGhostsInfos = i_value;
}

bool XMSession::showGhostsInfos() const {
  return m_showGhostsInfos;
}

void XMSession::setHideGhosts(bool i_value) {
  m_hideGhosts = i_value;
}

bool XMSession::hideGhosts() const {
  return m_hideGhosts;
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
  m_webConfAtInit = i_value;
}

bool XMSession::webConfAtInit() const {
  return m_webConfAtInit;
}

bool XMSession::storeReplays() const {
  return m_storeReplays;
}

bool XMSession::compressReplays() const {
  return m_compressReplays;
}

bool XMSession::enableReplayInterpolation() const {
  return m_enableReplayInterpolation;
}

void XMSession::setEnableReplayInterpolation(bool i_value) {
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

void XMSession::setLanguage(const std::string& i_value) {
  m_language = i_value;
}

void XMSession::setNotifyAtInit(bool i_value) {
  m_notifyAtInit = i_value;
}

bool XMSession::notifyAtInit() const {
  return m_notifyAtInit;
}

std::string XMSession::webLevelsUrl() const {
  return m_webLevelsUrl;
}

bool XMSession::mirrorMode() const {
  return m_mirrorMode;
}

void XMSession::setMirrorMode(bool i_value) {
  m_mirrorMode = i_value;
}

bool XMSession::useCrappyPack() const {
  return m_useCrappyPack;
}

void XMSession::setUseCrappyPack(bool i_value) {
  m_useCrappyPack = i_value;
}

ProxySettings* XMSession::proxySettings() {
  return &m_proxySettings;
}

ProxySettings::ProxySettings() {
  m_useProxy     = false;
  m_server       = "";
  m_port         = -1;
  m_type         = CURLPROXY_HTTP;
  m_authUser     = "";
  m_authPassword = "";
}

void ProxySettings::setServer(std::string p_server) {
  m_server = p_server;
}

void ProxySettings::setPort(long p_port) {
  m_port = p_port;
}

void ProxySettings::setType(const std::string& p_type) {
  m_useProxy = true;

  if(p_type == "SOCKS4") {
    m_type = CURLPROXY_SOCKS4;
  } else if(p_type == "SOCKS5") {
    m_type = CURLPROXY_SOCKS5;
  } else if(p_type == "HTTP") {
    m_type = CURLPROXY_HTTP;
  } else {
    m_useProxy = false;
  }
}

void ProxySettings::setAuthentification(std::string p_user, std::string p_password) {
  m_authUser     = p_user;
  m_authPassword = p_password;
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
  m_authUser     = "";
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
  if(m_useProxy == false) {
    return "";
  }

  switch(m_type) {

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

void XMSession::createDefaultConfig(UserConfig* v_config) {
  v_config->createVar( "Language" , "");
  v_config->createVar( "Theme",                  THEME_DEFAULT_THEMENAME);    

  /* Display */
  v_config->createVar( "DisplayWidth",           "800" );
  v_config->createVar( "DisplayHeight",          "600" );
  v_config->createVar( "DisplayBPP",             "32" );
  v_config->createVar( "DisplayWindowed",        "true" );
  v_config->createVar( "MenuGraphics",           "High" );
  v_config->createVar( "GameGraphics",           "High" );
  v_config->createVar( "DrawLib",                "OPENGL" );
        
  /* Audio */
  v_config->createVar( "AudioEnable",            "true" );
  v_config->createVar( "AudioSampleRate",        "22050" );
  v_config->createVar( "AudioSampleBits",        "16" );
  v_config->createVar( "AudioChannels",          "Mono" );
  v_config->createVar( "EngineSoundEnable",      "true" );

  /* Controls */
  v_config->createVar( "ControllerMode1",        "Keyboard" );
  v_config->createVar( "KeyDrive1",              "Up" );
  v_config->createVar( "KeyBrake1",              "Down" );
  v_config->createVar( "KeyFlipLeft1",           "Left" );
  v_config->createVar( "KeyFlipRight1",          "Right" );
  v_config->createVar( "KeyChangeDir1",          "Space" );
  v_config->createVar( "ControllerMode2",        "Keyboard" );
  v_config->createVar( "KeyDrive2",              "A" );
  v_config->createVar( "KeyBrake2",              "Q" );
  v_config->createVar( "KeyFlipLeft2",           "Z" );
  v_config->createVar( "KeyFlipRight2",          "E" );
  v_config->createVar( "KeyChangeDir2",          "W" );
  v_config->createVar( "ControllerMode3",        "Keyboard" );
  v_config->createVar( "KeyDrive3",              "R" );
  v_config->createVar( "KeyBrake3",              "F" );
  v_config->createVar( "KeyFlipLeft3",           "T" );
  v_config->createVar( "KeyFlipRight3",          "Y" );
  v_config->createVar( "KeyChangeDir3",          "V" );
  v_config->createVar( "ControllerMode4",        "Keyboard" );
  v_config->createVar( "KeyDrive4",              "Y" );
  v_config->createVar( "KeyBrake4",              "H" );
  v_config->createVar( "KeyFlipLeft4",           "U" );
  v_config->createVar( "KeyFlipRight4",          "I" );
  v_config->createVar( "KeyChangeDir4",          "N" );
    
  v_config->createVar( "AutosaveHighscoreReplays", "true");

  v_config->createVar( "KeyZoomIn",              "Pad 7" );
  v_config->createVar( "KeyZoomOut",             "Pad 9" );
  v_config->createVar( "KeyZoomInit",            "Home" );
  v_config->createVar( "KeyCameraMoveXUp",       "Pad 6" );
  v_config->createVar( "KeyCameraMoveXDown",     "Pad 4" );
  v_config->createVar( "KeyCameraMoveYUp",       "Pad 8" );
  v_config->createVar( "KeyCameraMoveYDown",     "Pad 2" );
     
  /* joystick */
  v_config->createVar( "JoyIdx1",                "0" );
  v_config->createVar( "JoyAxisPrim1",           "1" );
  v_config->createVar( "JoyAxisPrimMax1",        "32760" );
  v_config->createVar( "JoyAxisPrimMin1",        "-32760" );
  v_config->createVar( "JoyAxisPrimUL1",         "1024" );
  v_config->createVar( "JoyAxisPrimLL1",         "-1024" );
  v_config->createVar( "JoyAxisSec1",            "0" );
  v_config->createVar( "JoyAxisSecMax1",         "32760" );
  v_config->createVar( "JoyAxisSecMin1",         "-32760" );
  v_config->createVar( "JoyAxisSecUL1",          "1024" );
  v_config->createVar( "JoyAxisSecLL1",          "-1024" );
  v_config->createVar( "JoyButtonChangeDir1",    "0" );

  /* Misc */
  v_config->createVar( "DefaultProfile",         "" );
  v_config->createVar( "ScreenshotFormat",       "png" );
  v_config->createVar( "NotifyAtInit",           "true" );
  v_config->createVar( "ShowMiniMap",            "true" );
  v_config->createVar( "ShowEngineCounter",      "false" );

  v_config->createVar( "StoreReplays",           "true" );
  v_config->createVar( "ReplayFrameRate",        "25" );
  v_config->createVar( "CompressReplays",        "true" );
  v_config->createVar( "ContextHelp",            "true" );
  v_config->createVar( "MenuMusic",              "true" );    
  v_config->createVar( "InitZoom",               "true" );
  v_config->createVar( "DeathAnim",              "true" );

  v_config->createVar( "WebHighscores",            "false" );
  v_config->createVar( "CheckHighscoresAtStartup", "true" );
  v_config->createVar( "CheckNewLevelsAtStartup",  "true" );
  v_config->createVar( "ShowInGameWorldRecord",    "false" );
  v_config->createVar( "WebConfAtInit",            "true" );
  v_config->createVar( "UseCrappyPack",            "true" );
    
  /* Webstuff */
  v_config->createVar( "WebHighscoresURL",       DEFAULT_WEBHIGHSCORES_URL );
  v_config->createVar( "WebLevelsURL",           DEFAULT_WEBLEVELS_URL);
  v_config->createVar( "WebThemesURL",           DEFAULT_WEBTHEMES_URL);
  v_config->createVar( "WebThemesURLBase",       DEFAULT_WEBTHEMES_SPRITESURLBASE);
  v_config->createVar( "WebHighscoresIdRoom",     DEFAULT_WEBROOM_ID);

  /* Proxy */
  v_config->createVar( "ProxyType",              "" ); /* (blank), HTTP, SOCKS4, or SOCKS5 */
  v_config->createVar( "ProxyServer",            "" ); /* (may include user/pass and port) */
  v_config->createVar( "ProxyPort",              "-1" );
  v_config->createVar( "ProxyAuthUser",          "" ); 
  v_config->createVar( "ProxyAuthPwd",           "" );

  /* auto upload */
  v_config->createVar( "WebHighscoreUploadURL", DEFAULT_UPLOADREPLAY_URL);
  v_config->createVar( "WebHighscoreUploadLogin"    , "");
  v_config->createVar( "WebHighscoreUploadPassword" , ""); 

  v_config->createVar( "EnableGhost"        , "true");
  v_config->createVar( "GhostStrategy_MYBEST", "true");
  v_config->createVar( "GhostStrategy_THEBEST", "false");
  v_config->createVar( "GhostStrategy_BESTOFROOM", "false");
  v_config->createVar( "ShowGhostTimeDiff"  , "true");
  v_config->createVar( "DisplayGhostInfo"   , "false");
  v_config->createVar( "HideGhosts"   , "false");
  v_config->createVar( "GhostMotionBlur"    , "true" );

  /* quick start button */
  v_config->createVar("QSQualityMIN",    "1");
  v_config->createVar("QSDifficultyMIN", "1");
  v_config->createVar("QSQualityMAX",    "5");
  v_config->createVar("QSDifficultyMAX", "5");

  /* multi */
  v_config->createVar("MultiStopWhenOneFinishes" , "true");
}
