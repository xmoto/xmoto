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
  

  bool GameApp::isUglyMode() {
    return m_xmsession->ugly();
  }

GameApp::~GameApp() {
  if(m_db != NULL) {
    delete m_db;
  }

  if(drawLib != NULL) {
    delete drawLib;
  }

  delete m_xmsession;
  delete m_stateManager;

  StateManager::cleanStates();
}

GameApp::GameApp() {
  m_bQuit = false;
  drawLib = NULL;
  m_Renderer = NULL;
  
  m_xmsession = new XMSession();
  m_sysMsg = NULL;

  m_bEnableInitZoom=true;
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
  m_b50FpsMode = false;
  m_pJustPlayReplay = NULL;
  m_updateAutomaticallyLevels = false;
  m_reloadingLevelsUser = false;

  m_bRecordReplays = true;
  m_bCompressReplays = true;
 
  m_pWebHighscores = NULL;
  m_pWebLevels = NULL;
  m_pWebRooms = NULL;
  m_fDownloadTaskProgressLast = 0;
  m_bWebHighscoresUpdatedThisSession = false;
  m_bWebLevelsToDownload = false;
  
  m_MotoGame.setHooks(&m_MotoGameHooks);
  m_MotoGameHooks.setGameApps(this, &m_MotoGame);
  
  m_currentPlayingList = NULL;
  m_fReplayFrameRate = 25.0;
  m_allowReplayInterpolation = true;

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
  Update levels lists - must be done after each completed level
  ===========================================================================*/
  void GameApp::_UpdateLevelLists(void) {
    LevelsPack *v_levelsPack;

    _CreateLevelLists((UILevelList *)m_pLevelPacksWindow->getChild("LEVELPACK_TABS:ALLLEVELS_TAB:ALLLEVELS_LIST"), VPACKAGENAME_FAVORITE_LEVELS);

    _CreateLevelLists(m_pPlayNewLevelsList, VPACKAGENAME_NEW_LEVELS);
  }

  /*===========================================================================
  Update replays list
  ===========================================================================*/
  void GameApp::_UpdateReplaysList(void) {
    _CreateReplaysList((UIList *)m_pReplaysWindow->getChild("REPLAY_LIST"));                       
  }

  void GameApp::_UpdateRoomsLists(void) {
    _CreateRoomsList((UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_ROOMS_TAB:ROOMS_LIST"));
  }

  void GameApp::_UpdateThemesLists(void) {
    _CreateThemesList((UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:THEMES_LIST"));
  }

  /*===========================================================================
  Update settings
  ===========================================================================*/
  void GameApp::_UpdateSettings(void) {
    std::string v_menuGraphics =   m_Config.getString("MenuGraphics");
    if(v_menuGraphics == "Low")    m_xmsession->setMenuGraphics(GFX_LOW);
    if(v_menuGraphics == "Medium") m_xmsession->setMenuGraphics(GFX_MEDIUM);
    if(v_menuGraphics == "High")   m_xmsession->setMenuGraphics(GFX_HIGH);

    /* Game graphics */
    std::string s = m_Config.getString("GameGraphics");

    if(m_xmsession->useGraphics()) {
      if(s == "Low") m_Renderer->setQuality(GQ_LOW);
      else if(s == "Medium") m_Renderer->setQuality(GQ_MEDIUM);
      else if(s == "High") m_Renderer->setQuality(GQ_HIGH);
    }      

    /* Show mini map? && show engine counter */
    m_xmsession->setShowMinimap(m_Config.getBool("ShowMiniMap"));
    m_xmsession->setShowEngineCounter(m_Config.getBool("ShowEngineCounter"));

    /* Replay stuff */
    m_fReplayFrameRate = m_Config.getFloat("ReplayFrameRate");
    m_bRecordReplays = m_Config.getBool("StoreReplays");
    m_bCompressReplays = m_Config.getBool("CompressReplays");
    Replay::enableCompression(m_bCompressReplays);

    /* ghost */
    m_MotoGame.setShowGhostTimeDiff(m_xmsession->showGhostTimeDifference());
    if(m_xmsession->useGraphics()) {
      m_Renderer->setGhostMotionBlur(m_xmsession->ghostMotionBlur());
    }

    if(m_xmsession->useGraphics()) {
      m_Renderer->setGhostDisplayInformation(m_xmsession->showGhostsInfos());
      m_Renderer->setHideGhosts(m_xmsession->hideGhosts());
    }

    /* Other settings */
    m_xmsession->setEnableMenuMusic(m_Config.getBool("MenuMusic"));
    m_bEnableInitZoom = m_Config.getBool("InitZoom");
    m_xmsession->setEnableDeadAnimation(m_Config.getBool("DeathAnim"));

    /* www */
    m_WebHighscoresURL    = m_Config.getString("WebHighscoresURL");

    /* Configure proxy */
    _ConfigureProxy();
  }
    
  /*===========================================================================
  Screenshooting
  ===========================================================================*/
  void GameApp::_GameScreenshot(void) {
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
    v_ShotExtension = m_Config.getString("ScreenshotFormat");
    
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
  /* No matter what, F12 always equals a screenshot */
  if(nKey == SDLK_F12) {
    _GameScreenshot();
    return;        
  }

  if(nKey == SDLK_F8) {
    enableWWW(m_xmsession->www() == false);
    return;        
  }

  if(nKey == SDLK_F7) {
    enableFps(m_xmsession->fps() == false);
    return;        
  }

  if(nKey == SDLK_F9) {
    switchUglyMode(m_xmsession->ugly() == false);
    if(m_xmsession->ugly()) {
      m_sysMsg->displayText(SYS_MSG_UGLY_MODE_ENABLED);
    } else {
      m_sysMsg->displayText(SYS_MSG_UGLY_MODE_DISABLED);
    }
    return;        
  }

  if(nKey == SDLK_RETURN && (((mod & KMOD_LALT) == KMOD_LALT) || ((mod & KMOD_RALT) == KMOD_RALT))) {
    drawLib->toogleFullscreen();
    m_xmsession->setWindowed(m_xmsession->windowed() == false);
    return;
  }

  if(nKey == SDLK_F10) {
    switchTestThemeMode(m_xmsession->testTheme() == false);
    if(m_xmsession->testTheme()) {
      m_sysMsg->displayText(SYS_MSG_THEME_MODE_ENABLED);
    } else {
      m_sysMsg->displayText(SYS_MSG_THEME_MODE_DISABLED);
    }
    return;        
  }

  if(nKey == SDLK_F11) {
    switchUglyOverMode(m_xmsession->uglyOver() == false);
    if(m_xmsession->uglyOver()) {
      m_sysMsg->displayText(SYS_MSG_UGLY_OVER_MODE_ENABLED);
    } else {
      m_sysMsg->displayText(SYS_MSG_UGLY_OVER_MODE_DISABLED);
    }
    return;        
  }

  /* activate/desactivate interpolation */
  if(nKey == SDLK_i && ( (mod & KMOD_LCTRL) || (mod & KMOD_RCTRL) )) {
    m_allowReplayInterpolation = !m_allowReplayInterpolation;
    if(m_allowReplayInterpolation) {
      m_sysMsg->displayText(SYS_MSG_INTERPOLATION_ENABLED);
    } else {
      m_sysMsg->displayText(SYS_MSG_INTERPOLATION_DISABLED);
    }

    for(unsigned int i=0; i<m_MotoGame.Players().size(); i++) {
      m_MotoGame.Players()[i]->setInterpolation(m_allowReplayInterpolation);
    }

    return;
  }

  if(nKey == SDLK_m && ( (mod & KMOD_LCTRL) || (mod & KMOD_RCTRL) )) {
    for(unsigned int i=0; i<m_MotoGame.Cameras().size(); i++) {
      m_MotoGame.Cameras()[i]->setMirrored(m_MotoGame.Cameras()[i]->isMirrored() == false);
    }
    m_InputHandler.setMirrored(m_MotoGame.Cameras()[0]->isMirrored());
  }
    
  /* If message box... */
  if(m_pNotifyMsgBox) {
    if(nKey == SDLK_ESCAPE) {
      delete m_pNotifyMsgBox;
      m_pNotifyMsgBox = NULL;
    }    
    else
      m_Renderer->getGUI()->keyDown(nKey, mod, nChar);      
    return;
  }
  

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
  Notification popup
  ===========================================================================*/
  void GameApp::notifyMsg(std::string Msg) {
    if(m_pNotifyMsgBox != NULL) delete m_pNotifyMsgBox;
    m_Renderer->getGUI()->setFont(drawLib->getFontSmall());
    m_pNotifyMsgBox = m_Renderer->getGUI()->msgBox(Msg,(UIMsgBoxButton)(UI_MSGBOX_OK));
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
      notifyMsg(GAMETEXT_FAILEDTOSAVEREPLAY);
    } else {
      /* Update replay list to reflect changes */
      addReplay(v_outputfile);
      _UpdateReplaysList();
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

  void GameApp::_UpdateWebHighscores(bool bSilent) {
    if(!bSilent) {
      _SimpleMessage(GAMETEXT_DLHIGHSCORES,&m_InfoMsgBoxRect);
    }

    m_bWebHighscoresUpdatedThisSession = true;
    
    /* Try downloading the highscores */
    m_pWebHighscores->setWebsiteInfos(m_xmsession->idRoom(),
				      m_WebHighscoresURL);
    Logger::Log("WWW: Checking for new highscores...");
    m_pWebHighscores->update();
  }

  void GameApp::_UpdateWebLevels(bool bSilent, bool bEnableWeb) {
    if(!bSilent) {
      _SimpleMessage(GAMETEXT_DLLEVELSCHECK,&m_InfoMsgBoxRect);
    }

    /* Try download levels list */
    if(m_pWebLevels == NULL) {
      m_pWebLevels = new WebLevels(this,&m_ProxySettings);
    }
    m_pWebLevels->setURL(m_Config.getString("WebLevelsURL"));
    Logger::Log("WWW: Checking for new or updated levels...");

    m_pWebLevels->update(m_db);
    m_bWebLevelsToDownload = m_pWebLevels->nbLevelsToGet(m_db);
  }

  void GameApp::_UpdateWebThemes(bool bSilent) {
    if(!bSilent) {
      _SimpleMessage(GAMETEXT_DLTHEMESLISTCHECK,&m_InfoMsgBoxRect);
    }  

    m_themeChoicer->setURL(m_Config.getString("WebThemesURL"));

    Logger::Log("WWW: Checking for new or updated themes...");

    try {
      m_DownloadingInformation = "";
      m_themeChoicer->updateFromWWW(m_db);
      _UpdateThemesLists();
    } catch(Exception &e) {
      /* file probably doesn't exist */
      Logger::Log("** Warning ** : Failed to analyse web-themes file");   
    }
  }    

  void GameApp::_UpdateWebRooms(bool bSilent) {
    if(!bSilent) {
      _SimpleMessage(GAMETEXT_DLROOMSLISTCHECK,&m_InfoMsgBoxRect);
    }  

    m_pWebRooms->setURL(m_Config.getString("WebRoomsURL"));

    Logger::Log("WWW: Checking for rooms list...");

    try {
      m_pWebRooms->update();
    } catch(Exception &e) {
      /* file probably doesn't exist */
      Logger::Log("** Warning ** : Failed to analyse update webrooms list");    
    }
  }

  void GameApp::_UpdateWebTheme(const std::string& i_id_theme, bool bNotify) {
    char **v_result;
    unsigned int nrow;
    std::string v_id_theme;
    std::string v_ck1, v_ck2;
    bool v_onDisk = false;
    bool v_onWeb  = true;

    v_result = m_db->readDB("SELECT a.id_theme, a.checkSum, b.checkSum "
    			    "FROM themes AS a LEFT OUTER JOIN webthemes AS b "
    			    "ON a.id_theme=b.id_theme "
			    "WHERE a.id_theme=\"" + xmDatabase::protectString(i_id_theme) + "\";",
    			    nrow);
    if(nrow == 1) {
      v_onDisk   = true;
      v_id_theme = m_db->getResult(v_result, 3, 0, 0);
      v_ck1      = m_db->getResult(v_result, 3, 0, 1);
      if(m_db->getResult(v_result, 3, 0, 2) == NULL) {
	v_onWeb = false;
      } else {
	v_ck2      = m_db->getResult(v_result, 3, 0, 2);
      }
    }
    m_db->read_DB_free(v_result);

    if(v_onWeb == false) { /* available on the disk, not on the web */
      if(bNotify) {
	notifyMsg(GAMETEXT_UNUPDATABLETHEMEONWEB);
      }
      return;
    }


    m_DownloadingInformation = "";
    m_DownloadingMessage = std::string(GAMETEXT_DLTHEME) + "\n\n ";
    try {
      Logger::Log("WWW: Downloading a theme...");
      clearCancelAsSoonAsPossible();
      m_themeChoicer->updateThemeFromWWW(m_db, i_id_theme);
      _UpdateThemesLists();
      reloadTheme(); /* reload the theme */
      if(bNotify) {
	notifyMsg(GAMETEXT_THEMEUPTODATE);
      }
    } catch(Exception &e) {
      /* file probably doesn't exist */
      Logger::Log("** Warning ** : Failed to update theme ", i_id_theme.c_str());
      if(bNotify) {
	notifyMsg(GAMETEXT_FAILEDGETSELECTEDTHEME + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW);
      }
      return;
    }
  }

  void GameApp::_UpgradeWebHighscores() {
    /* Upgrade high scores */
    try {
      m_pWebHighscores->upgrade(m_db);
    } catch(Exception &e) {
      /* file probably doesn't exist */
      Logger::Log("** Warning ** : Failed to analyse web-highscores file");   
    }
  }

  void GameApp::_UpgradeWebRooms(bool bUpdateMenus) {
    /* Upgrade high scores */
    try {
      m_pWebRooms->upgrade(m_db);
      if(bUpdateMenus) {
	_UpdateRoomsLists();
      }
    } catch(Exception &e) {
      /* file probably doesn't exist */
      Logger::Log("** Warning ** : Failed to analyse webrooms file");   
    }
  }

  /*===========================================================================
  Extra WWW levels
  ===========================================================================*/
  void GameApp::_DownloadExtraLevels(void) {
      /* Download extra levels */
      m_DownloadingInformation = "";
      m_DownloadingMessage = std::string(GAMETEXT_DLLEVELS) + "\n\n ";

      if(m_pWebLevels != NULL) {
        try {                  
          Logger::Log("WWW: Downloading levels...");
          clearCancelAsSoonAsPossible();
          m_pWebLevels->upgrade(m_db);
	  m_bWebLevelsToDownload = false;
        } 
        catch(Exception &e) {
          Logger::Log("** Warning ** : Unable to download extra levels [%s]",e.getMsg().c_str());
  
          if(m_pInfoMsgBox != NULL) {
            delete m_pInfoMsgBox;
            m_pInfoMsgBox = NULL;
          }
          notifyMsg(GAMETEXT_FAILEDDLLEVELS + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW);
        }

        /* Got some new levels... load them! */
        Logger::Log("Loading new and updated levels...");
	m_levelsManager.updateLevelsFromLvl(m_db,
					    m_pWebLevels->getNewDownloadedLevels(),
					    m_pWebLevels->getUpdatedDownloadedLevels()
					    );

         /* Update level lists */
	_UpdateLevelsLists();
      }
  }

  void GameApp::checkForExtraLevels(void) {
    /* Check for extra levels */
    try {
      _SimpleMessage(GAMETEXT_CHECKINGFORLEVELS);
      
      if(m_pWebLevels == NULL) {
	m_pWebLevels = new WebLevels(this,&m_ProxySettings);
      }
      m_pWebLevels->setURL(m_Config.getString("WebLevelsURL"));
        
      Logger::Log("WWW: Checking for new or updated levels...");
      clearCancelAsSoonAsPossible();

      m_pWebLevels->update(m_db);
      int nULevels=0;
      nULevels = m_pWebLevels->nbLevelsToGet(m_db);
      m_bWebLevelsToDownload = nULevels!=0;

      Logger::Log("WWW: %d new or updated level%s found",nULevels,nULevels==1?"":"s");

      if(nULevels == 0) {
	notifyMsg(GAMETEXT_NONEWLEVELS);
      }        
      else {
	/* Ask user whether he want to download levels or snot */
	if(m_pInfoMsgBox == NULL) {
	  char cBuf[256];
	  snprintf(cBuf, 256, GAMETEXT_NEWLEVELAVAIL(nULevels), nULevels);
	  m_Renderer->getGUI()->setFont(drawLib->getFontSmall());
	  m_pInfoMsgBox = m_Renderer->getGUI()->msgBox(cBuf, (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
	}
      }
    } catch(Exception &e) {
      Logger::Log("** Warning ** : Unable to check for extra levels [%s]",e.getMsg().c_str());
      if(m_pInfoMsgBox != NULL) {
	delete m_pInfoMsgBox;
	m_pInfoMsgBox = NULL;
      }

      std::string v_msg = GAMETEXT_FAILEDCHECKLEVELS + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW;
      m_stateManager->pushState(new StateMessageBox(NULL, this, v_msg, UI_MSGBOX_OK));
    } 
  }

  /*===========================================================================
  WWWAppInterface implementation
  ===========================================================================*/
  bool GameApp::shouldLevelBeUpdated(const std::string &LevelID) {
    if(m_updateAutomaticallyLevels) {
      return true;
    }

    /* Hmm... ask user whether this level should be updated */
    bool bRet = true;
    bool bDialogBoxOpen = true;
    char cBuf[1024];
    char **v_result;
    unsigned int nrow;
    std::string v_levelName;
    std::string v_levelFileName;

    v_result = m_db->readDB("SELECT name, filepath "
			    "FROM levels "
			    "WHERE id_level=\"" + xmDatabase::protectString(LevelID) + "\";",
			    nrow);
    if(nrow != 1) {
      m_db->read_DB_free(v_result);
      return true;
    }

    v_levelName     = m_db->getResult(v_result, 2, 0, 0);
    v_levelFileName = m_db->getResult(v_result, 2, 0, 1);
    m_db->read_DB_free(v_result);

    sprintf(cBuf,(std::string(GAMETEXT_WANTTOUPDATELEVEL) + "\n(%s)").c_str(), v_levelName.c_str(),
	    v_levelFileName.c_str());
    m_Renderer->getGUI()->setFont(drawLib->getFontSmall());
    UIMsgBox *pMsgBox = m_Renderer->getGUI()->msgBox(cBuf,(UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO|UI_MSGBOX_YES_FOR_ALL));

    while(bDialogBoxOpen) {
      SDL_PumpEvents();
      
      SDL_Event Event;
      while(SDL_PollEvent(&Event)) {
	/* What event? */
	switch(Event.type) {
	case SDL_QUIT:  
	  /* Force quit */
	  quit();
	  setCancelAsSoonAsPossible();
	  return false;
	case SDL_MOUSEBUTTONDOWN:
	  mouseDown(Event.button.button);
	  break;
	case SDL_MOUSEBUTTONUP:
	  mouseUp(Event.button.button);
	  break;
	}
      }
      
      UIMsgBoxButton Button = pMsgBox->getClicked();
      if(Button != UI_MSGBOX_NOTHING) {
	if(Button == UI_MSGBOX_NO) {
	  bRet = false;
	}
	if(Button == UI_MSGBOX_YES_FOR_ALL) {
	  m_updateAutomaticallyLevels = true;
	}
	bDialogBoxOpen = false;
      }
      
      m_Renderer->getGUI()->dispatchMouseHover();
      
      m_Renderer->getGUI()->paint();
      
      UIRect TempRect;
      
      //if(m_pCursor != NULL) {        
      //	int nMX,nMY;
      //	getMousePos(&nMX,&nMY);      
      //	drawLib->drawImage(Vector2f(nMX-2,nMY-2),Vector2f(nMX+30,nMY+30),m_pCursor);
      //}
      
      drawLib->flushGraphics();
    }
    
    delete pMsgBox;
    setTaskProgress(m_fDownloadTaskProgressLast);
    
    return bRet;        
  }
        
  void GameApp::setTaskProgress(float fPercent) {
    int nBarHeight = 15;
    m_fDownloadTaskProgressLast = fPercent;
    readEvents();

    _SimpleMessage(m_DownloadingMessage,&m_InfoMsgBoxRect,true);
    
    drawLib->drawBox(Vector2f(m_InfoMsgBoxRect.nX+10,m_InfoMsgBoxRect.nY+ m_InfoMsgBoxRect.nHeight-
                                                   nBarHeight*2),
            Vector2f(m_InfoMsgBoxRect.nX+m_InfoMsgBoxRect.nWidth-10,
                     m_InfoMsgBoxRect.nY+m_InfoMsgBoxRect.nHeight-nBarHeight),
            0,MAKE_COLOR(0,0,0,255),0);
            
                
    drawLib->drawBox(Vector2f(m_InfoMsgBoxRect.nX+10,m_InfoMsgBoxRect.nY+
                                                   m_InfoMsgBoxRect.nHeight-
                                                   nBarHeight*2),
            Vector2f(m_InfoMsgBoxRect.nX+10+((m_InfoMsgBoxRect.nWidth-20)*(int)fPercent)/100,
                     m_InfoMsgBoxRect.nY+m_InfoMsgBoxRect.nHeight-nBarHeight),
            0,MAKE_COLOR(255,0,0,255),0);

    FontManager* v_fm = drawLib->getFontSmall();
    FontGlyph* v_fg = v_fm->getGlyph(m_DownloadingInformation);
    v_fm->printString(v_fg,
		      m_InfoMsgBoxRect.nX+10,
		      m_InfoMsgBoxRect.nY+m_InfoMsgBoxRect.nHeight-nBarHeight*2,
		      MAKE_COLOR(255,255,255,128));
    drawLib->flushGraphics();
  }
  
  void GameApp::setBeingDownloadedInformation(const std::string &p_information,bool bNew) {
    m_DownloadingInformation = p_information;
  }
  
  void GameApp::readEvents(void) {
    /* Check for events */ 
    SDL_PumpEvents();
    
    SDL_Event Event;
    while(SDL_PollEvent(&Event)) {
      /* What event? */
      switch(Event.type) {
        case SDL_KEYDOWN: 
          if(Event.key.keysym.sym == SDLK_ESCAPE)
            setCancelAsSoonAsPossible();
          break;
        case SDL_QUIT:  
          /* Force quit */
          quit();
          setCancelAsSoonAsPossible();
          return;
      }
    }    
  }
  
  /*===========================================================================
  Configure proxy
  ===========================================================================*/
  void GameApp::_ConfigureProxy(void) {
    bool bFetchPortAndServer = false;
  
    /* Proxy? */        
    std::string s = m_Config.getString("ProxyType");
    if(s == "HTTP") {
      m_ProxySettings.setType(CURLPROXY_HTTP);
      bFetchPortAndServer = true;
    }
    else if(s == "SOCKS4") {
      m_ProxySettings.setType(CURLPROXY_SOCKS4);
      bFetchPortAndServer = true;
    }
    else if(s == "SOCKS5") {
      m_ProxySettings.setType(CURLPROXY_SOCKS5);
      bFetchPortAndServer = true;
    }
    else {
      m_ProxySettings.setDefaultAuthentification();
      m_ProxySettings.setDefaultPort();
      m_ProxySettings.setDefaultServer();
      m_ProxySettings.setDefaultType();
    }
    
    if(bFetchPortAndServer) {
      m_ProxySettings.setServer(m_Config.getString("ProxyServer"));
      m_ProxySettings.setPort(m_Config.getInteger("ProxyPort"));
      m_ProxySettings.setAuthentification(m_Config.getString("ProxyAuthUser"),
            m_Config.getString("ProxyAuthPwd"));      
    }
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

  std::string GameApp::_getGhostReplayPath_bestOfTheRoom(std::string p_levelId, float &p_time) {
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
      res = std::string("Replays/") + v_replayName + std::string(".rpl");
    } else {
      if(m_xmsession->www()) {
	/* download the replay */
	try {
	  _SimpleMessage(GAMETEXT_DLGHOST,&m_InfoMsgBoxRect);
	  m_pWebHighscores->downloadReplay(v_fileUrl);
	  addReplay(v_replayName);
	  _UpdateReplaysList();
	  res = std::string("Replays/") + v_replayName + std::string(".rpl");
	} catch(Exception &e) {
	  /* do nothing */
	  enableWWW(false);
	}
      }
    }
    return res;
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

  void GameApp::uploadHighscore(std::string p_replayname, bool b_notify) {
    std::string v_msg;

    try {
      bool v_msg_status_ok;
      clearCancelAsSoonAsPossible();
      m_DownloadingInformation = "";
      m_DownloadingMessage = GAMETEXT_UPLOADING_HIGHSCORE;
      FSWeb::uploadReplay(FS::getUserDir() + "/Replays/" + p_replayname + ".rpl",
        m_xmsession->idRoom(),
        m_Config.getString("WebHighscoreUploadLogin"),
        m_Config.getString("WebHighscoreUploadPassword"),
        m_Config.getString("WebHighscoreUploadURL"),
        this,
        &m_ProxySettings,
        v_msg_status_ok,
        v_msg);
      if(v_msg_status_ok) {
	if(b_notify) {
	  notifyMsg(v_msg);
	}
      } else {
	if(b_notify) {
	  notifyMsg(std::string(GAMETEXT_UPLOAD_HIGHSCORE_WEB_WARNING_BEFORE) + "\n" + v_msg);
	}
      }
    } catch(Exception &e) {
      if(b_notify) {
	notifyMsg(GAMETEXT_UPLOAD_HIGHSCORE_ERROR + std::string("\n") + v_msg);
      } else {
	throw Exception(GAMETEXT_UPLOAD_HIGHSCORE_ERROR + std::string("\n") + v_msg);
      }
    }
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

  void GameApp::_UploadAllHighscores() {
    /* 1 is the main room ; don't allow full upload on it */
    if(m_xmsession->idRoom() == "1") return;

    _UpdateWebHighscores(false);
    char **v_result;
    unsigned int nrow;
    std::string v_previousIdLevel, v_currentIdLevel;

	std::string query = "SELECT r.id_level, r.name FROM replays r "
    "LEFT OUTER JOIN webhighscores h "
    "ON (r.id_level = h.id_level AND h.id_room=" + m_xmsession->idRoom() + ") "
    "INNER JOIN weblevels l ON r.id_level = l.id_level "
    "WHERE r.id_profile=\"" + xmDatabase::protectString(m_xmsession->profile()) + "\" "
    "AND r.isFinished "
    "AND ( (h.id_room IS NULL) OR xm_floord(h.finishTime*100.0) > xm_floord(r.finishTime*100.0)) "
    "ORDER BY r.id_level, r.finishTime;";
    v_result = m_db->readDB(query, nrow);

    try {
      for (int i = 0; i<nrow; i++) {
	std::ostringstream v_percentage;
	v_percentage << std::setprecision (1);
	v_percentage << (i*100.0/nrow);

	v_currentIdLevel = m_db->getResult(v_result, 2, i, 0);
	
	/* send only the best of the replay by level */
	if(v_previousIdLevel != v_currentIdLevel) {
	  v_previousIdLevel = v_currentIdLevel;
	  _SimpleMessage(GAMETEXT_UPLOADING_HIGHSCORE + std::string("\n") + v_percentage.str() + "%");
	  uploadHighscore(m_db->getResult(v_result, 2, i, 1), false);
	}
      }
    } catch(Exception &e) {
      notifyMsg(e.getMsg());
    }
    m_db->read_DB_free(v_result);
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
    m_Renderer->setUglyMode(bUgly);
  }

  void GameApp::switchTestThemeMode(bool mode) {
    m_xmsession->setTestTheme(mode);
    m_Renderer->setTestThemeMode(mode);
  }

  void GameApp::switchUglyOverMode(bool mode) {
    m_xmsession->setUglyOver(mode);
    m_Renderer->setUglyOverMode(mode);
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

  void GameApp::_UpdateLevelsLists() {
    _UpdateLevelPackList();
    _UpdateLevelLists();
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

  void GameApp::loadLevelHook(std::string i_level, int i_percentage) {
#if 0
    std::ostringstream v_percentage;
    v_percentage << i_percentage;
    v_percentage << "%";

    if(m_reloadingLevelsUser == false) {
      _UpdateLoadingScreen(0, std::string(GAMETEXT_LOAD_LEVEL_HOOK) + std::string("\n") + v_percentage.str() + std::string(" ") + i_level);
    } else {
      _SimpleMessage(GAMETEXT_RELOADINGLEVELS + std::string("\n") + v_percentage.str(), &m_InfoMsgBoxRect);
    }

    /* pump events to so that windows don't think the appli is crashed */
    SDL_PumpEvents();
#endif
  }

  void GameApp::updatingDatabase(std::string i_message) {
    _UpdateLoadingScreen(0, i_message);

    /* pump events to so that windows don't think the appli is crashed */
    SDL_PumpEvents();
  }

  bool GameApp::creditsModeActive() {
    return m_bCreditsModeActive;
  }

  void GameApp::initReplaysFromDir(xmDatabase* threadDb) {
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

  void GameApp::_UpdateCurrentPackList(const std::string& i_id_level, float i_playerHighscore) {
    //    if(m_pActiveLevelPack == NULL)
    //      return;

    UILevelList *pList = (UILevelList *)m_pLevelPackViewer->getChild("LEVELPACK_LEVEL_LIST"); 
    if(pList == NULL) return;

    pList->updateLevel(i_id_level, i_playerHighscore);
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
  _UpdateLevelPackLevelList(VPACKAGENAME_FAVORITE_LEVELS);
  _UpdateLevelLists();
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

  _UpdateLevelPackLevelList(VPACKAGENAME_FAVORITE_LEVELS);
  _UpdateLevelLists();
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

void GameApp::updateLevelsListsOnEnd() {
  _UpdateLevelsLists();
  _UpdateCurrentPackList(m_MotoGame.getLevelSrc()->Id(),
			 m_MotoGame.Players()[0]->finishTime());
}

Replay* GameApp::getCurrentReplay() {
  return m_pJustPlayReplay;
}

void GameApp::initReplay() {
  if(m_pJustPlayReplay != NULL) delete m_pJustPlayReplay;
  m_pJustPlayReplay = NULL;

  if(m_bRecordReplays && m_xmsession->multiNbPlayers() == 1) {
    m_pJustPlayReplay = new Replay;
    m_pJustPlayReplay->createReplay("Latest.rpl",
				    m_MotoGame.getLevelSrc()->Id(),
				    m_xmsession->profile(),
				    m_fReplayFrameRate,
				    sizeof(SerializedBikeState));
  }
}

void GameApp::updateWebHighscores()
{
  if(!m_bWebHighscoresUpdatedThisSession) {        
    try {
      _UpdateWebHighscores(false);
      _UpgradeWebHighscores();  
      _UpdateWebLevels(false);

      m_levelsManager.makePacks(m_db,
				m_xmsession->profile(),
				m_xmsession->idRoom(),
				m_xmsession->debug());
      _UpdateLevelsLists();
    } catch(Exception &e) {
      notifyMsg(GAMETEXT_FAILEDDLHIGHSCORES + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW);
    }
  }
}

LevelsManager* GameApp::getLevelsManager() {
  return &m_levelsManager;
}

ThemeChoicer* GameApp::getThemeChoicer()
{
  return m_themeChoicer;
}
