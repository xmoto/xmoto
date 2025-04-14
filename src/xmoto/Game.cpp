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

/*
 *  Game application.
 */
#include "Game.h"
#include "Credits.h"
#include "GameText.h"
#include "PhysSettings.h"
#include "Sound.h"
#include "SysMessage.h"
#include "UserConfig.h"
#include "XMDemo.h"
#include "common/Image.h"
#include "common/VFileIO.h"
#include "common/XMSession.h"
#include "db/xmDatabase.h"
#include "drawlib/DrawLib.h"
#include "gui/specific/GUIXMoto.h"
#include "helpers/Log.h"
#include "helpers/Text.h"
#include "input/Input.h"
#include "net/NetClient.h"
#include "xmscene/Bike.h"
#include "xmscene/BikeGhost.h"
#include "xmscene/BikePlayer.h"
#include "xmscene/Camera.h"
#include "xmscene/Entity.h"

#include "Replay.h"
#include "VirtualLevelsList.h"
#include "common/XMotoLoadReplaysInterface.h"
#include "states/StateDeadMenu.h"
#include "states/StateManager.h"
#include "states/StateMessageBox.h"
#include "states/StatePause.h"
#include "states/StatePlaying.h"
#include "states/StatePreplaying.h"
#include "states/StatePreplayingGame.h"
#include "states/StatePreplayingReplay.h"
#include "states/StateWaitServerInstructions.h"
#include "thread/XMThreadStats.h"
#include <curl/curl.h>
#include <iomanip>
#include <sstream>

void GameApp::getMousePos(int *pnX, int *pnY) {
  SDL_GetMouseState(pnX, pnY);
}

/*===========================================================================
Get real-time clock
===========================================================================*/
std::string GameApp::getTimeStamp(void) {
  struct tm *pTime;
  time_t T;
  char cBuf[256] = "";
  time(&T);
  pTime = localtime(&T);
  if (pTime != NULL) {
    snprintf(cBuf,
             256,
             "%d-%02d-%02d %02d:%02d:%02d",
             pTime->tm_year + 1900,
             pTime->tm_mon + 1,
             pTime->tm_mday,
             pTime->tm_hour,
             pTime->tm_min,
             pTime->tm_sec);
  }
  return cBuf;
}

float GameApp::timeToFloat(int i_time) {
  return ((float)(i_time)) / 100.0 +
         0.001; // add 0.001 to avoid 100 => 0.0099999
}

int GameApp::floatToTime(float ftime) {
  return (int)(ftime * 100.0);
}

double GameApp::getXMTime(void) {
  return SDL_GetTicks() / 1000.0f;
}

int GameApp::getXMTimeInt(void) {
  return SDL_GetTicks();
}

/*===========================================================================
Quits the application
===========================================================================*/
void GameApp::quit(void) {
  /* Set quit flag */
  m_bQuit = true;
}

/*===========================================================================
Init
===========================================================================*/
void GameApp::_InitWin(bool bInitGraphics) {
  /* Init SDL */
  if (bInitGraphics == false) {
    if (SDL_Init(SDL_INIT_TIMER) < 0)
      throw Exception("(1) SDL_Init : " + std::string(SDL_GetError()));

    /* No graphics mojo here, thank you */
    return;
  } else {
    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0)
      throw Exception("(2) SDL_Init : " + std::string(SDL_GetError()));
  }

  if (TTF_Init() < 0) {
    throw Exception("Initializing TTF failed: " + std::string(TTF_GetError()));
  }

  atexit(TTF_Quit);
}

GameApp::~GameApp() {
  xmDatabase::destroy("main");

  if (drawLib != NULL) {
    delete drawLib;
  }

  XMSession::instance()->destroy();

  StateManager::cleanStates();
  delete m_userConfig;
  //  delete m_fileGhost;
}

GameApp::GameApp() {
  m_bQuit = false;
  drawLib = NULL;

  m_pNotifyMsgBox = NULL;
  m_pInfoMsgBox = NULL;

  m_pWebConfEditor = NULL;
  m_pWebConfMsgBox = NULL;

  m_pSaveReplayMsgBox = NULL;
  m_pReplaysWindow = NULL;
  m_pLevelPacksWindow = NULL;
  m_pLevelPackViewer = NULL;
  m_pGameInfoWindow = NULL;
  m_fFrameTime = 0;
  m_fFPS_Rate = 0;
  m_updateAutomaticallyLevels = false;

  m_pWebHighscores = NULL;
  m_pWebLevels = NULL;
  m_fDownloadTaskProgressLast = 0;
  m_bWebHighscoresUpdatedThisSession = false;
  m_bWebLevelsToDownload = false;

  m_currentPlayingList = NULL;

  m_lastFrameTimeStamp = -1;
  m_frameLate = 0;
  m_loopWithoutNetwork = 0;
  m_loopWithoutRendering = 0;

  // assume all focus at startup
  m_hasMouseFocus = true;
  m_hasKeyboardFocus = true;
  m_isIconified = false;

  m_standAloneServer = NULL;

  m_userConfig = new UserConfig();

  m_xmdemo = NULL;
  m_loadLevelHook_per = 0;
}

/*===========================================================================
Screenshooting
===========================================================================*/
void GameApp::gameScreenshot() {
  Img *pShot = getDrawLib()->grabScreen();

  std::string v_ShotsDir;
  std::string v_ShotExtension;
  std::string v_destFile;
  int nShot = 0;
  char v_val[5];

  v_ShotsDir = XMFS::getUserDir(FDT_DATA) + std::string("/Screenshots");
  XMFS::mkArborescenceDir(v_ShotsDir);
  v_ShotExtension = XMSession::instance()->screenshotFormat();

  /* User preference for format? must be either jpeg or png */
  if (v_ShotExtension != "jpeg" && v_ShotExtension != "jpg" &&
      v_ShotExtension != "png") {
    LogWarning("unsupported screenshot format '%s', using png instead!",
               v_ShotExtension.c_str());
    v_ShotExtension = "png";
  }

  do {
    nShot++;
    if (nShot > 9999) {
      LogWarning("Too many screenshots !");
      delete pShot;
      return;
    }

    snprintf(v_val, 5, "%04d", nShot);
    v_destFile =
      v_ShotsDir + "/screenshot" + std::string(v_val) + "." + v_ShotExtension;
  } while (XMFS::fileExists(FDT_DATA, v_destFile));
  try {
    pShot->saveFile(v_destFile.c_str());
  } catch (Exception &e) {
    LogError(
      std::string("Unable to save the screenshot: " + e.getMsg()).c_str());
  }

  delete pShot;
}

void GameApp::enableFps(bool bValue) {
  XMSession::instance()->setFps(XMSession::instance()->fps() == false);
  if (XMSession::instance()->fps()) {
    SysMessage::instance()->displayText(SYS_MSG_FPS_ENABLED);
  } else {
    SysMessage::instance()->displayText(SYS_MSG_FPS_DISABLED);
  }
}

void GameApp::enableWWW(bool bValue) {
  XMSession::instance()->setWWW(XMSession::instance()->www() == false);
  if (XMSession::instance()->www()) {
    SysMessage::instance()->displayText(SYS_MSG_WWW_ENABLED);
  } else {
    SysMessage::instance()->displayText(SYS_MSG_WWW_DISABLED);
  }
}

bool GameApp::getCurrentMedal(int i_best_room_time,
                              int i_best_player_time,
                              std::string &o_medal) {
  int v_gold_time, v_silver_time, v_bronze_time;

  /* no best room time, so no next medal */
  if (i_best_room_time < 0 || i_best_player_time < 0) {
    return false;
  }

  /* get medal times*/
  v_gold_time = i_best_room_time / 0.95;
  v_silver_time = i_best_room_time / 0.90;
  v_bronze_time = i_best_room_time / 0.80;

  if (i_best_player_time > v_bronze_time) {
    return false;
  }

  if (i_best_player_time > v_silver_time) {
    o_medal = GAMETEXT_MEDAL_BRONZE;
    return true;
  }

  if (i_best_player_time > v_gold_time) {
    o_medal = GAMETEXT_MEDAL_SILVER;
    return true;
  }

  if (i_best_player_time >=
      i_best_room_time) { /* >= because, PLATINUM is when you're the owner, not
                             the same time */
    o_medal = GAMETEXT_MEDAL_GOLD;
    return true;
  }

  o_medal = GAMETEXT_MEDAL_PLATINIUM;
  return true;
}

bool GameApp::getNextMedal(const std::string &i_profile,
                           const std::string &i_best_room_author,
                           int i_best_room_time,
                           int i_best_player_time,
                           std::string &o_medal,
                           int &o_medal_time) {
  int v_gold_time, v_silver_time, v_bronze_time;

  if (i_profile == i_best_room_author) {
    return false;
  }

  /* no best room time, so no next medal */
  if (i_best_room_time < 0) {
    return false;
  }

  /* get medal times*/
  v_gold_time = i_best_room_time / 0.95;
  v_silver_time = i_best_room_time / 0.90;
  v_bronze_time = i_best_room_time / 0.80;

  /* no player best time, then, bronze */
  if (i_best_player_time < 0) {
    o_medal = GAMETEXT_MEDAL_BRONZE;
    o_medal_time = v_bronze_time;
    return true;
  }

  if (i_best_player_time > v_bronze_time) {
    o_medal = GAMETEXT_MEDAL_BRONZE;
    o_medal_time = v_bronze_time;
    return true;
  }

  if (i_best_player_time > v_silver_time) {
    o_medal = GAMETEXT_MEDAL_SILVER;
    o_medal_time = v_silver_time;
    return true;
  }

  if (i_best_player_time > v_gold_time) {
    o_medal = GAMETEXT_MEDAL_GOLD;
    o_medal_time = v_gold_time;
    return true;
  }

  if (i_best_player_time >=
      i_best_room_time) { /* hum >= depending on whether
                             you're the owner or not, but
                             the test is done at the first
                             time, you're not the author */
    o_medal = GAMETEXT_MEDAL_PLATINIUM;
    o_medal_time = i_best_room_time;
    return true;
  }

  /* no missing medal, you've all of them */
  return false;
}

std::string GameApp::getWorldRecord(unsigned int i_number,
                                    const std::string &LevelID,
                                    int &o_highscore_time,
                                    std::string &o_highscore_author) {
  char **v_result;
  unsigned int nrow;
  std::string v_roomName;
  std::string v_id_profile;
  int v_finishTime = 0;
  xmDatabase *v_pDb = xmDatabase::instance("main");

  o_highscore_time = -1;

  v_result =
    v_pDb->readDB("SELECT a.name, b.id_profile, b.id_profile, b.finishTime "
                  "FROM webrooms AS a LEFT OUTER JOIN webhighscores AS b "
                  "ON (a.id_room = b.id_room "
                  "AND b.id_level=\"" +
                    xmDatabase::protectString(LevelID) +
                    "\") "
                    "WHERE a.id_room=" +
                    XMSession::instance()->idRoom(i_number) + ";",
                  nrow);
  if (nrow != 1) {
    /* should not happend */
    v_pDb->read_DB_free(v_result);
    return GAMETEXT_WORLDRECORDNA + std::string(": WR");
  }
  v_roomName = v_pDb->getResult(v_result, 4, 0, 0);
  if (v_pDb->getResult(v_result, 4, 0, 1) != NULL) {
    o_highscore_author = v_pDb->getResult(v_result, 4, 0, 1);
  }
  if (v_pDb->getResult(v_result, 4, 0, 1) != NULL) {
    v_id_profile = v_pDb->getResult(v_result, 4, 0, 2);
    v_finishTime = atoi(v_pDb->getResult(v_result, 4, 0, 3));
  }
  v_pDb->read_DB_free(v_result);

  /* highscore found */
  if (v_id_profile != "") {
    o_highscore_time = v_finishTime;
    return formatTime(v_finishTime) + ": " + v_roomName + std::string(" (") +
           v_id_profile + std::string(")");
  }

  /* no highscore */
  return GAMETEXT_WORLDRECORDNA + std::string(": ") + v_roomName;
}

TColor GameApp::getColorFromPlayerNumber(int i_player) {
  // try to find nice colors for first player, then automatic
  switch (i_player) {
    case 0:
      return TColor(255, 255, 255, 0);
      break;

    case 1:
      return TColor(125, 125, 125, 0);
      break;

    case 2:
      return TColor(200, 100, 50, 0);
      break;

    case 3:
      return TColor(50, 255, 255, 0);
      break;

    default:
      return TColor(
        (i_player * 5) % 255, (i_player * 20) % 255, (i_player * 50) % 255, 0);
  }

  return TColor(255, 255, 255, 0);
}

TColor GameApp::getUglyColorFromPlayerNumber(int i_player) {
  // try to find nice colors for first player, then automatic
  Color v_color;

  switch (i_player) {
    case 0:
      v_color = Theme::instance()->getPlayerTheme()->getUglyRiderColor();
      return TColor(GET_RED(v_color), GET_GREEN(v_color), GET_BLUE(v_color));
      break;

    case 1:
      return TColor(125, 125, 125, 0);
      break;

    case 2:
      return TColor(255, 50, 50);
      break;

    case 3:
      return TColor(50, 50, 255);
      break;

    default:
      return TColor(
        (i_player * 5) % 255, (i_player * 20) % 255, (i_player * 50) % 255, 0);
  }
}

void GameApp::switchUglyMode(bool bUgly) {
  XMSession::instance()->setUgly(bUgly);
}

void GameApp::switchTestThemeMode(bool mode) {
  XMSession::instance()->setTestTheme(mode);
}

void GameApp::switchUglyOverMode(bool mode) {
  XMSession::instance()->setUglyOver(mode);
}

void GameApp::reloadTheme() {
  const auto loadTheme = [](const std::string &name) {
    auto filename = xmDatabase::instance("main")->themes_getFileName(name);
    Theme::instance()->load(FDT_DATA, filename);
  };

  try {
    loadTheme(XMSession::instance()->theme());
  } catch (Exception &e) {
    /* unable to load the theme, load the default one */
    loadTheme(DEFAULT_THEME);
  }
  LogInfo("Using theme: %s", Theme::instance()->Name().c_str());
}

void GameApp::initReplaysFromDir(
  xmDatabase *threadDb,
  XMotoLoadReplaysInterface *pLoadReplaysInterface) {
  std::vector<std::string> ReplayFiles;

  ReplayFiles = XMFS::findPhysFiles(FDT_DATA, "Replays/*.rpl");
  threadDb->replays_add_begin();

  for (unsigned int i = 0; i < ReplayFiles.size(); i++) {
    try {
      if (XMFS::getFileBaseName(ReplayFiles[i]) == "Latest") {
        continue;
      }
      addReplay(ReplayFiles[i], threadDb, false);
      if (pLoadReplaysInterface != NULL) {
        pLoadReplaysInterface->loadReplayHook(
          ReplayFiles[i], (int)((i * 100) / ((float)ReplayFiles.size())));
      }

    } catch (Exception &e) {
      // ok, forget this replay
    }
  }
  threadDb->replays_add_end();
}

void GameApp::addReplay(const std::string &i_file,
                        xmDatabase *pDb,
                        bool sendMessage) {
  ReplayInfo *rplInfos;

  rplInfos = Replay::getReplayInfos(XMFS::getFileBaseName(i_file));
  if (rplInfos == NULL) {
    throw Exception("Unable to extract data from replay file");
  }

  try {
    pDb->replays_add(rplInfos->Level,
                     rplInfos->Name,
                     rplInfos->Player,
                     rplInfos->IsFinished,
                     rplInfos->finishTime);
    if (sendMessage == true)
      StateManager::instance()->sendAsynchronousMessage("REPLAYS_UPDATED");

  } catch (Exception &e2) {
    delete rplInfos;
    throw e2;
  }
  delete rplInfos;
}

void GameApp::setSpecificReplay(const std::string &i_replay) {
  m_PlaySpecificReplay = i_replay;
}

void GameApp::setSpecificLevelId(const std::string &i_levelID) {
  m_PlaySpecificLevelId = i_levelID;
}

void GameApp::setSpecificLevelFile(const std::string &i_leveFile) {
  m_PlaySpecificLevelFile = i_leveFile;
}

void GameApp::playLevel(const std::string &levelId) {
  if (XMSession::instance()->clientGhostMode() == false &&
      NetClient::instance()->isConnected()) {
    StateManager::instance()->pushState(new StateWaitServerInstructions());
  } else {
    StateManager::instance()->pushState(
      new StatePreplayingGame(levelId, false));
  }
  LogInfo("Playing as '%s'...", XMSession::instance()->profile().c_str());
}

void GameApp::playReplay(const std::string &replayId) {
  StateManager::instance()->pushState(
    new StatePreplayingReplay(replayId, false));
}

std::string GameApp::loadDemoReplay(const std::string &demoFile) {
  std::string demoReplay;

  if (m_xmdemo != NULL) {
    m_xmdemo->destroyFiles();
    delete m_xmdemo;
    m_xmdemo = NULL;
  }

  /* demo : download the level and the replay
     load the level as external,
     play the replay
   */
  try {
    m_xmdemo = new XMDemo(demoFile);
    LogInfo("Loading demo file %s\n", demoFile.c_str());

    _UpdateLoadingScreen(GAMETEXT_DLLEVEL);
    m_xmdemo->getLevel(XMSession::instance()->proxySettings());
    _UpdateLoadingScreen(GAMETEXT_DLREPLAY);
    m_xmdemo->getReplay(XMSession::instance()->proxySettings());

    try {
      LevelsManager::instance()->addExternalLevel(
        m_xmdemo->levelFile(), xmDatabase::instance("main"), false);
    } catch (Exception &e) {
      LogError("Can't add level %s as external level",
               m_xmdemo->levelFile().c_str());
    }
    demoReplay = m_xmdemo->replayFile();
  } catch (Exception &e) {
    throw e;
  }
  return demoReplay;
}

void GameApp::addLevelToFavorite(const std::string &i_levelId) {
  LevelsManager::instance()->addToFavorite(
    XMSession::instance()->profile(), i_levelId, xmDatabase::instance("main"));
}

void GameApp::switchLevelToFavorite(const std::string &i_levelId,
                                    bool v_displayMessage) {
  if (LevelsManager::instance()->isInFavorite(XMSession::instance()->profile(),
                                              i_levelId,
                                              xmDatabase::instance("main"))) {
    LevelsManager::instance()->delFromFavorite(XMSession::instance()->profile(),
                                               i_levelId,
                                               xmDatabase::instance("main"));
    if (v_displayMessage) {
      SysMessage::instance()->displayText(GAMETEXT_LEVEL_DELETED_FROM_FAVORITE);
    }
  } else {
    LevelsManager::instance()->addToFavorite(XMSession::instance()->profile(),
                                             i_levelId,
                                             xmDatabase::instance("main"));
    if (v_displayMessage) {
      SysMessage::instance()->displayText(GAMETEXT_LEVEL_ADDED_TO_FAVORITE);
    }
  }
}

void GameApp::switchLevelToBlacklist(const std::string &i_levelId,
                                     bool v_displayMessage) {
  if (LevelsManager::instance()->isInBlacklist(XMSession::instance()->profile(),
                                               i_levelId,
                                               xmDatabase::instance("main"))) {
    LevelsManager::instance()->delFromBlacklist(
      XMSession::instance()->profile(),
      i_levelId,
      xmDatabase::instance("main"));
    if (v_displayMessage) {
      SysMessage::instance()->displayText(
        GAMETEXT_LEVEL_DELETED_FROM_BLACKLIST);
    }
  } else {
    LevelsManager::instance()->addToBlacklist(XMSession::instance()->profile(),
                                              i_levelId,
                                              xmDatabase::instance("main"));
    if (v_displayMessage) {
      SysMessage::instance()->displayText(GAMETEXT_LEVEL_ADDED_TO_BLACKLIST);
    }
  }
}

void GameApp::requestEnd() {
  m_bQuit = true;
}

bool GameApp::isRequestingEnd() {
  return m_bQuit;
}

void GameApp::playMenuMusic(const std::string &i_music) {
  if (XMSession::instance()->enableAudio() &&
      XMSession::instance()->enableMenuMusic()) {
    playMusic(i_music);
  } else {
    playMusic("");
  }
}

void GameApp::playGameMusic(const std::string &i_music) {
  if (XMSession::instance()->enableAudio() &&
      XMSession::instance()->enableGameMusic()) {
    playMusic(i_music);
  } else {
    playMusic("");
  }
}

void GameApp::playMusic(const std::string &i_music) {
  LogDebug("Playing '%s'\n", i_music.c_str());

  if (i_music != m_playingMusic) {
    try {
      if (i_music == "") {
        m_playingMusic = "";
        Sound::stopMusic();
      } else {
        m_playingMusic = i_music;
        Sound::playMusic(Theme::instance()->getMusic(i_music)->FilePath());
      }
    } catch (Exception &e) {
      LogWarning("PlayMusic(%s) failed", i_music.c_str());
      Sound::stopMusic();
    }
  }
}

void GameApp::toogleEnableMusic() {
  m_playingMusic = ""; // tell the game the last music played is "" otherwise,
  // it doesn't know when music is disable then enable back
  XMSession::instance()->setEnableAudio(!XMSession::instance()->enableAudio());
  Sound::setActiv(XMSession::instance()->enableAudio());

  if (XMSession::instance()->enableAudio()) {
    SysMessage::instance()->displayText(SYS_MSG_AUDIO_ENABLED);
  } else {
    SysMessage::instance()->displayText(SYS_MSG_AUDIO_DISABLED);
  }
  StateManager::instance()->sendAsynchronousMessage("ENABLEAUDIO_CHANGED");
}

std::string GameApp::getWebRoomURL(unsigned int i_number, xmDatabase *pDb) {
  char **v_result;
  unsigned int nrow;
  std::string v_url;

  v_result = pDb->readDB("SELECT highscoresUrl FROM webrooms WHERE id_room=" +
                           XMSession::instance()->idRoom(i_number) + ";",
                         nrow);
  if (nrow != 1) {
    pDb->read_DB_free(v_result);
    return DEFAULT_WEBROOMS_URL;
  }
  v_url = pDb->getResult(v_result, 1, 0, 0);
  pDb->read_DB_free(v_result);

  return v_url;
}

std::string GameApp::getWebRoomName(unsigned int i_number, xmDatabase *pDb) {
  char **v_result;
  unsigned int nrow;
  std::string v_name;

  /* set the room name ; set to WR if it cannot be determined */
  v_name = "WR";

  v_result = pDb->readDB("SELECT name FROM webrooms WHERE id_room=" +
                           XMSession::instance()->idRoom(i_number) + ";",
                         nrow);
  if (nrow == 1) {
    v_name = pDb->getResult(v_result, 1, 0, 0);
  }
  pDb->read_DB_free(v_result);

  return v_name;
}

bool GameApp::getHighscoreInfos(unsigned int i_number,
                                const std::string &i_id_level,
                                std::string *o_id_profile,
                                std::string *o_url,
                                bool *o_isAccessible) {
  char **v_result;
  unsigned int nrow;
  xmDatabase *pDb = xmDatabase::instance("main");

  v_result = pDb->readDB(
    "SELECT id_profile, fileUrl FROM webhighscores WHERE id_level=\"" +
      xmDatabase::protectString(i_id_level) +
      "\" AND id_room=" + XMSession::instance()->idRoom(0) + ";",
    nrow);
  if (nrow == 0) {
    pDb->read_DB_free(v_result);
    return false;
  }

  *o_id_profile = pDb->getResult(v_result, 2, 0, 0);
  *o_url = pDb->getResult(v_result, 2, 0, 1);
  pDb->read_DB_free(v_result);

  /* search if the replay is already downloaded */
  if (pDb->replays_exists(XMFS::getFileBaseName(*o_url))) {
    *o_isAccessible = true;
  } else {
    *o_isAccessible = XMSession::instance()->www();
  }

  return true;
}

void GameApp::loadLevelHook(std::string i_level, int i_percentage) {
  // percentage updated
  if (m_loadLevelHook_per != i_percentage) {
    std::ostringstream v_percentage;
    v_percentage << i_percentage;
    v_percentage << "%";

    if (DrawLib::isInitialized()) {
      _UpdateLoadingScreen(std::string(GAMETEXT_LOAD_LEVEL_HOOK) +
                             std::string("\n") + v_percentage.str(),
                           i_percentage);
      /* pump events to so that windows don't think the appli is crashed */
      SDL_PumpEvents();
    } else {
      _UpdateLoadingShell(std::string(GAMETEXT_LOAD_LEVEL_HOOK), i_percentage);
    }

    // update percentage
    m_loadLevelHook_per = i_percentage;
  }
}

void GameApp::updatingDatabase(const std::string &i_message, int i_percentage) {
  if (DrawLib::isInitialized()) {
    _UpdateLoadingScreen(i_message, i_percentage);
    /* pump events to so that windows don't think the appli is crashed */
    SDL_PumpEvents();
  } else {
    _UpdateLoadingShell(i_message, i_percentage);
  }
}

NetServer *GameApp::standAloneServer() {
  return m_standAloneServer;
}

bool GameApp::isThereAPreviousLevel(const std::string &i_id_level) {
  return determinePreviousLevel(i_id_level) != "";
}

bool GameApp::isThereANextLevel(const std::string &i_id_level) {
  return determineNextLevel(i_id_level) != "";
}

std::string GameApp::determineNextLevel(const std::string &i_id_level) {
  if (m_currentPlayingList == NULL) {
    return "";
  }
  return m_currentPlayingList->determineNextLevel(i_id_level);
}

std::string GameApp::determinePreviousLevel(const std::string &i_id_level) {
  if (m_currentPlayingList == NULL) {
    return "";
  }
  return m_currentPlayingList->determinePreviousLevel(i_id_level);
}
