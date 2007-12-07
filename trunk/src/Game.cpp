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
#include "XMSession.h"
#include "drawlib/DrawLib.h"
#include "Image.h"
#include "SysMessage.h"
#include "GameText.h"
#include "Credits.h"
#include "gui/specific/GUIXMoto.h"
#include "xmscene/Camera.h"
#include "xmscene/Entity.h"

#include <curl/curl.h>
#include <iomanip.h>
#include "states/StateManager.h"
#include "states/StatePause.h"
#include "states/StateDeadMenu.h"
#include "states/StatePlaying.h"
#include "states/StatePreplaying.h"
#include "states/StateMessageBox.h"
#include "XMotoLoadReplaysInterface.h"

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
      sprintf(cBuf,"%d-%02d-%02d %02d:%02d:%02d",
              pTime->tm_year+1900, pTime->tm_mon+1, pTime->tm_mday,
	      pTime->tm_hour, pTime->tm_min, pTime->tm_sec);                    
    }    
    return cBuf;
  }
  
  double GameApp::getXMTime(void) {
    return SDL_GetTicks() / 1000.0f;
  }

  int GameApp::getXMTimeInt(void){
    return SDL_GetTicks();
  }

  std::string GameApp::formatTime(float fSecs) {
    char cBuf[256];
    int nM, nS, nH;
    float nHres;

    nM = (int)(fSecs/60.0);
    nS = (int)(fSecs - ((float)nM)*60.0);
    nHres = (fSecs - ((float)nM)*60.0 - ((float)nS));
    nH = (int)(nHres * 100.0);

    /* hum, case, in which 0.9800 * 100.0 => 0.9799999*/
    if(((int)(nHres * 100.0)) < ((int)((nHres * 100.0) + 0.001))) {
      nH = ((int)((nHres * 100.0) + 0.001));
      nH %= 100;
    }

    sprintf(cBuf,"%02d:%02d:%02d", nM, nS, nH);
    return cBuf;
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
      if(SDL_Init(SDL_INIT_TIMER) < 0)
        throw Exception("(1) SDL_Init : " + std::string(SDL_GetError()));
      
      /* No graphics mojo here, thank you */
      return;
    } else {
      if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0)
        throw Exception("(2) SDL_Init : " + std::string(SDL_GetError()));
    }
    /* Set window title */
    SDL_WM_SetCaption(XMBuild::getVersionString(true).c_str(), XMBuild::getVersionString(true).c_str());

#if !defined(WIN32) && !defined(__APPLE__) 
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

  /*===========================================================================
  Uninit 
  ===========================================================================*/
  void GameApp::_Uninit(void) {
    /* Tell user app to turn off */
    userShutdown();

    if(m_xmsession->useGraphics()) {
      /* Uninit drawing library */
      drawLib->unInit();
    }
    
    Logger::uninit();
    
    /* Shutdown SDL */
    SDL_Quit();
  }
  
GameApp::~GameApp() {
  delete m_stateManager;

  if(m_db != NULL) {
    delete m_db;
  }

  if(drawLib != NULL) {
    delete drawLib;
  }

  delete m_xmsession;

  StateManager::cleanStates();
}

GameApp::GameApp() {
  m_bQuit = false;
  drawLib = NULL;
  m_Renderer = NULL;
  
  m_xmsession = new XMSession();
  m_sysMsg = NULL;

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
  m_pJustPlayReplay = NULL;
  m_updateAutomaticallyLevels = false;

  m_pWebHighscores = NULL;
  m_pWebLevels = NULL;
  m_fDownloadTaskProgressLast = 0;
  m_bWebHighscoresUpdatedThisSession = false;
  m_bWebLevelsToDownload = false;
  
  m_MotoGame.setHooks(&m_MotoGameHooks);
  m_MotoGameHooks.setGameApps(this, &m_MotoGame);
  
  m_currentPlayingList = NULL;

  m_db = NULL;

  m_stateManager       = new StateManager(this);
  m_lastFrameTimeStamp = -1;
  m_frameLate          = 0;
}
    
  std::string GameApp::splitText(const std::string &str, int p_breakLineLength) {
    std::string v_res = "";
    char c[2] = {' ', '\0'};    
    int lineLength = 0;

    for(int i=0; i<str.length(); i++) {
      if((lineLength > p_breakLineLength && str[i] == ' ') ||
        str[i] == '\n') {
        c[0] = '\n';
        v_res.append(c);
        lineLength = 0;
      } else {
        c[0] = str[i];
        v_res.append(c);
        lineLength++;
      }
    }
    return v_res;
  }
    
  /*===========================================================================
  Screenshooting
  ===========================================================================*/
  void GameApp::gameScreenshot() {
    //    Img *pShot = getDrawLib()->grabScreen(2);      
    Img *pShot = getDrawLib()->grabScreen();
    FileHandle *pfh;

    std::string v_ShotsDir;
    std::string v_ShotExtension;
    std::string v_destFile;
    int nShot=0;
    char v_val[5];

    v_ShotsDir = FS::getUserDir() + std::string("/Screenshots");
    FS::mkArborescenceDir(v_ShotsDir);
    v_ShotExtension = m_xmsession->screenshotFormat();
    
    /* User preference for format? must be either jpeg or png */
    if(v_ShotExtension != "jpeg" && v_ShotExtension != "jpg" && v_ShotExtension != "png") {
      Logger::Log("** Warning ** : unsupported screenshot format '%s', using png instead!", v_ShotExtension.c_str());
      v_ShotExtension = "png";
    }    

    do {
      nShot++;
      if(nShot > 9999) {
	Logger::Log("Too many screenshots !");
	delete pShot;
	return;
      }

      snprintf(v_val, 5, "%04d", nShot);
      v_destFile = v_ShotsDir + "/screenshot" + std::string(v_val) + "." + v_ShotExtension;
    } while(FS::fileExists(v_destFile));
    try {
      pShot->saveFile(v_destFile.c_str());
    } catch(Exception &e) {
      Logger::Log(std::string("Unable to save the screenshot: " + e.getMsg()).c_str());
    }

    delete pShot;
  }

  void GameApp::enableFps(bool bValue) {
    m_xmsession->setFps(m_xmsession->fps() == false);
    if(m_xmsession->fps()) {
      m_sysMsg->displayText(SYS_MSG_FPS_ENABLED);
    } else {
      m_sysMsg->displayText(SYS_MSG_FPS_DISABLED);
    }
  }

  void GameApp::enableWWW(bool bValue) {
    m_xmsession->setWWW(m_xmsession->www() == false);
    if(m_xmsession->www()) {
      m_sysMsg->displayText(SYS_MSG_WWW_ENABLED);
    } else {
      m_sysMsg->displayText(SYS_MSG_WWW_DISABLED);
    }
  }

  /*===========================================================================
  Key down event
  ===========================================================================*/
void GameApp::keyDown(int nKey, SDLMod mod, int nChar) {
  m_stateManager->keyDown(nKey, mod, nChar);
}

/*===========================================================================
  Key up event
  ===========================================================================*/
  void GameApp::keyUp(int nKey, SDLMod mod) {
    m_stateManager->keyUp(nKey, mod);
  }

  /*===========================================================================
  Mouse events
  ===========================================================================*/
  void GameApp::mouseDoubleClick(int nButton) {
    m_stateManager->mouseDoubleClick(nButton);
  }

  void GameApp::mouseDown(int nButton) {
    m_stateManager->mouseDown(nButton);
  }

  void GameApp::mouseUp(int nButton) {
    m_stateManager->mouseUp(nButton);
  }

  /*===========================================================================
  Save a replay
  ===========================================================================*/
  void GameApp::saveReplay(const std::string &Name) {
    /* This is simply a job of copying the Replays/Latest.rpl file into 
       Replays/Name.rpl */
    std::string RealName = Name;
    
    /* Strip illegal characters from name */
    int i=0;
    while(1) {
      if(i >= RealName.length()) break;
      
      if((RealName[i] >= 'a' && RealName[i] <= 'z') ||
         (RealName[i] >= 'A' && RealName[i] <= 'Z') ||
         (RealName[i] >= '0' && RealName[i] <= '9') ||
         RealName[i]=='!' || RealName[i]=='@' || RealName[i]=='#' || RealName[i]=='&' ||
         RealName[i]=='(' || RealName[i]==')' || RealName[i]=='-' || RealName[i]=='_' ||
         RealName[i]==' ' || RealName[i]=='.' || RealName[i]==',' || RealName[i]=='*') {
        /* This is ok */
        i++;
      }
      else {
        /* Not ok */
        RealName.erase(RealName.begin() + i);
      }            
    }

    /* Try saving */
    std::string v_outputfile;
    if(!FS::copyFile("Replays/Latest.rpl",
         std::string("Replays/") + RealName + std::string(".rpl"),
         v_outputfile)) {
      Logger::Log("** Warning ** : Failed to save replay: %s",Name.c_str());
      //notifyMsg(GAMETEXT_FAILEDTOSAVEREPLAY);
    } else {
      /* Update replay list to reflect changes */
      addReplay(v_outputfile);
      getStateManager()->sendAsynchronousMessage("REPLAYS_UPDATED");
    }
  }

  std::string GameApp::determineNextLevel(const std::string& i_id_level) {
    if(m_currentPlayingList == NULL) {
      return "";
    }

    for(int i=0;i<m_currentPlayingList->getEntries().size()-1;i++) {
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

    for(int i=1;i<m_currentPlayingList->getEntries().size();i++) {
      if((*((std::string*)m_currentPlayingList->getEntries()[i]->pvUser)) == i_id_level) {
	return *((std::string*)m_currentPlayingList->getEntries()[i-1]->pvUser);
      }
    }
    return *((std::string*)m_currentPlayingList->getEntries()[m_currentPlayingList->getEntries().size()-1]->pvUser);
  }
  
  bool GameApp::isThereAPreviousLevel(const std::string& i_id_level) {
    return determinePreviousLevel(i_id_level) != "";
  } 

  std::string GameApp::getWorldRecord(const std::string &LevelID) {  
    char **v_result;
    unsigned int nrow;
    std::string v_roomName;
    std::string v_id_profile;
    float       v_finishTime;

    v_result = m_db->readDB("SELECT a.name, b.id_profile, b.finishTime "
			    "FROM webrooms AS a LEFT OUTER JOIN webhighscores AS b "
			    "ON (a.id_room = b.id_room "
			    "AND b.id_level=\"" + xmDatabase::protectString(LevelID) + "\") "
			    "WHERE a.id_room=" + m_xmsession->idRoom() + ";",
			    nrow);
    if(nrow != 1) {
      /* should not happend */
      m_db->read_DB_free(v_result);
      return std::string("WR: ") + GAMETEXT_WORLDRECORDNA;
    }
    v_roomName = m_db->getResult(v_result, 3, 0, 0);
    if(m_db->getResult(v_result, 3, 0, 1) != NULL) {
      v_id_profile = m_db->getResult(v_result, 3, 0, 1);
      v_finishTime = atof(m_db->getResult(v_result, 3, 0, 2));
    }
    m_db->read_DB_free(v_result);
    
    if(v_id_profile != "") {
      return v_roomName + ": " + GameApp::formatTime(v_finishTime) + std::string(" (") + v_id_profile + std::string(")");
    }
     
    return v_roomName + ": " + GAMETEXT_WORLDRECORDNA;
  }
  
  std::string GameApp::_getGhostReplayPath_bestOfThePlayer(std::string p_levelId, float &p_time) {
    char **v_result;
    unsigned int nrow;
    std::string res;

    p_time = -1.0;

    v_result = m_db->readDB("SELECT name, finishTime FROM replays "
			    "WHERE id_profile=\"" + xmDatabase::protectString(m_xmsession->profile()) + "\" "
			    "AND   id_level=\""   + xmDatabase::protectString(p_levelId) + "\" "
			    "AND   isFinished=1 "
			    "ORDER BY finishTime LIMIT 1;",
			    nrow);    
    if(nrow == 0) {
      m_db->read_DB_free(v_result);
      return "";
    }

    res = std::string("Replays/") + m_db->getResult(v_result, 2, 0, 0) + std::string(".rpl");
    p_time = atof(m_db->getResult(v_result, 2, 0, 1));

    m_db->read_DB_free(v_result);
    return res;
  }

std::string GameApp::_getGhostReplayPath_bestOfTheRoom(std::string p_levelId, float &p_time)
{
  char **v_result;
  unsigned int nrow;
  std::string res;
  std::string v_replayName;
  std::string v_fileUrl;

  v_result = m_db->readDB("SELECT fileUrl, finishTime FROM webhighscores "
			  "WHERE id_room=" + m_xmsession->idRoom() + " "
			  "AND id_level=\"" + xmDatabase::protectString(p_levelId) + "\";",
			  nrow);    
  if(nrow == 0) {
    p_time = -1.0;
    m_db->read_DB_free(v_result);
    return "";
  }

  v_fileUrl = m_db->getResult(v_result, 2, 0, 0);
  v_replayName = FS::getFileBaseName(v_fileUrl);
  p_time = atof(m_db->getResult(v_result, 2, 0, 1));
  m_db->read_DB_free(v_result);

  /* search if the replay is already downloaded */
  if(m_db->replays_exists(v_replayName)) {
    return std::string("Replays/") + v_replayName + std::string(".rpl");
  } else {
    return "";
  }
}

  std::string GameApp::_getGhostReplayPath_bestOfLocal(std::string p_levelId, float &p_time) {
    char **v_result;
    unsigned int nrow;
    std::string res;

    v_result = m_db->readDB("SELECT a.name, a.finishTime FROM replays AS a INNER JOIN stats_profiles AS b "
			    "ON a.id_profile = b.id_profile "
			    "WHERE a.id_level=\""   + xmDatabase::protectString(p_levelId) + "\" "
			    "AND   a.isFinished=1 "
			    "ORDER BY a.finishTime LIMIT 1;",
			    nrow);    
    if(nrow == 0) {
      m_db->read_DB_free(v_result);
      return "";
    }

    res = std::string("Replays/") + m_db->getResult(v_result, 2, 0, 0) + std::string(".rpl");
    p_time = atof(m_db->getResult(v_result, 2, 0, 1));
    m_db->read_DB_free(v_result);
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
      v_color = m_theme.getPlayerTheme()->getUglyRiderColor();
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
    m_xmsession->setUgly(bUgly);
    if(bUgly == false) {
      SDL_ShowCursor(SDL_DISABLE);        
    } else {
      SDL_ShowCursor(SDL_ENABLE);
    }
  }

  void GameApp::switchTestThemeMode(bool mode) {
    m_xmsession->setTestTheme(mode);
  }

  void GameApp::switchUglyOverMode(bool mode) {
    m_xmsession->setUglyOver(mode);
  }

  void GameApp::initCameras(int nbPlayer) {
    int width  = drawLib->getDispWidth();
    int height = drawLib->getDispHeight();

    switch(nbPlayer){
    default:
    case 1:
      m_MotoGame.addCamera(Vector2i(0,0),
			   Vector2i(width, height));
      break;
    case 2:
      m_MotoGame.addCamera(Vector2i(0,height/2),
			   Vector2i(width, height));
      m_MotoGame.addCamera(Vector2i(0,0),
			   Vector2i(width, height/2));
      break;
    case 3:
    case 4:
      m_MotoGame.addCamera(Vector2i(0,height/2),
			   Vector2i(width/2, height));
      m_MotoGame.addCamera(Vector2i(width/2,height/2),
			   Vector2i(width, height));
      m_MotoGame.addCamera(Vector2i(0,0),
			   Vector2i(width/2, height/2));
      m_MotoGame.addCamera(Vector2i(width/2,0),
			   Vector2i(width, height/2));
      break;
    }
    // the autozoom camera is a special one in multi player
    if(nbPlayer > 1){
      m_MotoGame.addCamera(Vector2i(0,0),
			   Vector2i(width, height));
    }
    // current cam is autozoom one
    m_MotoGame.setAutoZoomCamera();
  }

  void GameApp::reloadTheme() {
    try {
      m_theme.load(m_db->themes_getFileName(m_xmsession->theme()));
    } catch(Exception &e) {
      /* unable to load the theme, load the default one */
      m_theme.load(m_db->themes_getFileName(THEME_DEFAULT_THEMENAME));
    }
  }

  void XMMotoGameHooks::OnTakeEntity() {
    /* Play yummy-yummy sound */
    if(m_GameApp != NULL) {
      try {
	Sound::playSampleByName(m_GameApp->getTheme()->getSound
				(m_MotoGame->getLevelSrc()->SoundForPickUpStrawberry())->FilePath());
      } catch(Exception &e) {
      }
    }
  }
  
  void XMMotoGameHooks::setGameApps(GameApp *i_GameApp, MotoGame *i_MotoGame) {
    m_GameApp = i_GameApp;
    m_MotoGame = i_MotoGame;
  }

  XMMotoGameHooks::XMMotoGameHooks() {
    m_GameApp = NULL;
  }
  
  XMMotoGameHooks::~XMMotoGameHooks() {
  }

  void GameApp::TeleportationCheatTo(int i_player, Vector2f i_position) {
    m_MotoGame.setPlayerPosition(i_player, i_position.x, i_position.y, true);
    m_MotoGame.getCamera()->initCamera();
    m_MotoGame.addPenalityTime(900); /* 15 min of penality for that ! */
  }

  void GameApp::initReplaysFromDir(xmDatabase* threadDb,
				   XMotoLoadReplaysInterface* pLoadReplaysInterface) {
    ReplayInfo* rplInfos;
    std::vector<std::string> ReplayFiles;
    ReplayFiles = FS::findPhysFiles("Replays/*.rpl");
    xmDatabase* pDb = NULL;

    if(threadDb == NULL){
      pDb = m_db;
    } else {
      pDb = threadDb;
    }

    pDb->replays_add_begin();

    for(unsigned int i=0; i<ReplayFiles.size(); i++) {
      try {
	if(FS::getFileBaseName(ReplayFiles[i]) == "Latest") {
	  continue;
	}
	addReplay(ReplayFiles[i], pDb);
	if(pLoadReplaysInterface != NULL){
	  pLoadReplaysInterface->loadReplayHook(ReplayFiles[i], (int)((i*100)/((float)ReplayFiles.size())));
	}

      } catch(Exception &e) {
	// ok, forget this replay
      }
    }
    pDb->replays_add_end();
  }

  void GameApp::addReplay(const std::string& i_file, xmDatabase* threadDb) {
    ReplayInfo* rplInfos;
    xmDatabase* pDb = NULL;

    if(threadDb == NULL){
      pDb = m_db;
    } else {
      pDb = threadDb;
    }
    
    rplInfos = Replay::getReplayInfos(FS::getFileBaseName(i_file));
    if(rplInfos == NULL) {
      throw Exception("Unable to extract data from replay file");
    }

    try {
      pDb->replays_add(rplInfos->Level,
		       rplInfos->Name,
		       rplInfos->Player,
		       rplInfos->IsFinished,
		       rplInfos->fFinishTime);

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

void GameApp::addGhosts(MotoGame* i_motogame, Theme* i_theme) {
  std::string v_replay_MYBEST;
  std::string v_replay_THEBEST;
  std::string v_replay_BESTOFROOM;
  float v_fFinishTime;
  float v_player_fFinishTime;

  /* first, add the best of the room -- because if mybest or thebest = bestofroom, i prefer to see writen bestofroom */
  if(m_xmsession->ghostStrategy_BESTOFROOM()) {
    std::string v_replay_MYBEST_tmp;
    v_replay_MYBEST_tmp = _getGhostReplayPath_bestOfThePlayer(i_motogame->getLevelSrc()->Id(), v_player_fFinishTime);
    v_replay_BESTOFROOM = _getGhostReplayPath_bestOfTheRoom(i_motogame->getLevelSrc()->Id(), v_fFinishTime);

    /* add MYBEST if MYBEST if better the  BESTOF ROOM */
    if(v_player_fFinishTime > 0.0 && (v_fFinishTime < 0.0 || v_player_fFinishTime < v_fFinishTime)) {
      v_replay_BESTOFROOM = v_replay_MYBEST_tmp;
    }
    
    if(v_replay_BESTOFROOM != "") {
	m_MotoGame.addGhostFromFile(v_replay_BESTOFROOM,
				    m_db->webrooms_getName(m_xmsession->idRoom()),
				    &m_theme, m_theme.getGhostTheme(),
				    TColor(255,255,255,0),
				    TColor(GET_RED(i_theme->getGhostTheme()->getUglyRiderColor()),
					   GET_GREEN(i_theme->getGhostTheme()->getUglyRiderColor()),
					   GET_BLUE(i_theme->getGhostTheme()->getUglyRiderColor()),
					   0)
				    );
    }
  }

  /* second, add your best */
  if(m_xmsession->ghostStrategy_MYBEST()) {
    v_replay_MYBEST = _getGhostReplayPath_bestOfThePlayer(i_motogame->getLevelSrc()->Id(), v_fFinishTime);
    if(v_replay_MYBEST != "") {
      if(v_replay_MYBEST != v_replay_BESTOFROOM) {
	i_motogame->addGhostFromFile(v_replay_MYBEST, GAMETEXT_GHOST_BEST,
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
  if(m_xmsession->ghostStrategy_THEBEST()) {
    v_replay_THEBEST = _getGhostReplayPath_bestOfLocal(i_motogame->getLevelSrc()->Id(), v_fFinishTime);
    if(v_replay_THEBEST != "") {
      if(v_replay_THEBEST != v_replay_MYBEST && v_replay_THEBEST != v_replay_BESTOFROOM) { /* don't add two times the same ghost */
	i_motogame->addGhostFromFile(v_replay_THEBEST, GAMETEXT_GHOST_LOCAL,
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

}

void GameApp::addLevelToFavorite(const std::string& i_levelId) {
  m_levelsManager.addToFavorite(m_db, m_xmsession->profile(), i_levelId);
}

void GameApp::switchLevelToFavorite(const std::string& i_levelId, bool v_displayMessage) {
  if(m_levelsManager.isInFavorite(m_db, m_xmsession->profile(), i_levelId)) {
    m_levelsManager.delFromFavorite(m_db, m_xmsession->profile(), i_levelId);
    if(v_displayMessage) {
      m_sysMsg->displayText(GAMETEXT_LEVEL_DELETED_FROM_FAVORITE);
    }
  } else {
    m_levelsManager.addToFavorite(m_db, m_xmsession->profile(), i_levelId);
    if(v_displayMessage) {
      m_sysMsg->displayText(GAMETEXT_LEVEL_ADDED_TO_FAVORITE);
    }
  }
}

void GameApp::switchFollowCamera() {
  m_Renderer->switchFollow();

  m_sysMsg->displayText(m_MotoGame.getCamera()->
			getPlayerToFollow()->
			getQuickDescription());
}

XMSession* GameApp::getSession() {
  return m_xmsession;
}

MotoGame* GameApp::getMotoGame() {
  return &m_MotoGame;
}

void GameApp::requestEnd() {
  m_bQuit = true;
}

void GameApp::playMusic(const std::string& i_music) {
  if( (m_xmsession->enableAudio() && m_xmsession->enableMenuMusic()) || i_music == "") {
    if(i_music != m_playingMusic) {
      try {
	if(i_music == "") {
	  m_playingMusic = "";
	  Sound::stopMusic();
	} else {
	  m_playingMusic = i_music;
	  Sound::playMusic(m_theme.getMusic(i_music)->FilePath());
	}
      } catch(Exception &e) {
	Logger::Log("** Warning ** : PlayMusic(%s) failed", i_music.c_str());
	Sound::stopMusic();
      }
    }
  }
}

xmDatabase* GameApp::getDb() {
  return m_db;
}

bool GameApp::isAReplayToSave() const {
  return m_pJustPlayReplay != NULL;
}

void GameApp::isTheCurrentPlayAHighscore(bool& o_personal, bool& o_room) {
  int v_best_personal_time;
  int v_current_time;
  int v_best_room_time;
  char **v_result;
  unsigned int nrow;
  char *v_res;

  o_personal = o_room = false;

  if(m_MotoGame.Players().size() != 1) {
    return;
  }

  v_current_time = (int)(100.0 * m_MotoGame.Players()[0]->finishTime());

  /* get best player result */
  v_result = m_db->readDB("SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
			  "id_level=\"" + 
			  xmDatabase::protectString(m_MotoGame.getLevelSrc()->Id()) + "\" " + 
			  "AND id_profile=\"" + xmDatabase::protectString(m_xmsession->profile())  + "\";",
			  nrow);
  v_res = m_db->getResult(v_result, 1, 0, 0);
  if(v_res != NULL) {
    v_best_personal_time = (int)(100.0 * (atof(v_res) + 0.001)); /* + 0.001 because it is converted into a float */
  } else {
    /* should never happend because the score is already stored */
    v_best_personal_time = -1;
  }
  m_db->read_DB_free(v_result);
  o_personal = (v_current_time <= v_best_personal_time
		|| v_best_personal_time < 0);

  /* search a better webhighscore */
  v_best_room_time = (int)(100.0 * m_db->webrooms_getHighscoreTime(m_xmsession->idRoom(), m_MotoGame.getLevelSrc()->Id()));
  o_room = (v_current_time < v_best_room_time
	    || v_best_room_time < 0);
}

StateManager* GameApp::getStateManager() {
  return m_stateManager;
}

GameRenderer* GameApp::getGameRenderer() {
  return m_Renderer;
}

InputHandler* GameApp::getInputHandler() {
  return &m_InputHandler;
}

void GameApp::finalizeReplay(bool i_finished) {
  if(m_MotoGame.Players().size() != 1) return;

  /* save the last state because scene don't record each frame */
  SerializedBikeState BikeState;
  MotoGame::getSerializedBikeState(m_MotoGame.Players()[0]->getState(), m_MotoGame.getTime(), &BikeState);
  m_pJustPlayReplay->storeState(BikeState);
  m_pJustPlayReplay->finishReplay(i_finished, i_finished ? m_MotoGame.Players()[0]->finishTime() : 0.0);
}

Replay* GameApp::getCurrentReplay() {
  return m_pJustPlayReplay;
}

void GameApp::initReplay() {
  if(m_pJustPlayReplay != NULL) delete m_pJustPlayReplay;
  m_pJustPlayReplay = NULL;

  if(m_xmsession->storeReplays() && m_xmsession->multiNbPlayers() == 1) {
    m_pJustPlayReplay = new Replay;
    m_pJustPlayReplay->createReplay("Latest.rpl",
				    m_MotoGame.getLevelSrc()->Id(),
				    m_xmsession->profile(),
				    m_xmsession->replayFrameRate(),
				    sizeof(SerializedBikeState));
  }
}

LevelsManager* GameApp::getLevelsManager() {
  return &m_levelsManager;
}

ThemeChoicer* GameApp::getThemeChoicer()
{
  return m_themeChoicer;
}

std::string GameApp::getWebRoomURL(xmDatabase* pDb) {
  char **v_result;
  unsigned int nrow;
  std::string v_url;

  if(pDb == NULL){
    pDb = m_db;
  }

  v_result = pDb->readDB("SELECT highscoresUrl FROM webrooms WHERE id_room=" + m_xmsession->idRoom() + ";", nrow);
  if(nrow != 1) {
    pDb->read_DB_free(v_result);
    return DEFAULT_WEBROOMS_URL;
  }
  v_url = pDb->getResult(v_result, 1, 0, 0);
  pDb->read_DB_free(v_result);  

  return v_url;
}

std::string GameApp::getWebRoomName(xmDatabase* pDb) {
  char **v_result;
  unsigned int nrow;
  std::string v_name;

  if(pDb == NULL){
    pDb = m_db;
  }

  /* set the room name ; set to WR if it cannot be determined */
  v_name = "WR";

  v_result = pDb->readDB("SELECT name FROM webrooms WHERE id_room=" + m_xmsession->idRoom() + ";", nrow);
  if(nrow == 1) {
    v_name = pDb->getResult(v_result, 1, 0, 0);
  }
  pDb->read_DB_free(v_result);

  return v_name;
}

bool GameApp::getHighscoreInfos(const std::string& i_id_level, std::string* o_id_profile, std::string* o_url, bool* o_isAccessible) {
  char **v_result;
  unsigned int nrow;
    
  v_result = m_db->readDB("SELECT id_profile, fileUrl FROM webhighscores WHERE id_level=\"" + 
			  xmDatabase::protectString(i_id_level) + "\" AND id_room=" + m_xmsession->idRoom() + ";",
			  nrow);
  if(nrow == 0) {
    m_db->read_DB_free(v_result);
    return false;
  }

  *o_id_profile = m_db->getResult(v_result, 2, 0, 0);
  *o_url        = m_db->getResult(v_result, 2, 0, 1);
  m_db->read_DB_free(v_result);

  /* search if the replay is already downloaded */
  if(m_db->replays_exists(FS::getFileBaseName(*o_url))) {
    *o_isAccessible = true;
  } else {
    *o_isAccessible = m_xmsession->www();
  }

  return true;
}

void GameApp::loadLevelHook(std::string i_level, int i_percentage)
{
  std::ostringstream v_percentage;
  v_percentage << i_percentage;
  v_percentage << "%";
  _UpdateLoadingScreen(0, std::string(GAMETEXT_LOAD_LEVEL_HOOK) + std::string("\n") + v_percentage.str() + std::string(" ") + i_level);

  /* pump events to so that windows don't think the appli is crashed */
  SDL_PumpEvents();
}

void GameApp::updatingDatabase(std::string i_message) {
  _UpdateLoadingScreen(0, i_message);

  /* pump events to so that windows don't think the appli is crashed */
  SDL_PumpEvents();
}

void GameApp::drawFrame(void) {
  Sound::update();
  m_InputHandler.updateInput(m_MotoGame.Players());
  
  m_stateManager->update();
  m_stateManager->render(); 
}
