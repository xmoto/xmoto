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
#include "db/xmDatabase.h"
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
  m_www_password                  = "";
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
  m_ghostStrategy_BESTOFREFROOM   = false;
  m_ghostStrategy_BESTOFOTHERROOMS = false;
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
  m_audioSampleRate               = 44100;
  m_audioSampleBits               = 16;
  m_audioChannels                 = 2;
  m_enableAudioEngine             = true;
  m_checkNewLevelsAtStartup       = true;
  m_checkNewHighscoresAtStartup   = true;
  m_showHighscoreInGame           = true;
  for(unsigned int i=0; i<ROOMS_NB_MAX; i++) {
    m_idRoom[i]                   = DEFAULT_WEBROOM_ID;
  }
  m_nbRoomsEnabled = 1;
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
  m_useChildrenCompliant          = false;
	m_forceChildrenCompliant        = false;
  m_enableVideoRecording          = false;
  m_videoRecordingDivision        = VR_DEFAULT_DIVISION;
  m_videoRecordingFramerate       = VR_DEFAULT_FRAMERATE;
  m_videoRecordingStartTime       = -1;
  m_videoRecordingEndTime         = -1;
  m_hidePlayingInformation        = false;
  m_enableInitZoom                = true;
  m_enableActiveZoom              = true;
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

	if(i_xmargs->isOptForceChildrenCompliant()) {
		m_forceChildrenCompliant = true;
	}
}

void XMSession::load(UserConfig* m_Config) {
  m_profile            = m_Config->getString("DefaultProfile");

  m_resolutionWidth    = m_Config->getInteger("DisplayWidth");
  m_resolutionHeight   = m_Config->getInteger("DisplayHeight");
  m_bpp                = m_Config->getInteger("DisplayBPP");
  m_windowed           = m_Config->getBool("DisplayWindowed");
  m_drawlib            = m_Config->getString("DrawLib");

  m_screenshotFormat   = m_Config->getString("ScreenshotFormat");
  m_storeReplays       = m_Config->getBool("StoreReplays");
  m_replayFrameRate    = m_Config->getFloat("ReplayFrameRate");
  m_compressReplays    = m_Config->getBool("CompressReplays");

  m_uploadHighscoreUrl = m_Config->getString("WebHighscoreUploadURL");
  m_webThemesURL       = m_Config->getString("WebThemesURL");
  m_webThemesURLBase   = m_Config->getString("WebThemesURLBase");
  m_webLevelsUrl       = m_Config->getString("WebLevelsURL");

}

void XMSession::loadProfile(const std::string& i_id_profile, xmDatabase* pDb) {
  /* here you can change the default option values for profiles */
  /* note the duplication with setToDefault() */

  m_www          	    	   = pDb->config_getBool   (i_id_profile, "WebHighscores"    	       	  , true                   );
  m_www_password 	    	   = pDb->config_getString (i_id_profile, "WWWPassword"      	       	  , ""                     );
  m_theme        	    	   = pDb->config_getString (i_id_profile, "Theme"            	       	  , THEME_DEFAULT_THEMENAME);
  m_language     	    	   = pDb->config_getString (i_id_profile, "Language"         	       	  , ""                     );
  m_quickStartQualityMIN    	   = pDb->config_getInteger(i_id_profile, "QSQualityMIN"     	       	  , 1                      );
  m_quickStartQualityMAX    	   = pDb->config_getInteger(i_id_profile, "QSQualityMAX"     	       	  , 5                      );
  m_quickStartDifficultyMIN 	   = pDb->config_getInteger(i_id_profile, "QSDifficultyMIN"  	       	  , 1                      );
  m_quickStartDifficultyMAX 	   = pDb->config_getInteger(i_id_profile, "QSDifficultyMAX"  	       	  , 5                      );
  m_enableAudio     	    	   = pDb->config_getBool   (i_id_profile, "AudioEnable"      	       	  , true                   );
  m_audioSampleRate    	    	   = pDb->config_getInteger(i_id_profile, "AudioSampleRate"  	       	  , 44100                  );
  m_audioSampleBits    	    	   = pDb->config_getInteger(i_id_profile, "AudioSampleBits"  	       	  , 16                     );
  m_audioChannels     	    	   = pDb->config_getString (i_id_profile, "AudioChannels"    	       	  , "Stereo") == "Mono" ? 1 : 2;
  m_enableAudioEngine  	    	   = pDb->config_getBool   (i_id_profile, "EngineSoundEnable"	       	  , true                   );
  m_autosaveHighscoreReplays       = pDb->config_getBool   (i_id_profile, "AutosaveHighscoreReplays"      , true                   );
  m_notifyAtInit                   = pDb->config_getBool   (i_id_profile, "NotifyAtInit"                  , true                   );
  m_showMinimap                    = pDb->config_getBool   (i_id_profile, "ShowMiniMap"                   , true                   );
  m_showEngineCounter              = pDb->config_getBool   (i_id_profile, "ShowEngineCounter"             , false                  );
  m_enableContextHelp        	   = pDb->config_getBool   (i_id_profile, "ContextHelp"                   , true                   );
  m_enableMenuMusic                = pDb->config_getBool   (i_id_profile, "MenuMusic"                     , true                   );
  m_enableInitZoom           	   = pDb->config_getBool   (i_id_profile, "InitZoom"                      , true                   );
  m_enableActiveZoom         	   = pDb->config_getBool   (i_id_profile, "CameraActiveZoom"              , true                   );
  m_enableDeadAnimation      	   = pDb->config_getBool   (i_id_profile, "DeathAnim"                     , true                   );
  m_checkNewLevelsAtStartup        = pDb->config_getBool   (i_id_profile, "CheckNewLevelsAtStartup"       , true                   );
  m_checkNewHighscoresAtStartup    = pDb->config_getBool   (i_id_profile, "CheckHighscoresAtStartup"      , true                   );
  m_showHighscoreInGame            = pDb->config_getBool   (i_id_profile, "ShowInGameWorldRecord"         , true                   );
  m_webConfAtInit                  = pDb->config_getBool   (i_id_profile, "WebConfAtInit"                 , true                   );
  m_useCrappyPack                  = pDb->config_getBool   (i_id_profile, "UseCrappyPack"                 , true                   );
  m_useChildrenCompliant           = pDb->config_getBool   (i_id_profile, "UseChildrenCompliant"          , false                  );
  m_enableGhosts                   = pDb->config_getBool   (i_id_profile, "EnableGhost"                   , true                   );
  m_ghostStrategy_MYBEST           = pDb->config_getBool   (i_id_profile, "GhostStrategy_MYBEST"          , true                   );
  m_ghostStrategy_THEBEST          = pDb->config_getBool   (i_id_profile, "GhostStrategy_THEBEST"         , false                  );
  m_ghostStrategy_BESTOFREFROOM    = pDb->config_getBool   (i_id_profile, "GhostStrategy_BESTOFREFROOM"   , false                  );
  m_ghostStrategy_BESTOFOTHERROOMS = pDb->config_getBool   (i_id_profile, "GhostStrategy_BESTOFOTHERROOMS", false                  );
  m_showGhostTimeDifference        = pDb->config_getBool   (i_id_profile, "ShowGhostTimeDiff"             , true                   );
  m_showGhostsInfos                = pDb->config_getBool   (i_id_profile, "DisplayGhostInfo"              , false                  );
  m_ghostMotionBlur                = pDb->config_getBool   (i_id_profile, "GhostMotionBlur"               , true                   );
  m_hideGhosts                     = pDb->config_getBool   (i_id_profile, "HideGhosts"                    , false                  );
  m_multiStopWhenOneFinishes 	   = pDb->config_getBool   (i_id_profile, "MultiStopWhenOneFinishes"      , true                   );

  m_nbRoomsEnabled                 = pDb->config_getInteger(i_id_profile, "WebHighscoresNbRooms"          , 1                      );
  if(m_nbRoomsEnabled < 1) { m_nbRoomsEnabled = 1; }
  if(m_nbRoomsEnabled > ROOMS_NB_MAX) { m_nbRoomsEnabled = ROOMS_NB_MAX; }
  for(unsigned int i=0; i<ROOMS_NB_MAX; i++) {
    if(i == 0) {
      m_idRoom[i] = pDb->config_getString(i_id_profile, "WebHighscoresIdRoom", DEFAULT_WEBROOM_ID);
    } else {
      std::ostringstream v_strRoom;
      v_strRoom << i;
      m_idRoom[i] = pDb->config_getString(i_id_profile, "WebHighscoresIdRoom" + v_strRoom.str(), DEFAULT_WEBROOM_ID);
    }
  }

  m_proxySettings.setPort(            pDb->config_getInteger(i_id_profile, "ProxyPort"    , -1));
  m_proxySettings.setType(            pDb->config_getString (i_id_profile, "ProxyType"    , ""));
  m_proxySettings.setServer(          pDb->config_getString (i_id_profile, "ProxyServer"  , ""));
  m_proxySettings.setAuthentification(pDb->config_getString (i_id_profile, "ProxyAuthUser", ""),
				      pDb->config_getString (i_id_profile, "ProxyAuthPwd" , ""));

  std::string v_menuGraphics = pDb->config_getString(i_id_profile, "MenuGraphics", "High");
  if(v_menuGraphics == "Low")    m_menuGraphics = GFX_LOW;
  if(v_menuGraphics == "Medium") m_menuGraphics = GFX_MEDIUM;
  if(v_menuGraphics == "High")   m_menuGraphics = GFX_HIGH;

  std::string v_gameGraphics = pDb->config_getString(i_id_profile, "GameGraphics", "High");
  if(v_gameGraphics == "Low")    m_gameGraphics = GFX_LOW;
  if(v_gameGraphics == "Medium") m_gameGraphics = GFX_MEDIUM;
  if(v_gameGraphics == "High")   m_gameGraphics = GFX_HIGH;

}

void XMSession::save(UserConfig* v_config, xmDatabase* pDb) {
  v_config->setString("DefaultProfile",         m_profile);

  v_config->setInteger("DisplayWidth",          m_resolutionWidth);
  v_config->setInteger("DisplayHeight",         m_resolutionHeight);
  v_config->setInteger("DisplayBPP",            m_bpp);
  v_config->setBool   ("DisplayWindowed",       m_windowed);

  v_config->setString ("WebThemesURL",          m_webThemesURL);
  v_config->setString ("WebThemesURLBase",      m_webThemesURLBase);
  v_config->setString ("WebHighscoreUploadURL", m_uploadHighscoreUrl);
  v_config->setString ("WebLevelsURL",          m_webLevelsUrl);

  v_config->setFloat  ("ReplayFrameRate",       m_replayFrameRate);
  v_config->setBool   ("StoreReplays",          m_storeReplays);
  v_config->setBool   ("CompressReplays",       m_compressReplays);

  saveProfile(pDb);
}

void XMSession::saveProfile(xmDatabase* pDb) {
  pDb->config_setBool   (m_profile, "WebHighscores"    	      	    , m_www                    );
  pDb->config_setString (m_profile, "WWWPassword"      	      	    , m_www_password           );
  pDb->config_setString (m_profile, "Theme"            	      	    , m_theme                  );
  pDb->config_setString (m_profile, "Language"         	      	    , m_language               );
  pDb->config_setInteger(m_profile, "QSQualityMIN"     	      	    , m_quickStartQualityMIN   );
  pDb->config_setInteger(m_profile, "QSQualityMAX"     	      	    , m_quickStartQualityMAX   );
  pDb->config_setInteger(m_profile, "QSDifficultyMIN"  	      	    , m_quickStartDifficultyMIN);
  pDb->config_setInteger(m_profile, "QSDifficultyMAX"  	      	    , m_quickStartDifficultyMAX);
  pDb->config_setBool   (m_profile, "AudioEnable"      	      	    , m_enableAudio            );
  pDb->config_setInteger(m_profile, "AudioSampleRate"  	      	    , m_audioSampleRate        );
  pDb->config_setInteger(m_profile, "AudioSampleBits"  	      	    , m_audioSampleBits        );
  pDb->config_setString (m_profile, "AudioChannels"    	      	    , m_audioChannels == 1 ? "Mono" : "Stereo");
  pDb->config_setBool   (m_profile, "EngineSoundEnable"	      	    , m_enableAudioEngine      );
  pDb->config_setBool   (m_profile, "AutosaveHighscoreReplays"      , m_autosaveHighscoreReplays);
  pDb->config_setBool   (m_profile, "NotifyAtInit"                  , m_notifyAtInit);
  pDb->config_setBool   (m_profile, "ShowMiniMap"                   , m_showMinimap);
  pDb->config_setBool   (m_profile, "ShowEngineCounter"             , m_showEngineCounter);
  pDb->config_setBool   (m_profile, "ContextHelp"                   , m_enableContextHelp);
  pDb->config_setBool   (m_profile, "MenuMusic"                     , m_enableMenuMusic);
  pDb->config_setBool   (m_profile, "InitZoom"                      , m_enableInitZoom);
  pDb->config_setBool   (m_profile, "CameraActiveZoom"              , m_enableActiveZoom);
  pDb->config_setBool   (m_profile, "DeathAnim"                     , m_enableDeadAnimation);
  pDb->config_setBool   (m_profile, "CheckNewLevelsAtStartup"       , m_checkNewLevelsAtStartup);
  pDb->config_setBool   (m_profile, "CheckHighscoresAtStartup"      , m_checkNewHighscoresAtStartup);
  pDb->config_setBool   (m_profile, "ShowInGameWorldRecord"         , m_showHighscoreInGame);
  pDb->config_setBool   (m_profile, "WebConfAtInit"                 , m_webConfAtInit);
  pDb->config_setBool   (m_profile, "UseCrappyPack"                 , m_useCrappyPack);
  pDb->config_setBool   (m_profile, "UseChildrenCompliant"          , m_useChildrenCompliant);
  pDb->config_setBool   (m_profile, "EnableGhost"           	    , m_enableGhosts);
  pDb->config_setBool   (m_profile, "GhostStrategy_MYBEST"  	    , m_ghostStrategy_MYBEST);
  pDb->config_setBool   (m_profile, "GhostStrategy_THEBEST" 	    , m_ghostStrategy_THEBEST);
  pDb->config_setBool   (m_profile, "GhostStrategy_BESTOFREFROOM"   , m_ghostStrategy_BESTOFREFROOM);
  pDb->config_setBool   (m_profile, "GhostStrategy_BESTOFOTHERROOMS", m_ghostStrategy_BESTOFOTHERROOMS);
  pDb->config_setBool   (m_profile, "ShowGhostTimeDiff"             , m_showGhostTimeDifference);
  pDb->config_setBool   (m_profile, "DisplayGhostInfo"              , m_showGhostsInfos);
  pDb->config_setBool   (m_profile, "HideGhosts"                    , m_hideGhosts);
  pDb->config_setBool   (m_profile, "GhostMotionBlur"               , m_ghostMotionBlur);
  pDb->config_setBool   (m_profile, "MultiStopWhenOneFinishes"      , m_multiStopWhenOneFinishes);
  pDb->config_setString (m_profile, "MenuGraphics", m_menuGraphics == GFX_LOW ? "Low" : m_menuGraphics == GFX_MEDIUM ? "Medium":"High");
  pDb->config_setString (m_profile, "GameGraphics", m_gameGraphics == GFX_LOW ? "Low" : m_gameGraphics == GFX_MEDIUM ? "Medium":"High");
  pDb->config_setString (m_profile, "ProxyType",     proxySettings()->getTypeStr());
  pDb->config_setString (m_profile, "ProxyServer",   proxySettings()->getServer());
  pDb->config_setString (m_profile, "ProxyAuthUser", proxySettings()->getAuthentificationUser());
  pDb->config_setString (m_profile, "ProxyAuthPwd" , proxySettings()->getAuthentificationPassword());
  pDb->config_setInteger(m_profile, "ProxyPort"                      , proxySettings()->getPort());

  pDb->config_setInteger(m_profile, "WebHighscoresNbRooms"           , m_nbRoomsEnabled);
  for(unsigned int i=0; i<ROOMS_NB_MAX; i++) {
    if(i==0) {
      pDb->config_setString(m_profile, "WebHighscoresIdRoom", m_idRoom[i]);
    } else {
      std::ostringstream v_strRoom;
      v_strRoom << i;
      pDb->config_setString(m_profile, "WebHighscoresIdRoom" + v_strRoom.str(), m_idRoom[i]);
    }
  }
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

std::string XMSession::wwwPassword() const {
  return m_www_password;
}

void XMSession::setWwwPassword(const std::string& i_password) {
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

bool XMSession::ghostStrategy_BESTOFREFROOM() const {
  return m_ghostStrategy_BESTOFREFROOM;
}

void XMSession::setGhostStrategy_BESTOFREFROOM(bool i_value) {
  m_ghostStrategy_BESTOFREFROOM = i_value;
}

bool XMSession::ghostStrategy_BESTOFOTHERROOMS() const {
  return m_ghostStrategy_BESTOFOTHERROOMS;
}

void XMSession::setGhostStrategy_BESTOFOTHERROOMS(bool i_value) {
  m_ghostStrategy_BESTOFOTHERROOMS = i_value;
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

void XMSession::setEnableActiveZoom(bool i_value) {
  m_enableActiveZoom = i_value;
}

bool XMSession::enableActiveZoom() const {
  return m_enableActiveZoom;
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

unsigned int XMSession::nbRoomsEnabled() const {
  return m_nbRoomsEnabled;
}

void XMSession::setNbRoomsEnabled(unsigned int i_value) {
  m_nbRoomsEnabled = i_value;
}

void XMSession::setIdRoom(unsigned int i_number, const std::string& i_value) {
  m_idRoom[i_number] = i_value;
}

std::string XMSession::idRoom(unsigned int i_number) const {
  return m_idRoom[i_number];
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

bool XMSession::useChildrenCompliant() const {
	if(m_forceChildrenCompliant) {
		return true;
	}

  return m_useChildrenCompliant;
}

void XMSession::setChildrenCompliant(bool i_value) {
  m_useChildrenCompliant = i_value;
}

bool XMSession::forceChildrenCompliant() const {
	return m_forceChildrenCompliant;
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

  /* Display */
  /* don't store theses values in the database */
  /* so that in case of pb, people can change them easyly */
  /* moreover, dbinitialisation require win to be initialized (this can be changed) */
  v_config->createVar( "DisplayWidth",           "800" );
  v_config->createVar( "DisplayHeight",          "600" );
  v_config->createVar( "DisplayBPP",             "32" );
  v_config->createVar( "DisplayWindowed",        "true" );
  v_config->createVar( "DrawLib",                "OPENGL" );

  /* option not easy to change (not in the options tab) ; keep them here */ 
  v_config->createVar( "DefaultProfile",         "" );
  v_config->createVar( "ScreenshotFormat",       "png" );
  v_config->createVar( "StoreReplays",           "true" );
  v_config->createVar( "ReplayFrameRate",        "25" );
  v_config->createVar( "CompressReplays",        "true" );

  /* server url, keep them easy to modify */
  v_config->createVar( "WebLevelsURL",           DEFAULT_WEBLEVELS_URL);
  v_config->createVar( "WebThemesURL",           DEFAULT_WEBTHEMES_URL);
  v_config->createVar( "WebThemesURLBase",       DEFAULT_WEBTHEMES_SPRITESURLBASE);
  v_config->createVar( "WebHighscoreUploadURL",  DEFAULT_UPLOADREPLAY_URL);

  /* Controls -- no options to change this for the moment, then, keep them here */
  v_config->createVar( "ControllerMode1",        "Keyboard" );
  v_config->createVar( "ControllerMode2",        "Keyboard" );
  v_config->createVar( "ControllerMode3",        "Keyboard" );
  v_config->createVar( "ControllerMode4",        "Keyboard" );

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
}
