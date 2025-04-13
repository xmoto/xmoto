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

#ifndef __GAME_H__
#define __GAME_H__

#include "LevelsManager.h"
#include "common/VCommon.h"
#include "common/WWW.h"
#include "common/XMotoLoadLevelsInterface.h"
#include "db/xmDatabaseUpdateInterface.h"
#include "helpers/Singleton.h"
#include "input/Input.h"
#include "xmscene/Scene.h"

class XMArguments;
class xmDatabase;
class Img;
class GameApp;
class DrawLib;
class WebRoom;
class WebRooms;
class WebLevels;
class LevelsPack;
class UIWindow;
class UILevelList;
class SoundSample;
class XMotoLoadReplaysInterface;
class UIMsgBox;
class UITabView;
class UIButton;
class UIFrame;
class UIStatic;
class NetServer;
class VirtualLevelsList;
class XMDemo;

/*===========================================================================
  Game application
  ===========================================================================*/
class GameApp
  : public XMotoLoadLevelsInterface
  , public XmDatabaseUpdateInterface
  , public Singleton<GameApp> {
  friend class Singleton<GameApp>;

private:
  GameApp();
  ~GameApp();

public:
  void run(int nNumArgs, char **ppcArgs);
  void run_load(int nNumArgs, char **ppcArgs);
  void run_loop();
  void run_unload();

  /* load level */
  void loadLevelHook(std::string i_level, int i_percentage);
  void updatingDatabase(const std::string &i_message, int i_percentage);

  void reloadTheme();

  void setSpecificReplay(const std::string &i_replay);
  void setSpecificLevelId(const std::string &i_levelID);
  void setSpecificLevelFile(const std::string &i_leveFile);

  /* */
  static double getXMTime(void);
  static int getXMTimeInt(void);
  static std::string getTimeStamp(void);

  // these two functions exists because old xmoto time storage was float for
  // seconds. Int for hundreads is better.
  static float timeToFloat(int i_time);
  static int floatToTime(float ftime);

  void quit(void);
  static void getMousePos(int *pnX, int *pnY);

  Img *grabScreen(void);

  DrawLib *getDrawLib() { return drawLib; };

  UserConfig *getUserConfig() { /* to remove */ return m_userConfig; }

  void switchLevelToFavorite(const std::string &i_levelId,
                             bool v_displayMessage = false);
  void switchLevelToBlacklist(const std::string &i_levelId,
                              bool v_displayMessage = false);

  std::string getWorldRecord(unsigned int i_number,
                             const std::string &LevelID,
                             int &o_highscore_time,
                             std::string &o_highscore_author);
  bool getCurrentMedal(int i_best_room_time,
                       int i_best_player_time,
                       std::string &o_medal);
  bool getNextMedal(const std::string &i_profile,
                    const std::string &i_best_room_author,
                    int i_best_room_time,
                    int i_best_player_time,
                    std::string &o_medal,
                    int &o_medal_time);

  // to call while playing
  void toogleEnableMusic();
  void playMenuMusic(const std::string &i_music); // "" => no music
  void playGameMusic(const std::string &i_music); // "" => no music

  // ask the game to close as soon as possible
  void requestEnd();
  bool isRequestingEnd();

  static TColor getColorFromPlayerNumber(int i_player);
  static TColor getUglyColorFromPlayerNumber(int i_player);

  bool getHighscoreInfos(unsigned int i_number,
                         const std::string &i_id_level,
                         std::string *io_id_profile,
                         std::string *io_url,
                         bool *o_isAccessible);

  void playLevel(const std::string &levelId);
  void playReplay(const std::string &replayId);
  std::string loadDemoReplay(const std::string &demoFile);

  void addLevelToFavorite(const std::string &i_levelId);

  // list played
  void setCurrentPlayingList(VirtualLevelsList *i_levelsList) {
    m_currentPlayingList = i_levelsList;
  }

  void updateWebHighscores();

  std::string getWebRoomURL(unsigned int i_number, xmDatabase *pDb);
  std::string getWebRoomName(unsigned int i_number, xmDatabase *pDb);

  bool isThereANextLevel(const std::string &i_id_level);
  bool isThereAPreviousLevel(const std::string &i_id_level);
  std::string determineNextLevel(const std::string &i_id_level);
  std::string determinePreviousLevel(const std::string &i_id_level);

  void initReplaysFromDir(
    xmDatabase *threadDb,
    XMotoLoadReplaysInterface *pLoadReplaysInterface = NULL);

  void gameScreenshot();
  void enableWWW(bool bValue);
  void enableFps(bool bValue);
  void switchUglyMode(bool bUgly);
  void switchTestThemeMode(bool mode);
  void switchUglyOverMode(bool mode);

  void addReplay(const std::string &i_file,
                 xmDatabase *pDb,
                 bool sendMessage = true);

  NetServer *standAloneServer();

  static void wait(int &io_lastFrameTimeStamp, int &io_frameLate, int i_maxFps);

  bool hasMouseFocus() const { return m_hasMouseFocus; }
  bool hasKeyboardFocus() const { return m_hasKeyboardFocus; }
  bool isIconified() const { return m_isIconified; }

protected:
  void createDefaultConfig();

private:
  void manageEvent(SDL_Event *Event);
  void playMusic(const std::string &i_music); // "" => no music

  void initNetwork(bool i_forceNoServerStarted, bool i_forceNoClientStarted);
  void uninitNetwork();

  ReplayBiker *m_replayBiker; /* link to the replay biker in REPLAYING state */

  std::string m_playingMusic; /* name of the music played to not restart it if
                                 the same must be played on an action */

  // loading
  int m_loadLevelHook_per;

  /* WWW */
  WebRoom *m_pWebHighscores;
  WebLevels *m_pWebLevels;
  ProxySettings m_ProxySettings;
  std::string m_DownloadingInformation;
  std::string m_DownloadingMessage;
  float m_fDownloadTaskProgressLast;

  bool m_bWebHighscoresUpdatedThisSession; /* true: Updated this session */
  bool m_bWebLevelsToDownload; /* true: there are new levels to download */

  /* Various popups */
  UIMsgBox *m_pNotifyMsgBox;
  UIMsgBox *m_pInfoMsgBox;

  /* Main menu buttons and stuff */
  int m_nNumMainMenuButtons;
  UITabView *m_pLevelPackTabs;
  UIButton *m_pMainMenuButtons[10];
  UIFrame *m_pOptionsWindow, *m_pPlayWindow, *m_pReplaysWindow,
    *m_pLevelPacksWindow;
  UIWindow *m_pMainMenu;
  UIFrame *m_pGameInfoWindow;

  /* LEVEL lists */
  VirtualLevelsList *m_currentPlayingList;
  UILevelList *m_pAllLevelsList;
  UILevelList *m_pPlayNewLevelsList;

  UIWindow *m_pLevelInfoFrame;
  UIButton *m_pLevelInfoViewReplayButton;
  UIStatic *m_pBestPlayerText;

  UIWindow *m_pPackLevelInfoFrame;
  UIButton *m_pPackLevelInfoViewReplayButton;
  UIStatic *m_pPackBestPlayerText;

  std::string m_pLevelToShowOnViewHighscore;

  /* if true, don't ask for updating levels */
  bool m_updateAutomaticallyLevels;

  /* a way to know if ODE has been initialized */
  bool m_isODEInitialized;

  /* Internet connection configurator */
  UIFrame *m_pWebConfEditor;
  UIMsgBox *m_pWebConfMsgBox;

  /* Level pack viewer fun */
  UIFrame *m_pLevelPackViewer;

  /* Replay saving UI fun */
  UIMsgBox *m_pSaveReplayMsgBox;

  /* Main loop statics */
  double m_fFrameTime;
  float m_fFPS_Rate;

  std::string m_PlaySpecificReplay;
  std::string m_PlaySpecificLevelId;
  std::string m_PlaySpecificLevelFile;

  UserConfig *m_userConfig;

  DrawLib *drawLib;

  // standalone server;
  NetServer *m_standAloneServer;

  /* Run-time fun */
  bool m_bQuit; /* Quit flag */

  // calculate sleeping time
  int m_lastFrameTimeStamp;
  int m_frameLate;
  int m_loopWithoutNetwork;
  int m_loopWithoutRendering;

  /* Helpers */

  void _UpdateLoadingScreen(const std::string &NextTask = "",
                            int i_percentage = -1);
  void _UpdateLoadingShell(const std::string &NextTask = "",
                           int i_percentage = -1);

  void _UpdateWebLevels(bool bSilent, bool bEnableWeb = true);
  void _DownloadExtraLevels(void);

  /* */
  void _InitWin(bool bInitGraphics);

  // focus
  bool m_hasMouseFocus;
  bool m_hasKeyboardFocus;
  bool m_isIconified;

  // demo
  XMDemo *m_xmdemo;
};

#endif
