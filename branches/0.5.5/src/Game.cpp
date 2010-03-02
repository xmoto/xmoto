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
#include "VFileIO.h"
#include "Sound.h"
#include "PhysSettings.h"
#include "Input.h"
#include "xmscene/Bike.h"
#include "xmscene/BikeGhost.h"
#include "xmscene/BikePlayer.h"
#include "db/xmDatabase.h"
#include "helpers/Log.h"
#include "helpers/Text.h"
#include "XMSession.h"
#include "drawlib/DrawLib.h"
#include "Image.h"
#include "SysMessage.h"
#include "GameText.h"
#include "Credits.h"
#include "gui/specific/GUIXMoto.h"
#include "xmscene/Camera.h"
#include "xmscene/Entity.h"
#include "UserConfig.h"
#include "VXml.h"

#include <curl/curl.h>
#include <iomanip>
#include "states/StateManager.h"
#include "states/StatePause.h"
#include "states/StateDeadMenu.h"
#include "states/StatePlaying.h"
#include "states/StatePreplaying.h"
#include "states/StateMessageBox.h"
#include "XMotoLoadReplaysInterface.h"
#include "Replay.h"
#include <sstream>

  bool GameApp::haveMouseMoved() {
    int nX,nY;
    SDL_GetRelativeMouseState(&nX,&nY);
    return (nX == 0 && nY == 0);
  }

  void GameApp::getMousePos(int *pnX,int *pnY) {
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
    if(pTime != NULL) {
      snprintf(cBuf,256, "%d-%02d-%02d %02d:%02d:%02d",
	       pTime->tm_year+1900, pTime->tm_mon+1, pTime->tm_mday,
	       pTime->tm_hour, pTime->tm_min, pTime->tm_sec);                    
    }
    return cBuf;
  }

  float GameApp::timeToFloat(int i_time) {
    return ((float)(i_time)) / 100.0 + 0.001; // add 0.001 to avoid 100 => 0.0099999
  }

  int GameApp::floatToTime(float ftime) {
    return (int)(ftime*100.0);
  }

  double GameApp::getXMTime(void) {
    return SDL_GetTicks() / 1000.0f;
  }

  int GameApp::getXMTimeInt(void){
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
    if(bInitGraphics == false) {
      /* SDL_INIT_TIMER is not initialized while
	 it's not required for SDL_Delay and SDL_GetTicks according to the
	 code + it's not precised in the doc that it's required
       */
      if(SDL_Init(0) < 0)
        throw Exception("(1) SDL_Init : " + std::string(SDL_GetError()));
      
      /* No graphics mojo here, thank you */
      return;
    } else {
      if(SDL_Init(SDL_INIT_VIDEO) < 0)
        throw Exception("(2) SDL_Init : " + std::string(SDL_GetError()));
    }
    /* Set window title */
    SDL_WM_SetCaption(XMBuild::getVersionString(true).c_str(), XMBuild::getVersionString(true).c_str());

#if !defined(WIN32) && !defined(__APPLE__) && !defined(__amigaos4__) 
    SDL_Surface *v_icon = SDL_LoadBMP(GAMEDATADIR "/xmoto_icone_x.ico");
    if(v_icon != NULL) {
      SDL_SetColorKey(v_icon, SDL_SRCCOLORKEY,
		      SDL_MapRGB(v_icon->format, 236, 45, 211));
      SDL_WM_SetIcon(v_icon, NULL);
    }
#endif

    if(TTF_Init() < 0) {
      throw Exception("Initializing TTF failed: " + std::string(TTF_GetError()));
    }
    atexit(TTF_Quit);
  }

GameApp::~GameApp() {
  xmDatabase::destroy("main");

  if(drawLib != NULL) {
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
  
  m_pNotifyMsgBox=NULL;
  m_pInfoMsgBox=NULL;

  m_pWebConfEditor=NULL;
  m_pWebConfMsgBox=NULL;

  m_pSaveReplayMsgBox=NULL;
  m_pReplaysWindow=NULL;
  m_pLevelPacksWindow=NULL;
  m_pLevelPackViewer=NULL;  
  m_pGameInfoWindow=NULL;
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
  m_frameLate          = 0;

  // assume all focus at startup
  m_hasMouseFocus    = true;
  m_hasKeyboardFocus = true;
  m_isIconified      = false;

  m_standAloneServer = NULL;

  m_userConfig = new UserConfig();
}
   
  /*===========================================================================
  Screenshooting
  ===========================================================================*/
  void GameApp::gameScreenshot() {     
    Img *pShot = getDrawLib()->grabScreen();

    std::string v_ShotsDir;
    std::string v_ShotExtension;
    std::string v_destFile;
    int nShot=0;
    char v_val[5];

    v_ShotsDir = XMFS::getUserDir(FDT_DATA) + std::string("/Screenshots");
    XMFS::mkArborescenceDir(v_ShotsDir);
    v_ShotExtension = XMSession::instance()->screenshotFormat();
    
    /* User preference for format? must be either jpeg or png */
    if(v_ShotExtension != "jpeg" && v_ShotExtension != "jpg" && v_ShotExtension != "png") {
      LogWarning("unsupported screenshot format '%s', using png instead!", v_ShotExtension.c_str());
      v_ShotExtension = "png";
    }    

    do {
      nShot++;
      if(nShot > 9999) {
	LogWarning("Too many screenshots !");
	delete pShot;
	return;
      }

      snprintf(v_val, 5, "%04d", nShot);
      v_destFile = v_ShotsDir + "/screenshot" + std::string(v_val) + "." + v_ShotExtension;
    } while(XMFS::fileExists(FDT_DATA, v_destFile));
    try {
      pShot->saveFile(v_destFile.c_str());
    } catch(Exception &e) {
      LogError(std::string("Unable to save the screenshot: " + e.getMsg()).c_str());
    }

    delete pShot;
  }

  void GameApp::enableFps(bool bValue) {
    XMSession::instance()->setFps(XMSession::instance()->fps() == false);
    if(XMSession::instance()->fps()) {
      SysMessage::instance()->displayText(SYS_MSG_FPS_ENABLED);
    } else {
      SysMessage::instance()->displayText(SYS_MSG_FPS_DISABLED);
    }
  }

  void GameApp::enableWWW(bool bValue) {
    XMSession::instance()->setWWW(XMSession::instance()->www() == false);
    if(XMSession::instance()->www()) {
      SysMessage::instance()->displayText(SYS_MSG_WWW_ENABLED);
    } else {
      SysMessage::instance()->displayText(SYS_MSG_WWW_DISABLED);
    }
  }

  std::string GameApp::determineNextLevel(const std::string& i_id_level) {
    if(m_currentPlayingList == NULL) {
      return "";
    }

    for(unsigned int i=0;i<m_currentPlayingList->getEntries().size()-1;i++) {
      if((*((std::string*)m_currentPlayingList->getEntries()[i]->pvUser)) == i_id_level) {
	return *((std::string*)m_currentPlayingList->getEntries()[i+1]->pvUser);
      }
    }
    return *((std::string*)m_currentPlayingList->getEntries()[0]->pvUser);
  }
  
  bool GameApp::isThereANextLevel(const std::string& i_id_level) {
    return determineNextLevel(i_id_level) != "";
  }

  std::string GameApp::determinePreviousLevel(const std::string& i_id_level) {
    if(m_currentPlayingList == NULL) {
      return "";
    }

    for(unsigned int i=1;i<m_currentPlayingList->getEntries().size();i++) {
      if((*((std::string*)m_currentPlayingList->getEntries()[i]->pvUser)) == i_id_level) {
	return *((std::string*)m_currentPlayingList->getEntries()[i-1]->pvUser);
      }
    }
    return *((std::string*)m_currentPlayingList->getEntries()[m_currentPlayingList->getEntries().size()-1]->pvUser);
  }
  
  bool GameApp::isThereAPreviousLevel(const std::string& i_id_level) {
    return determinePreviousLevel(i_id_level) != "";
  } 

  std::string GameApp::getWorldRecord(unsigned int i_number, const std::string &LevelID) {  
    char **v_result;
    unsigned int nrow;
    std::string v_roomName;
    std::string v_id_profile;
    int       v_finishTime = 0;
    xmDatabase* v_pDb = xmDatabase::instance("main");

    v_result = v_pDb->readDB("SELECT a.name, b.id_profile, b.finishTime "
			    "FROM webrooms AS a LEFT OUTER JOIN webhighscores AS b "
			    "ON (a.id_room = b.id_room "
			    "AND b.id_level=\"" + xmDatabase::protectString(LevelID) + "\") "
			    "WHERE a.id_room=" + XMSession::instance()->idRoom(i_number) + ";",
			    nrow);
    if(nrow != 1) {
      /* should not happend */
      v_pDb->read_DB_free(v_result);
      return GAMETEXT_WORLDRECORDNA + std::string(": WR");
    }
    v_roomName = v_pDb->getResult(v_result, 3, 0, 0);
    if(v_pDb->getResult(v_result, 3, 0, 1) != NULL) {
      v_id_profile = v_pDb->getResult(v_result, 3, 0, 1);
      v_finishTime = atoi(v_pDb->getResult(v_result, 3, 0, 2));
    }
    v_pDb->read_DB_free(v_result);
    
    if(v_id_profile != "") {
      return formatTime(v_finishTime) + ": " + v_roomName +  std::string(" (") + v_id_profile + std::string(")");
    }
     
    return GAMETEXT_WORLDRECORDNA + std::string(": ") + v_roomName;
  }
  
  std::string GameApp::_getGhostReplayPath_bestOfThePlayer(std::string p_levelId, int &p_time) {
    char **v_result;
    unsigned int nrow;
    std::string res;
    xmDatabase* v_pDb = xmDatabase::instance("main");

    p_time = -1;

    v_result = v_pDb->readDB("SELECT name, finishTime FROM replays "
			    "WHERE id_profile=\"" + xmDatabase::protectString(XMSession::instance()->profile()) + "\" "
			    "AND   id_level=\""   + xmDatabase::protectString(p_levelId) + "\" "
			    "AND   isFinished=1 "
			    "ORDER BY finishTime LIMIT 1;",
			    nrow);    
    if(nrow == 0) {
      v_pDb->read_DB_free(v_result);
      return "";
    }

    res = std::string("Replays/") + v_pDb->getResult(v_result, 2, 0, 0) + std::string(".rpl");
    p_time = atoi(v_pDb->getResult(v_result, 2, 0, 1));

    v_pDb->read_DB_free(v_result);
    return res;
  }

std::string GameApp::_getGhostReplayPath_bestOfTheRoom(unsigned int i_number, std::string p_levelId, int &p_time)
{
  char **v_result;
  unsigned int nrow;
  std::string res;
  std::string v_replayName;
  std::string v_fileUrl;
  xmDatabase* v_pDb = xmDatabase::instance("main");

  v_result = v_pDb->readDB("SELECT fileUrl, finishTime FROM webhighscores "
			  "WHERE id_room=" + XMSession::instance()->idRoom(i_number) + " "
			  "AND id_level=\"" + xmDatabase::protectString(p_levelId) + "\";",
			  nrow);    
  if(nrow == 0) {
    p_time = -1;
    v_pDb->read_DB_free(v_result);
    return "";
  }

  v_fileUrl = v_pDb->getResult(v_result, 2, 0, 0);
  v_replayName = XMFS::getFileBaseName(v_fileUrl);
  p_time = atoi(v_pDb->getResult(v_result, 2, 0, 1));
  v_pDb->read_DB_free(v_result);

  /* search if the replay is already downloaded */
  if(v_pDb->replays_exists(v_replayName)) {
    return std::string("Replays/") + v_replayName + std::string(".rpl");
  } else {
    return "";
  }
}

  std::string GameApp::_getGhostReplayPath_bestOfLocal(std::string p_levelId, int &p_time) {
    char **v_result;
    unsigned int nrow;
    std::string res;
    xmDatabase* v_pDb = xmDatabase::instance("main");

    v_result = v_pDb->readDB("SELECT a.name, a.finishTime FROM replays AS a INNER JOIN stats_profiles AS b "
			    "ON a.id_profile = b.id_profile "
			    "WHERE a.id_level=\""   + xmDatabase::protectString(p_levelId) + "\" "
			    "AND   a.isFinished=1 "
			    "ORDER BY a.finishTime+0 LIMIT 1;",
			    nrow);    
    if(nrow == 0) {
      v_pDb->read_DB_free(v_result);
      return "";
    }

    res = std::string("Replays/") + v_pDb->getResult(v_result, 2, 0, 0) + std::string(".rpl");
    p_time = atoi(v_pDb->getResult(v_result, 2, 0, 1));
    v_pDb->read_DB_free(v_result);
    return res;
  }

  TColor GameApp::getColorFromPlayerNumber(int i_player) {
    // try to find nice colors for first player, then automatic
    switch(i_player) {

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
      return TColor((i_player*5)%255, (i_player*20)%255, (i_player*50)%255, 0);
    }

    return TColor(255, 255, 255, 0);
  }

  TColor GameApp::getUglyColorFromPlayerNumber(int i_player) {
    // try to find nice colors for first player, then automatic
    Color v_color;

    switch(i_player) {
      
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
      return TColor((i_player*5)%255, (i_player*20)%255, (i_player*50)%255, 0);
    }
  }

  void GameApp::switchUglyMode(bool bUgly) {
    XMSession::instance()->setUgly(bUgly);
    
    displayCursor(bUgly);
  }

void GameApp::displayCursor(bool display)
{
  if(display == false) {
    SDL_ShowCursor(SDL_DISABLE);        
  } else {
    SDL_ShowCursor(SDL_ENABLE);
  }
}

  void GameApp::switchTestThemeMode(bool mode) {
    XMSession::instance()->setTestTheme(mode);
  }

  void GameApp::switchUglyOverMode(bool mode) {
    XMSession::instance()->setUglyOver(mode);
  }

  void GameApp::reloadTheme() {
    try {
      Theme::instance()->load(FDT_DATA, xmDatabase::instance("main")->themes_getFileName(XMSession::instance()->theme())/*HIER: BIKETHEME*/);
    } catch(Exception &e) {
      /* unable to load the theme, load the default one */
      Theme::instance()->load(FDT_DATA, xmDatabase::instance("main")->themes_getFileName(DEFAULT_THEME)); // no XMDefault::DefaultTheme, the DEFAULT_THEME one is included into xmoto files
    }
  }

  void GameApp::initReplaysFromDir(xmDatabase* threadDb,
				   XMotoLoadReplaysInterface* pLoadReplaysInterface) {
    std::vector<std::string> ReplayFiles;

    ReplayFiles = XMFS::findPhysFiles(FDT_DATA, "Replays/*.rpl");
    threadDb->replays_add_begin();

    for(unsigned int i=0; i<ReplayFiles.size(); i++) {
      try {
	if(XMFS::getFileBaseName(ReplayFiles[i]) == "Latest") {
	  continue;
	}
	addReplay(ReplayFiles[i], threadDb, false);
	if(pLoadReplaysInterface != NULL){
	  pLoadReplaysInterface->loadReplayHook(ReplayFiles[i], (int)((i*100)/((float)ReplayFiles.size())));
	}

      } catch(Exception &e) {
	// ok, forget this replay
      }
    }
    threadDb->replays_add_end();
  }

  void GameApp::addReplay(const std::string& i_file, xmDatabase* pDb, bool sendMessage) {
    ReplayInfo* rplInfos;
    
    rplInfos = Replay::getReplayInfos(XMFS::getFileBaseName(i_file));
    if(rplInfos == NULL) {
      throw Exception("Unable to extract data from replay file");
    }

    try {
      pDb->replays_add(rplInfos->Level,
		       rplInfos->Name,
		       rplInfos->Player,
		       rplInfos->IsFinished,
		       rplInfos->finishTime);
      if(sendMessage == true)
	StateManager::instance()->sendAsynchronousMessage("REPLAYS_UPDATED");

    } catch(Exception &e2) {
      delete rplInfos;
      throw e2;
    }
  }

  void GameApp::setSpecificReplay(const std::string& i_replay) {
    m_PlaySpecificReplay = i_replay;
  }
  
  void GameApp::setSpecificLevelId(const std::string& i_levelID) {
    m_PlaySpecificLevelId = i_levelID;
  }

  void GameApp::setSpecificLevelFile(const std::string& i_leveFile) {
    m_PlaySpecificLevelFile = i_leveFile;
  }

void GameApp::addGhosts(Scene* i_motogame, Theme* i_theme) {
  std::string v_replay_MYBEST;
  std::string v_replay_THEBEST;
  std::string v_replay_BESTOFROOM[ROOMS_NB_MAX];
  std::string v_replay_MYBEST_tmp;
  int v_finishTime;
  int v_player_finishTime;
  bool v_exists;

  LogDebug("addGhosts stategy:");

  v_replay_MYBEST_tmp = _getGhostReplayPath_bestOfThePlayer(i_motogame->getLevelSrc()->Id(), v_player_finishTime);

  /* first, add the best of the room -- because if mybest or thebest = bestofroom, i prefer to see writen bestofroom */
  for(unsigned int i=0; i<XMSession::instance()->nbRoomsEnabled(); i++) {
    if( (XMSession::instance()->ghostStrategy_BESTOFREFROOM()    && i==0) ||
	(XMSession::instance()->ghostStrategy_BESTOFOTHERROOMS() && i!=0) ){

      LogDebug("Choosing ghost for room %i", i);

      v_replay_BESTOFROOM[i] = _getGhostReplayPath_bestOfTheRoom(i, i_motogame->getLevelSrc()->Id(), v_finishTime);
      LogDebug("the room ghost: %s", v_replay_BESTOFROOM[i].c_str());

      /* add MYBEST if MYBEST if better the BESTOF the other ROOM */
      if(v_player_finishTime > 0 && (v_finishTime < 0 || v_player_finishTime < v_finishTime)) {
	v_replay_BESTOFROOM[i] = v_replay_MYBEST_tmp;
	LogDebug("my best time is %i ; room one is %i", v_player_finishTime, v_finishTime);
	LogDebug("my best is better than the one of the room => choose it");
      }
      
      v_exists = false;
      for(unsigned int j=0; j<i; j++) {
      	if(v_replay_BESTOFROOM[i] == v_replay_BESTOFROOM[j]) {
      	  v_exists = true;
	  LogDebug("the ghost is already set by room %i", j);
      	}
      }

      if(v_replay_BESTOFROOM[i] != "" && v_exists == false) {
	LogInfo("add ghost %s", v_replay_BESTOFROOM[i].c_str());

        i_motogame->addGhostFromFile(v_replay_BESTOFROOM[i],
				     xmDatabase::instance("main")->webrooms_getName(XMSession::instance()->idRoom(i)), i==0,
				     Theme::instance(), Theme::instance()->getGhostTheme(),
				     TColor(255,255,255,0),
				     TColor(GET_RED(i_theme->getGhostTheme()->getUglyRiderColor()),
					    GET_GREEN(i_theme->getGhostTheme()->getUglyRiderColor()),
					    GET_BLUE(i_theme->getGhostTheme()->getUglyRiderColor()),
					    0)
			   );
      }
    }
  }

  /* second, add your best */
  if(XMSession::instance()->ghostStrategy_MYBEST()) {
    LogDebug("Choosing ghost MYBEST");
    v_replay_MYBEST = _getGhostReplayPath_bestOfThePlayer(i_motogame->getLevelSrc()->Id(), v_finishTime);
    LogDebug("MYBEST ghost is %s", v_replay_MYBEST.c_str());
    if(v_replay_MYBEST != "") {
      v_exists = false;
      for(unsigned int i=0; i<XMSession::instance()->nbRoomsEnabled(); i++) {
	if(v_replay_MYBEST == v_replay_BESTOFROOM[i]) {
	  LogDebug("the ghost is already set by room %i", i);
	  v_exists = true;
	}
      }

      if(v_exists == false) {
	LogDebug("add ghost %s", v_replay_MYBEST.c_str());
	i_motogame->addGhostFromFile(v_replay_MYBEST, GAMETEXT_GHOST_BEST, true,
				     i_theme, i_theme->getGhostTheme(),
				     TColor(85,255,255,0),
				     TColor(GET_RED(i_theme->getGhostTheme()->getUglyRiderColor()),
					    GET_GREEN(i_theme->getGhostTheme()->getUglyRiderColor()),
					    GET_BLUE(i_theme->getGhostTheme()->getUglyRiderColor()),
					    0)
				     );
      }
    }
  }

  /* third, the best locally */
  if(XMSession::instance()->ghostStrategy_THEBEST()) {
    LogDebug("Choosing ghost LOCAL BEST");
    v_replay_THEBEST = _getGhostReplayPath_bestOfLocal(i_motogame->getLevelSrc()->Id(), v_finishTime);
    if(v_replay_THEBEST != "") {

      v_exists = false;
      for(unsigned int i=0; i<XMSession::instance()->nbRoomsEnabled(); i++) {
	if(v_replay_THEBEST == v_replay_BESTOFROOM[i]) {
	  LogDebug("the ghost is already set by room %i", i);
	  v_exists = true;
	}
      }

      if(v_replay_THEBEST != v_replay_MYBEST && v_exists == false) { /* don't add two times the same ghost */
	LogDebug("add ghost %s", v_replay_THEBEST.c_str());
	i_motogame->addGhostFromFile(v_replay_THEBEST, GAMETEXT_GHOST_LOCAL, false,
				     i_theme, i_theme->getGhostTheme(),
				     TColor(255,200,140,0),
				     TColor(GET_RED(i_theme->getGhostTheme()->getUglyRiderColor()),
					    GET_GREEN(i_theme->getGhostTheme()->getUglyRiderColor()),
					    GET_BLUE(i_theme->getGhostTheme()->getUglyRiderColor()),
					    0)
				     );
      }
    }
  }

  LogDebug("addGhosts stategy finished");
}

void GameApp::addLevelToFavorite(const std::string& i_levelId) {
  LevelsManager::instance()->addToFavorite(XMSession::instance()->profile(), i_levelId, xmDatabase::instance("main"));
}

void GameApp::switchLevelToFavorite(const std::string& i_levelId, bool v_displayMessage) {
  if(LevelsManager::instance()->isInFavorite(XMSession::instance()->profile(), i_levelId, xmDatabase::instance("main"))) {
    LevelsManager::instance()->delFromFavorite(XMSession::instance()->profile(), i_levelId, xmDatabase::instance("main"));
    if(v_displayMessage) {
      SysMessage::instance()->displayText(GAMETEXT_LEVEL_DELETED_FROM_FAVORITE);
    }
  } else {
    LevelsManager::instance()->addToFavorite(XMSession::instance()->profile(), i_levelId, xmDatabase::instance("main"));
    if(v_displayMessage) {
      SysMessage::instance()->displayText(GAMETEXT_LEVEL_ADDED_TO_FAVORITE);
    }
  }
}

void GameApp::switchLevelToBlacklist(const std::string& i_levelId, bool v_displayMessage) {
  if(LevelsManager::instance()->isInBlacklist(XMSession::instance()->profile(), i_levelId, xmDatabase::instance("main"))) {
    LevelsManager::instance()->delFromBlacklist(XMSession::instance()->profile(), i_levelId, xmDatabase::instance("main"));
    if(v_displayMessage) {
      SysMessage::instance()->displayText(GAMETEXT_LEVEL_DELETED_FROM_BLACKLIST);
    }
  } else {
    LevelsManager::instance()->addToBlacklist(XMSession::instance()->profile(), i_levelId, xmDatabase::instance("main"));
    if(v_displayMessage) {
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

void GameApp::playMenuMusic(const std::string& i_music) {
  if(XMSession::instance()->enableAudio() && XMSession::instance()->enableMenuMusic()) {
    playMusic(i_music);
  } else {
    playMusic("");
  }
}

void GameApp::playGameMusic(const std::string& i_music) {
  if(XMSession::instance()->enableAudio() && XMSession::instance()->enableGameMusic()) {
    playMusic(i_music);
  } else {
    playMusic("");
  }
}

void GameApp::playMusic(const std::string& i_music) {
  LogDebug("Playing '%s'\n", i_music.c_str());

  if(i_music != m_playingMusic) {
    try {
      if(i_music == "") {
	m_playingMusic = "";
	Sound::stopMusic();
      } else {
	m_playingMusic = i_music;
	Sound::playMusic(Theme::instance()->getMusic(i_music)->FilePath());
      }
    } catch(Exception &e) {
      LogWarning("PlayMusic(%s) failed", i_music.c_str());
      Sound::stopMusic();
    }
  }
}

void GameApp::toogleEnableMusic() {
  m_playingMusic = ""; // tell the game the last music played is "" otherwise, it doesn't know when music is disable then enable back
  XMSession::instance()->setEnableAudio(! XMSession::instance()->enableAudio());
  Sound::setActiv(XMSession::instance()->enableAudio());

  if(XMSession::instance()->enableAudio()) {
    SysMessage::instance()->displayText(SYS_MSG_AUDIO_ENABLED);
  } else {
    SysMessage::instance()->displayText(SYS_MSG_AUDIO_DISABLED);
  }
  StateManager::instance()->sendAsynchronousMessage("ENABLEAUDIO_CHANGED");
}

std::string GameApp::getWebRoomURL(unsigned int i_number, xmDatabase* pDb) {
  char **v_result;
  unsigned int nrow;
  std::string v_url;

  v_result = pDb->readDB("SELECT highscoresUrl FROM webrooms WHERE id_room=" + XMSession::instance()->idRoom(i_number) + ";", nrow);
  if(nrow != 1) {
    pDb->read_DB_free(v_result);
    return DEFAULT_WEBROOMS_URL;
  }
  v_url = pDb->getResult(v_result, 1, 0, 0);
  pDb->read_DB_free(v_result);  

  return v_url;
}

std::string GameApp::getWebRoomName(unsigned int i_number, xmDatabase* pDb) {
  char **v_result;
  unsigned int nrow;
  std::string v_name;

  /* set the room name ; set to WR if it cannot be determined */
  v_name = "WR";

  v_result = pDb->readDB("SELECT name FROM webrooms WHERE id_room=" + XMSession::instance()->idRoom(i_number) + ";", nrow);
  if(nrow == 1) {
    v_name = pDb->getResult(v_result, 1, 0, 0);
  }
  pDb->read_DB_free(v_result);

  return v_name;
}

bool GameApp::getHighscoreInfos(unsigned int i_number, const std::string& i_id_level, std::string* o_id_profile, std::string* o_url, bool* o_isAccessible) {
  char **v_result;
  unsigned int nrow;
  xmDatabase *pDb = xmDatabase::instance("main");

  v_result = pDb->readDB("SELECT id_profile, fileUrl FROM webhighscores WHERE id_level=\"" + 
			  xmDatabase::protectString(i_id_level) + "\" AND id_room=" + XMSession::instance()->idRoom(0) + ";",
			  nrow);
  if(nrow == 0) {
    pDb->read_DB_free(v_result);
    return false;
  }

  *o_id_profile = pDb->getResult(v_result, 2, 0, 0);
  *o_url        = pDb->getResult(v_result, 2, 0, 1);
  pDb->read_DB_free(v_result);

  /* search if the replay is already downloaded */
  if(pDb->replays_exists(XMFS::getFileBaseName(*o_url))) {
    *o_isAccessible = true;
  } else {
    *o_isAccessible = XMSession::instance()->www();
  }

  return true;
}

void GameApp::loadLevelHook(std::string i_level, int i_percentage)
{
  std::ostringstream v_percentage;
  v_percentage << i_percentage;
  v_percentage << "%";
  _UpdateLoadingScreen(std::string(GAMETEXT_LOAD_LEVEL_HOOK) + std::string("\n") + v_percentage.str() + std::string(" ") + i_level);

  /* pump events to so that windows don't think the appli is crashed */
  SDL_PumpEvents();
}

void GameApp::updatingDatabase(std::string i_message) {
  _UpdateLoadingScreen(i_message);

  /* pump events to so that windows don't think the appli is crashed */
  SDL_PumpEvents();
}

void GameApp::drawFrame(void) {
  Sound::update();
  StateManager::instance()->update();
  StateManager::instance()->render(); 
}

NetServer* GameApp::standAloneServer() {
  return m_standAloneServer;
}

void GameApp::initBikesFromDir(xmDatabase* i_db) {
#define BIKES_DIR "Bikes"
  std::vector<std::string> v_bikesFiles = XMFS::findPhysFiles(FDT_DATA, std::string(BIKES_DIR)
							       + std::string("/*.xml"), true);
  std::string v_name;

  i_db->bikes_add_begin();
  for(unsigned int i=0; i<v_bikesFiles.size(); i++) {
    try {
      v_name = getThemeNameFromFile(v_bikesFiles[i], "xmoto_bike");
      if(i_db->bikes_exists(v_name) == false) {
	i_db->bikes_add(v_name, v_bikesFiles[i]);
      } else {
	LogWarning(std::string("Bike " + v_name + " is present several times").c_str());
      }
    } catch(Exception &e) {
      /* anyway, give up this bike */
    }
  }
  i_db->bikes_add_end();		  
}


void GameApp::initPhysicsFromDir() {
#define PHYSICS_DIR "Physics"
  std::vector<std::string> v_physicsFiles = XMFS::findPhysFiles(FDT_DATA, std::string(PHYSICS_DIR)
							       + std::string("/*.xml"), true);
  std::string v_name;

  for(unsigned int i=0; i<v_physicsFiles.size(); i++) {
    try {
      v_name = getThemeNameFromFile(v_physicsFiles[i], "xmoto_physics");
      m_availablePhysics.push_back(v_physicsFiles[i]);
      LogInfo("found physics: %s", v_physicsFiles[i].c_str() );
    } catch(Exception &e) {
      /* anyway, give up this theme */
    }
  }
}


std::string GameApp::getThemeNameFromFile(std::string p_themeFile, std::string i_element) {
  XMLDocument v_ThemeXml;
  TiXmlDocument *v_ThemeXmlData;
  TiXmlElement *v_ThemeXmlDataElement;
  const char *pc;
  std::string m_name;

  /* open the file */
  v_ThemeXml.readFromFile(FDT_DATA, p_themeFile);   
  v_ThemeXmlData = v_ThemeXml.getLowLevelAccess();
  
  if(v_ThemeXmlData == NULL) {
    throw Exception("error : unable to analyse xml theme file");
  }
  
  /* read the theme name */
  v_ThemeXmlDataElement = v_ThemeXmlData->FirstChildElement(i_element.c_str()); // "xmoto_physics");
  if(v_ThemeXmlDataElement != NULL) {
    pc = v_ThemeXmlDataElement->Attribute("name");
    m_name = pc;
  }
  
  if(m_name == "") {
    throw Exception("error : the theme has no name !");
  }
  
  return m_name;
}

std::vector<std::string> GameApp::getAvailablePhysics() {
  return m_availablePhysics;
}

std::string GameApp::getPhysicsFromBike() {
  char **v_result;
  unsigned int nrow;
  XMLDocument v_bikeXml;
  TiXmlDocument *v_bikeXmlData;
  TiXmlElement *v_bikeXmlDataElement;
  const char *pc;
  std::string v_name;

  /* in physics dev mode, make a long way short */
  if(XMSession::instance()->bikesOverride()) {
    return XMSession::instance()->bikePhysics();
  }

  /* get file name from current bike */
  
  std::string v_id_bike;
  std::string v_bikeFile;
  v_result = xmDatabase::instance("main")->readDB("SELECT filepath FROM bikes WHERE id_bike=\"" + XMSession::instance()->bike() + "\";", nrow);
  v_bikeFile = xmDatabase::instance("main")->getResult(v_result, 1, 0, 0);

  /* open the file */
  v_bikeXml.readFromFile(FDT_DATA, v_bikeFile);   
  v_bikeXmlData = v_bikeXml.getLowLevelAccess();
  LogInfo("Datei:%s",v_bikeFile.c_str());
  if(v_bikeXmlData == NULL) {
    throw Exception("error : unable analyse xml bike file");
  }
  
  /* read the theme name */
  v_bikeXmlDataElement = v_bikeXmlData->FirstChildElement("physics"); // "xmoto_physics");
  if(v_bikeXmlDataElement != NULL) {
    pc = v_bikeXmlDataElement->Attribute("name");
    v_name = pc;
  }
  if(v_name == "") {
    throw Exception("error : the physics defined in the bike theme  cant be found !");
  }
  
  std::string v_themeFile = "none";
  for(unsigned int i=0; i<m_availablePhysics.size(); i++) {
    if(getThemeNameFromFile(m_availablePhysics[i], "xmoto_physics").c_str() == v_name) {
      v_themeFile= m_availablePhysics[i];
      LogInfo("We get: %s",m_availablePhysics[i].c_str());
    }
  }
  
  return v_themeFile;
}