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

enum GraphicsLevel { GFX_LOW, GFX_MEDIUM, GFX_HIGH };

enum MultiGameMode {
  MULTI_MODE_TIME_ATTACK,
  MULTI_MODE_RACE,
  MULTI_MODE_TEAMPLAY
};

#include "XMSession_default.h"

#include "helpers/MultiSingleton.h"
#include "helpers/TFunctor.h"
#include <string>

/*
  XMSession   : current session options
  UserConfig  : config file options
  XMArguments : command line options
  DB::profiles_configs : config specific to the profile

  first, XMSession constructor init session values,
  then, UserConfig are loaded, and finally, XMArguments overwrite session's
  values if they are defined
*/

class XMArguments;
class UserConfig;
class xmDatabase;

class ProxySettings {
public:
  ProxySettings();
  void operator=(const ProxySettings &i_copy);

  void setServer(std::string p_server);
  void setPort(long p_port);
  void setType(
    const std::string &p_type); /* CURLPROXY_HTTP OR CURLPROXY_SOCKS5 */
  void setAuthentification(std::string p_user, std::string p_password);

  void setDefault();
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

  /* default means : curl try to find default values (no proxy, or environment
   * vars) */
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

class XMSession : public MultiSingleton<XMSession> {
  friend class MultiSingleton<XMSession>;

private:
  XMSession();
  virtual ~XMSession() {};

public:
  void loadArgs(const XMArguments *i_xmargs);
  void loadConfig(UserConfig *m_Config, bool loadProfile = true);
  void loadProfile(const std::string &i_id_profile,
                   xmDatabase *pDb); /* give the database as argument ; to be
                                        use that any developper call it before
                                        db is initialized */
  void save(UserConfig *m_Config, xmDatabase *pDb);
  void saveProfile(xmDatabase *pDb);
  void setToDefault();
  bool isVerbose() const;
  static void createDefaultConfig(UserConfig *v_config);

  void setUseGraphics(bool i_value);
  bool useGraphics() const;
  int resolutionWidth() const;
  int resolutionHeight() const;
  int maxRenderFps() const;
  bool windowed() const;
  void setResolutionWidth(int i_value);
  void setResolutionHeight(int i_value);
  void setMaxRenderFps(int i_value);
  void setWindowed(bool i_value);
  bool useThemeCursor() const;
  void setUseThemeCursor(bool i_value);
  bool glExts() const;
  bool glVOBS() const;
  std::string drawlib() const;
  bool www() const;
  void setWWW(bool i_value);
  bool benchmark() const;
  bool debug() const;
  bool sqlTrace() const;
  std::string profile() const;
  void setProfile(const std::string &i_profile);
  std::string sitekey() const;
  std::string wwwPassword() const;
  void setWwwPassword(const std::string &i_password);
  bool gDebug() const;
  std::string gDebugFile() const;
  bool timedemo() const;
  bool fps() const;
  void setFps(bool i_value);
  bool ugly() const;
  void setUgly(bool i_value);
  bool uglyOver() const;
  void setUglyOver(bool i_value);
  bool hideSpritesUgly() const;
  void setHideSpritesUgly(bool i_value);
  bool hideSpritesMinimap() const;
  void setHideSpritesMinimap(bool i_value);
  bool noLog() const;
  bool testTheme() const;
  void setTestTheme(bool i_value);
  bool ghostStrategy_MYBEST() const;
  void setGhostStrategy_MYBEST(bool i_value);
  bool ghostStrategy_THEBEST() const;
  void setGhostStrategy_THEBEST(bool i_value);
  bool ghostStrategy_BESTOFREFROOM() const;
  void setGhostStrategy_BESTOFREFROOM(bool i_value);
  bool ghostStrategy_BESTOFOTHERROOMS() const;
  void setGhostStrategy_BESTOFOTHERROOMS(bool i_value);
  bool autosaveHighscoreReplays() const;
  void setAutosaveHighscoreReplays(bool i_value);
  void setEnableGhosts(bool i_value);
  bool enableGhosts() const;
  bool disableAnimations() const;
  bool permanentConsole() const;
  bool showGameInformationInConsole() const;
  unsigned int consoleSize() const;
  void setEnableEngineSound(bool i_value);
  bool enableEngineSound() const;
  void setShowEngineCounter(bool i_value);
  bool showEngineCounter() const;
  void setShowMinimap(bool i_value);
  bool showMinimap() const;
  void setMultiStopWhenOneFinishes(bool i_value);
  bool MultiStopWhenOneFinishes() const;
  void setMultiGameMode(MultiGameMode i_value);
  MultiGameMode multiGameMode() const;
  void setEnableMenuMusic(bool i_value);
  bool enableMenuMusic() const;
  void setEnableGameMusic(bool i_value);
  bool enableGameMusic() const;
  void setEnableInitZoom(bool i_value);
  bool enableInitZoom() const;
  void setEnableActiveZoom(bool i_value);
  bool enableActiveZoom() const;
  void setEnableTrailCam(bool i_value);
  bool enableTrailCam() const;
  void setRenderGhostTrail(bool i_value);
  bool renderGhostTrail() const;
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
  void setEnableContextHelp(bool i_value);
  bool enableContextHelp() const;
  void setTheme(const std::string &i_value);
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
  void setNextMedalInGame(bool i_value);
  bool showNextMedalInGame() const;
  void setIdRoom(unsigned int i_number, const std::string &i_value);
  std::string idRoom(unsigned int i_number) const;
  void setNbRoomsEnabled(unsigned int i_value);
  unsigned int nbRoomsEnabled() const;
  void setShowGhostTimeDifference(bool i_value);
  bool showGhostTimeDifference() const;
  void setGhostMotionBlur(bool i_value);
  bool ghostMotionBlur() const;
  void setShowGhostsInfos(bool i_value);
  bool showGhostsInfos() const;
  void setShowBikersArrows(bool i_value);
  bool showBikersArrows() const;
  void setHideGhosts(bool i_value);
  bool hideGhosts() const;
  float replayFrameRate() const;
  std::string webThemesURL() const;
  std::string webThemesURLBase() const;
  std::string webRoomsURL() const;
  ProxySettings *proxySettings();
  void setProxySettings(const ProxySettings &i_value);
  void markProxyUpdated();
  void setWebConfAtInit(bool i_value);
  bool webConfAtInit() const;
  bool storeReplays() const;
  bool enableReplayInterpolation() const;
  void setEnableReplayInterpolation(bool i_value);
  std::string uploadHighscoreUrl() const;
  std::string screenshotFormat() const;
  std::string language() const;
  void setLanguage(const std::string &i_value);
  void setNotifyAtInit(bool i_value);
  bool notifyAtInit() const;
  std::string webLevelsUrl() const;
  std::string uploadDbSyncUrl() const;
  bool mirrorMode() const;
  void setMirrorMode(bool i_value);
  bool enableJoysticks() const;
  bool adminMode() const;
  void setAdminMode(bool i_value);
  void setEnableJoysticks(bool i_value);
  bool useCrappyPack() const;
  void setUseCrappyPack(bool i_value);
  bool useChildrenCompliant() const;
  bool forceChildrenCompliant() const;
  void setChildrenCompliant(bool i_value);
  bool enableVideoRecording() const;
  std::string videoRecordName() const;
  int videoRecordingDivision() const;
  int videoRecordingFramerate() const;
  int videoRecordingStartTime();
  int videoRecordingEndTime();
  bool hidePlayingInformation();
  void setDbsynchronizeOnQuit(bool i_value);
  bool dbsynchronizeOnQuit() const;
  void setBeatingMode(bool i_value);
  void setDisableAnimations(bool i_value);
  void setPermanentConsole(bool i_value);
  void setShowGameInformationInConsole(bool i_value);
  void setConsoleSize(unsigned int i_value);
  bool beatingMode() const;
  void setWebForms(bool i_value);
  bool webForms() const;
  bool serverStartAtStartup() const;
  void setServerStartAtStartup(bool i_value);
  bool clientConnectAtStartup() const;
  void setClientConnectAtStartup(bool i_value);
  int serverPort() const;
  void setServerPort(int i_value);
  unsigned int serverMaxClients() const;
  void setServerMaxClients(unsigned int i_value);
  std::string clientServerName() const;
  void setClientServerName(const std::string &i_value);
  bool clientGhostMode() const;
  void setClientGhostMode(bool i_value);
  int clientServerPort() const;
  void setClientServerPort(int i_value);
  int clientFramerateUpload() const;
  void setClientFramerateUpload(int i_value);
  bool musicOnAllLevels() const;
  void setMusicOnAllLevels(bool i_value);

  // there are two dbSync values, one for the profile side, one on the serveur
  // side
  // note that because these values must always be synchronised with db, there
  // are always read and write from and to the db
  /**
     dbSync field is the revision in which you upload the line.
     -> tagging lines where upd=0 and dbsync is null to the last dbsync
     -> sending via xml all the lines where synchronized=0

     if server answer ko : do nothing
     if server answer ok : update synchronized to 1
     if xmoto crashes, when you restart, xmoto doesn't know the answer of the
     server :
     - if the answer was ok, lines with synchronized=0 are resend, but will not
     be used on the server side on the next update
     - if the answer was ko, lines with synchronized=0 are resend, and will be
     used on the server side.
  */
  int dbSync(xmDatabase *pDb, const std::string &i_id_profile);
  void setDbSync(xmDatabase *pDb,
                 const std::string &i_id_profile,
                 int i_dbSync);
  int dbSyncServer(xmDatabase *pDb, const std::string &i_id_profile);
  void setDbSyncServer(xmDatabase *pDb,
                       const std::string &i_id_profile,
                       int i_dbSyncServer);

private:
  bool m_verbose;
  int m_resolutionWidth;
  int m_resolutionHeight;
  int m_bpp; // kept for backward compatibility
  int m_maxRenderFps;
  bool m_windowed;
  bool m_useThemeCursor;
  bool m_glExts;
  bool m_glVOBS;
  std::string m_drawlib;
  bool m_www;
  bool m_benchmark;
  bool m_debug;
  bool m_sqlTrace;
  std::string m_profile;
  std::string m_sitekey;
  std::string m_www_password;
  bool m_gdebug;
  std::string m_gdebug_file;
  bool m_timedemo;
  bool m_fps;
  bool m_ugly;
  bool m_uglyOver;
  bool m_hideSpritesUgly;
  bool m_hideSpritesMinimap;
  bool m_testTheme;
  bool m_autosaveHighscoreReplays;
  bool m_disableAnimations;
  bool m_ghostStrategy_MYBEST;
  bool m_ghostStrategy_THEBEST;
  bool m_ghostStrategy_BESTOFREFROOM;
  bool m_ghostStrategy_BESTOFOTHERROOMS;
  bool m_enableGhosts;
  bool m_enableEngineSound;
  bool m_showEngineCounter;
  bool m_showMinimap;
  bool m_multiStopWhenOneFinishes; /* in multiplayer, stop the game when one
                                      finishes the level */
  MultiGameMode m_multiGameMode; /* sets mode for multiplayer game: race, time
                                    attack or teamplay */
  bool m_enableMenuMusic;
  bool m_enableGameMusic;
  bool m_enableInitZoom; /* true: Perform initial level scroll/zoom */
  bool m_enableActiveZoom;
  bool m_enableTrailCam;
  bool m_renderGhostTrail;
  bool m_enableDeadAnimation;
  bool m_enablePermanentConsole;
  bool m_showGameInformationInConsole;
  int m_consoleSize;
  GraphicsLevel m_menuGraphics;
  GraphicsLevel m_gameGraphics;
  int m_quickStartQualityMIN;
  int m_quickStartQualityMAX;
  int m_quickStartDifficultyMIN;
  int m_quickStartDifficultyMAX;
  int m_multiNbPlayers;
  bool m_multiScenes;
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
  bool m_showNextMedalInGame;
  std::string m_uploadHighscoreUrl;
  std::string m_idRoom[ROOMS_NB_MAX];
  unsigned int m_nbRoomsEnabled;
  bool m_showGhostTimeDifference;
  bool m_ghostMotionBlur;
  bool m_showGhostsInfos;
  bool m_showBikersArrows;
  bool m_hideGhosts;
  float m_replayFrameRate;
  std::string m_webThemesURL;
  std::string m_webThemesURLBase;
  std::string m_webRoomsURL;
  ProxySettings m_proxySettings;
  bool m_webConfAtInit;
  bool m_storeReplays;
  bool m_enableReplayInterpolation;
  std::string m_screenshotFormat;
  std::string m_language;
  bool m_notifyAtInit;
  std::string m_webLevelsUrl;
  std::string m_uploadDbSyncUrl;
  bool m_mirrorMode;
  bool m_useCrappyPack;
  bool m_useChildrenCompliant;
  bool m_forceChildrenCompliant;
  bool m_enableVideoRecording;
  std::string m_videoRecordName;
  int m_videoRecordingDivision;
  int m_videoRecordingFramerate;
  int m_videoRecordingStartTime;
  int m_videoRecordingEndTime;
  bool m_hidePlayingInformation;
  bool m_dbsynchronizeOnQuit;
  bool m_enableJoysticks;
  bool m_beatingMode;
  bool m_webForms;
  bool m_serverStartAtStartup;
  bool m_clientConnectAtStartup;
  int m_serverPort;
  unsigned int m_serverMaxClients;
  std::string m_clientServerName;
  int m_clientServerPort;
  int m_clientFramerateUpload;
  bool m_clientGhostMode;
  bool m_musicOnAllLevels;
  bool m_noLog;
  bool m_adminMode;

  bool m_bSafemodeActive;

public:
  inline bool isSafemodeActive() { return m_bSafemodeActive; }
  inline void setSafemodeActive(bool isSafemodeActive) {
    m_bSafemodeActive = isSafemodeActive;
  }
};

#endif
