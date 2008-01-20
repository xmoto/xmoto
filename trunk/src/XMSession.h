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

#ifndef __XMCONF_H__
#define __XMCONF_H__

#include <string>
#include "helpers/Singleton.h"

#define THEME_DEFAULT_THEMENAME "Classic"
#define DEFAULT_WEBROOM_ID      "1"
#define DATABASE_FILE FS::getUserDirUTF8() + "/" + "xm.db"

/*
  XMSession   : current session options
  UserConfig  : config file options
  XMArguments : command line options

  first, XMSession constructor init session values,
  then, UserConfig are loaded, and finally, XMArguments overwrite session's values if they are defined
*/

class XMArguments;
class UserConfig;

enum GraphicsLevel {
  GFX_LOW,
  GFX_MEDIUM,
  GFX_HIGH
};

class ProxySettings {
 public:
  ProxySettings();
  void setServer(std::string p_server);
  void setPort(long p_port);
  void setType(const std::string& p_type); /* CURLPROXY_HTTP OR CURLPROXY_SOCKS5 */
  void setAuthentification(std::string p_user, std::string p_password);

  void setDefaultServer();
  void setDefaultPort();
  void setDefaultType();
  void setDefaultAuthentification();

  std::string getServer() const;
  long getPort() const;
  long getType() const;
  std::string getTypeStr() const;
  std::string getAuthentificationUser() const;
  std::string getAuthentificationPassword() const;

  /* default means : curl try to find default values (no proxy, or environment vars) */
  bool useDefaultServer() const;
  bool useDefaultPort() const;
  bool useDefaultAuthentification() const;

 private:
  bool m_useProxy;
  std::string m_server;
  long m_port;
  long m_type;
  std::string m_authUser;
  std::string m_authPassword;
};

class XMSession : public Singleton<XMSession> {
  friend class Singleton<XMSession>;

private:
  XMSession();
  virtual ~XMSession() {};

public:
  void load(const XMArguments* i_xmargs);
  void load(UserConfig* m_Config);
  void save(UserConfig* m_Config);
  void setToDefault();
  bool isVerbose() const;
  static void createDefaultConfig(UserConfig* v_config);

  void setUseGraphics(bool i_value);
  bool useGraphics() const;
  int resolutionWidth() const;
  int resolutionHeight() const;
  int bpp() const;
  bool windowed() const;
  void setResolutionWidth(int i_value);
  void setResolutionHeight(int i_value);
  void setBpp(int i_value);
  void setWindowed(bool i_value);
  bool glExts() const;
  std::string drawlib() const;
  bool www() const;
  void setWWW(bool i_value);
  bool benchmark() const;
  bool debug() const;
  bool sqlTrace() const;
  std::string profile() const;
  void setProfile(const std::string& i_profile);
  bool gDebug() const;
  std::string gDebugFile() const;
  bool timedemo() const;
  bool fps() const;
  void setFps(bool i_value);
  bool ugly() const;
  void setUgly(bool i_value);
  bool uglyOver() const;
  void setUglyOver(bool i_value);
  bool testTheme() const;
  void setTestTheme(bool i_value);
  bool ghostStrategy_MYBEST() const;
  void setGhostStrategy_MYBEST(bool i_value);
  bool ghostStrategy_THEBEST() const;
  void setGhostStrategy_THEBEST(bool i_value);
  bool ghostStrategy_BESTOFROOM() const;
  void setGhostStrategy_BESTOFROOM(bool i_value);
  bool autosaveHighscoreReplays() const;
  void setAutosaveHighscoreReplays(bool i_value);
  void setEnableGhosts(bool i_value);
  bool enableGhosts() const;
  void setEnableEngineSound(bool i_value);
  bool enableEngineSound() const;
  void setShowEngineCounter(bool i_value);
  bool showEngineCounter() const;
  void setShowMinimap(bool i_value);
  bool showMinimap() const;
  void setMultiStopWhenOneFinishes(bool i_value);
  bool MultiStopWhenOneFinishes() const;
  void setEnableMenuMusic(bool i_value);
  bool enableMenuMusic() const;
  void setEnableInitZoom(bool i_value);
  bool enableInitZoom() const;
  void setEnableDeadAnimation(bool i_value);
  bool enableDeadAnimation() const;
  void setMenuGraphics(GraphicsLevel i_value);
  GraphicsLevel menuGraphics() const;
  void setGameGraphics(GraphicsLevel i_value);
  GraphicsLevel gameGraphics() const;
  void setQuickStartQualityMIN(int i_value);
  int quickStartQualityMIN() const;
  void setQuickStartQualityMAX(int i_value);
  int quickStartQualityMAX() const;
  void setQuickStartDifficultyMIN(int i_value);
  int quickStartDifficultyMIN() const;
  void setQuickStartDifficultyMAX(int i_value);
  int quickStartDifficultyMAX() const;
  void setMultiNbPlayers(int i_value);
  int multiNbPlayers() const;
  void setMultiScenes(bool i_value);
  bool multiScenes() const;
  void setMultiEnableStopWheNoneFinishes(bool i_value);
  bool multiEnableStopWheNoneFinishes() const;
  void setEnableContextHelp(bool i_value);
  bool enableContextHelp() const;
  void setTheme(const std::string& i_value);
  std::string theme() const;
  void setEnableAudio(bool i_value);
  bool enableAudio() const;
  void setAudioSampleRate(int i_value);
  int audioSampleRate() const;
  void setAudioSampleBits(int i_value);
  int audioSampleBits() const;
  void setAudioChannels(int i_value);
  int audioChannels() const;
  void setEnableAudioEngine(bool i_value);
  bool enableAudioEngine() const;
  void setCheckNewLevelsAtStartup(bool i_value);
  bool checkNewLevelsAtStartup() const;
  void setCheckNewHighscoresAtStartup(bool i_value);
  bool checkNewHighscoresAtStartup() const;
  void setShowHighscoreInGame(bool i_value);
  bool showHighscoreInGame() const;
  void setUploadLogin(const std::string& i_value);
  std::string uploadLogin() const;
  void setUploadPassword(const std::string& i_value);
  std::string uploadPassword() const;
  void setIdRoom(const std::string& i_value);
  std::string idRoom() const;
  void setShowGhostTimeDifference(bool i_value);
  bool showGhostTimeDifference() const;
  void setGhostMotionBlur(bool i_value);
  bool ghostMotionBlur() const;
  void setShowGhostsInfos(bool i_value);
  bool showGhostsInfos() const;
  void setHideGhosts(bool i_value);
  bool hideGhosts() const;
  float replayFrameRate() const;
  std::string webThemesURL() const;
  std::string webThemesURLBase() const;
  std::string webRoomsURL() const;
  ProxySettings* proxySettings();
  void setWebConfAtInit(bool i_value);
  bool webConfAtInit() const;
  bool storeReplays() const;
  bool compressReplays() const;
  bool enableReplayInterpolation() const;
  void setEnableReplayInterpolation(bool i_value);
  std::string uploadHighscoreUrl() const;
  std::string screenshotFormat() const;
  std::string language() const;
  void setNotifyAtInit(bool i_value);
  bool notifyAtInit() const;
  std::string webLevelsUrl() const;
  bool mirrorMode() const;
  void setMirrorMode(bool i_value);
  bool useCrappyPack() const;
  void setUseCrappyPack(bool i_value);
  bool enableVideoRecording() const;
  std::string videoRecordName() const;
  int videoRecordingDivision() const;
  int videoRecordingFramerate() const;

  private:
  bool m_verbose;
  int  m_resolutionWidth;
  int  m_resolutionHeight;
  int  m_bpp;
  bool m_windowed;
  bool m_glExts;
  std::string m_drawlib;
  bool m_www;
  bool m_benchmark;
  bool m_debug;
  bool m_sqlTrace;
  std::string m_profile;
  bool m_gdebug;
  std::string m_gdebug_file;
  bool m_timedemo;
  bool m_fps;
  bool m_ugly;
  bool m_uglyOver;
  bool m_testTheme;
  bool m_autosaveHighscoreReplays;
  bool m_ghostStrategy_MYBEST;
  bool m_ghostStrategy_THEBEST;
  bool m_ghostStrategy_BESTOFROOM;
  bool m_enableGhosts;
  bool m_enableEngineSound;
  bool m_showEngineCounter;
  bool m_showMinimap;
  bool m_multiStopWhenOneFinishes; /* in multiplayer, stop the game when one finishes the level */
  bool m_enableMenuMusic;
  bool m_enableInitZoom; /* true: Perform initial level scroll/zoom */
  bool m_enableDeadAnimation;
  GraphicsLevel m_menuGraphics;
  GraphicsLevel m_gameGraphics;
  int m_quickStartQualityMIN;
  int m_quickStartQualityMAX;
  int m_quickStartDifficultyMIN;
  int m_quickStartDifficultyMAX;
  int m_multiNbPlayers;
  bool m_multiScenes;
  bool m_multiEnableStopWheNoneFinishes;
  bool m_enableContextHelp;
  std::string m_theme;
  bool m_enableAudio;
  int m_audioSampleRate;
  int m_audioSampleBits;
  int m_audioChannels;
  bool m_enableAudioEngine;
  bool m_checkNewLevelsAtStartup;
  bool m_checkNewHighscoresAtStartup;
  bool m_showHighscoreInGame;
  std::string m_uploadLogin;
  std::string m_uploadPassword;
  std::string m_uploadHighscoreUrl;
  std::string m_idRoom;
  bool m_showGhostTimeDifference;
  bool m_ghostMotionBlur;
  bool m_showGhostsInfos;
  bool m_hideGhosts;
  float m_replayFrameRate;
  std::string m_webThemesURL;
  std::string m_webThemesURLBase;
  std::string m_webRoomsURL;
  ProxySettings m_proxySettings;
  bool m_webConfAtInit;
  bool m_storeReplays;
  bool m_compressReplays;
  bool m_enableReplayInterpolation;
  std::string m_screenshotFormat;
  std::string m_language;
  bool m_notifyAtInit;
  std::string m_webLevelsUrl;
  bool m_mirrorMode;
  bool m_useCrappyPack;
  bool m_enableVideoRecording;
  std::string m_videoRecordName;
  int m_videoRecordingDivision;
  int m_videoRecordingFramerate;
};

#endif

