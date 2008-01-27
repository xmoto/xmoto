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
 *  Game application. (init-related stuff)
 */
 
/* rneckelmann 2006-09-30: moved a lot of stuff from Game.cpp into here to 
                           make it a tad smaller */ 
#include "GameText.h"
#include "Game.h"
#include "VFileIO.h"
#include "Sound.h"
#include "PhysSettings.h"
#include "Input.h"
#include "db/xmDatabase.h"
#include "helpers/Log.h"

#include <curl/curl.h>
#include "XMSession.h"
#include "XMArgs.h"
#include "drawlib/DrawLib.h"
#include "Packager.h"
#include "helpers/SwapEndian.h"
#include "SysMessage.h"
#include "gui/specific/GUIXMoto.h"
#include "Credits.h"
#include "Replay.h"

#include "states/StateManager.h"
#include "states/StateEditProfile.h"
#include "states/StateReplaying.h"
#include "states/StatePreplaying.h"
#include "states/StateMainMenu.h"
#include "states/StateMessageBox.h"

#if defined(WIN32)
int SDL_main(int nNumArgs, char **ppcArgs) {
#else
int main(int nNumArgs, char **ppcArgs) {
#endif
  /* Start application */
  try {     
    /* Setup basic info */
    GameApp::instance()->run(nNumArgs, ppcArgs);
  }
  catch (Exception &e) {
    if(Logger::isInitialized()) {
      Logger::Log((std::string("Exception: ") + e.getMsg()).c_str());
    }    

    printf("fatal exception : %s\n", e.getMsg().c_str());        
    SDL_Quit(); /* make sure SDL shuts down gracefully */
    
#if defined(WIN32)
    char cBuf[1024];
    sprintf(cBuf,"Fatal exception occured: %s\n"
	    "Consult the file xmoto.log for more information about what\n"
	    "might has occured.\n",e.getMsg().c_str());                    
    MessageBox(NULL,cBuf,"X-Moto Error",MB_OK|MB_ICONERROR);
#endif
  }
  return 0;
}

void GameApp::run(int nNumArgs, char** ppcArgs) {
  run_load(nNumArgs, ppcArgs);
  run_loop();
  run_unload();
}

void GameApp::run_load(int nNumArgs, char** ppcArgs) {
  XMArguments v_xmArgs;
  bool v_useGraphics = true;

  /* check args */
  try {
    v_xmArgs.parse(nNumArgs, ppcArgs);
  } catch (Exception &e) {
    printf("syntax error : %s\n", e.getMsg().c_str());
    v_xmArgs.help(nNumArgs >= 1 ? ppcArgs[0] : "xmoto");
    quit();
    return; /* abort */
  }

  /* help */
  if(v_xmArgs.isOptHelp()) {
    v_xmArgs.help(nNumArgs >= 1 ? ppcArgs[0] : "xmoto");
    quit();
    return;
  }

  /* init sub-systems */
  SwapEndian::Swap_Init();
  srand(time(NULL));

  /* package / unpackage */
  if(v_xmArgs.isOptPack()) {
    Packager::go(v_xmArgs.getOpt_pack_bin() == "" ? "xmoto.bin" : v_xmArgs.getOpt_pack_bin(),
		 v_xmArgs.getOpt_pack_dir() == "" ? "."         : v_xmArgs.getOpt_pack_dir());
    quit();
    return;
  }
  if(v_xmArgs.isOptUnPack()) {
    Packager::goUnpack(v_xmArgs.getOpt_unpack_bin() == "" ? "xmoto.bin" : v_xmArgs.getOpt_unpack_bin(),
		       v_xmArgs.getOpt_unpack_dir() == "" ? "."         : v_xmArgs.getOpt_unpack_dir(),
		       v_xmArgs.getOpt_unpack_noList() == false);
    quit();
    return;
  }
  /* ***** */

  if(v_xmArgs.isOptConfigPath()) {
    FS::init("xmoto", "xmoto.bin", "xmoto.log", v_xmArgs.getOpt_configPath_path());
  } else {
    FS::init("xmoto", "xmoto.bin", "xmoto.log");
  }
  Logger::init(FS::getUserDir() + "/xmoto.log");

  /* load config file, the session */
  XMSession::createDefaultConfig(&m_Config);
  m_Config.loadFile();
  XMSession::instance()->load(&m_Config); /* overload default session by userConfig */
  XMSession::instance()->load(&v_xmArgs); /* overload default session by xmargs     */
  Logger::setVerbose(XMSession::instance()->isVerbose()); /* apply verbose mode */

#ifdef USE_GETTEXT
    std::string v_locale = Locales::init(XMSession::instance()->language());
#endif

  Logger::Log("compiled at "__DATE__" "__TIME__);
  if(SwapEndian::bigendien) {
    Logger::Log("Systeme is bigendien");
  } else {
    Logger::Log("Systeme is littleendien");
  }
  Logger::Log("User directory: %s", FS::getUserDir().c_str());
  Logger::Log("Data directory: %s", FS::getDataDir().c_str());

#ifdef USE_GETTEXT
  Logger::Log("Locales set to '%s' (directory '%s')", v_locale.c_str(), LOCALESDIR);
#endif

  if(v_xmArgs.isOptListLevels() || v_xmArgs.isOptListReplays() || v_xmArgs.isOptReplayInfos()) {
    v_useGraphics = false;
  }

  _InitWin(v_useGraphics);

  if(v_useGraphics) {
    /* init drawLib */
    drawLib = DrawLib::DrawLibFromName(XMSession::instance()->drawlib());

    if(drawLib == NULL) {
      throw Exception("Drawlib not initialized");
    }

    SysMessage::instance()->setDrawLib(drawLib);
    
    drawLib->setNoGraphics(v_useGraphics == false);
    drawLib->setDontUseGLExtensions(XMSession::instance()->glExts() == false);

    /* Init! */
    drawLib->init(XMSession::instance()->resolutionWidth(), XMSession::instance()->resolutionHeight(), XMSession::instance()->bpp(), XMSession::instance()->windowed());
    /* drawlib can change the final resolution if it fails, then, reinit session one's */
    XMSession::instance()->setResolutionWidth(drawLib->getDispWidth());
    XMSession::instance()->setResolutionHeight(drawLib->getDispHeight());
    XMSession::instance()->setBpp(drawLib->getDispBPP());
    XMSession::instance()->setWindowed(drawLib->getWindowed());
    Logger::Log("Resolution: %ix%i (%i bpp)", XMSession::instance()->resolutionWidth(), XMSession::instance()->resolutionHeight(), XMSession::instance()->bpp());
    /* */
    
    if(!drawLib->isNoGraphics()) {        
      drawLib->setDrawDims(XMSession::instance()->resolutionWidth(), XMSession::instance()->resolutionHeight(),
			   XMSession::instance()->resolutionWidth(), XMSession::instance()->resolutionHeight());
    }
  }

  /* Init sound system */
  if(v_useGraphics) {
    Sound::init(XMSession::instance());
  }

  /* Init renderer */
  if(v_useGraphics) {
    switchUglyMode(XMSession::instance()->ugly());
    switchTestThemeMode(XMSession::instance()->testTheme());
  }    

  if(v_useGraphics) {
    if(XMSession::instance()->gDebug())
      GameRenderer::instance()->loadDebugInfo(XMSession::instance()->gDebugFile());
  }

  /* database */
  xmDatabase* pDb = xmDatabase::instance("main");
  pDb->init(DATABASE_FILE,
	    XMSession::instance()->profile() == "" ? std::string("") : XMSession::instance()->profile(),
	    FS::getDataDir(), FS::getUserDir(), FS::binCheckSum(),
	    v_useGraphics ? this : NULL);
  if(XMSession::instance()->sqlTrace()) {
    pDb->setTrace(XMSession::instance()->sqlTrace());
  }

  /* load theme */
  if(pDb->themes_isIndexUptodate() == false) {
    ThemeChoicer::initThemesFromDir(pDb);
  }
  try {
    reloadTheme();
  } catch(Exception &e) {
    /* if the theme cannot be loaded, try to reload from files */
    /* perhaps that the xm.db comes from an other computer */
    Logger::Log("** warning ** : Theme cannot be reload, try to update themes into the database");
    ThemeChoicer::initThemesFromDir(pDb);
    reloadTheme();
  }
  
  /* load levels */
  if(pDb->levels_isIndexUptodate() == false) {
      LevelsManager::instance()->reloadLevelsFromLvl(NULL, v_useGraphics ? this : NULL);
  }
  LevelsManager::instance()->reloadExternalLevels(pDb, v_useGraphics ? this : NULL);
  
  /* Update replays */
  if(pDb->replays_isIndexUptodate() == false) {
    initReplaysFromDir();
  }
  
  /* List replays? */  
  if(v_xmArgs.isOptListReplays()) {
    char **v_result;
    unsigned int nrow;
    
    printf("\nReplay                    Level                     Player\n");
    printf("-----------------------------------------------------------------------\n");
    
    v_result = pDb->readDB("SELECT a.name, a.id_profile, b.name "
			    "FROM replays AS a LEFT OUTER JOIN levels AS b "
			    "ON a.id_level = b.id_level;", nrow);
    if(nrow == 0) {
      printf("(none)\n");
    } else {
      std::string v_levelName;

      for(unsigned int i=0; i<nrow; i++) {
	if(pDb->getResult(v_result, 3, i, 2) == NULL) {
	  v_levelName = GAMETEXT_UNKNOWN;
	} else {
	  v_levelName = pDb->getResult(v_result, 3, i, 2);
	}
	printf("%-25s %-25s %-25s\n",
	       pDb->getResult(v_result, 3, i, 0),
	       v_levelName.c_str(),
	       pDb->getResult(v_result, 3, i, 1)
	       );
      }
    }
    pDb->read_DB_free(v_result);
    quit();
    return;
  }
  
  if(v_xmArgs.isOptReplayInfos()) {
    Replay v_replay;
    std::string v_levelId;
      std::string v_player;
    
    v_levelId = v_replay.openReplay(v_xmArgs.getOpt_replayInfos_file(), v_player, true);
    if(v_levelId == "") {
      throw Exception("Invalid replay");
    }
    
    quit();
    return;	
  }
  
  if(v_xmArgs.isOptLevelID()) {
    m_PlaySpecificLevelId = v_xmArgs.getOpt_levelID_id();
  }
  if(v_xmArgs.isOptLevelFile()) {
    m_PlaySpecificLevelFile = v_xmArgs.getOpt_levelFile_file();
  }
  if(v_xmArgs.isOptReplay()) {
    m_PlaySpecificReplay = v_xmArgs.getOpt_replay_file();
  }
 
  /* Should we clean the level cache? (can also be done when disabled) */
  if(v_xmArgs.isOptCleanCache()) {
    LevelsManager::cleanCache();
  }
  
  /* -listlevels? */
  if(v_xmArgs.isOptListLevels()) {
    LevelsManager::instance()->printLevelsList();
    quit();
    return;
  }
  
  /* requires graphics now */
  if(v_useGraphics == false) {
    quit();
    return;
  }
  
  _UpdateLoadingScreen(0, GAMETEXT_INITMENUS);

  /* Load sounds */
  try {
    for(unsigned int i=0; i<Theme::instance()->getSoundsList().size(); i++) {
      Sound::loadSample(Theme::instance()->getSoundsList()[i]->FilePath());
    }
  } catch(Exception &e) {
    Logger::Log("*** Warning *** : %s\n", e.getMsg().c_str());
    /* hum, not cool */
  }
    
  Logger::Log(" %d sound%s loaded",Sound::getNumSamples(),Sound::getNumSamples()==1?"":"s");
    
  /* Find all files in the textures dir and load them */     
  UITexture::setApp(this);
  UIWindow::setDrawLib(getDrawLib());

  /* Initialize renderer */
  GameRenderer::instance()->init(drawLib);
  
  /* build handler */
  InputHandler::instance()->init(&m_Config);
  Replay::enableCompression(XMSession::instance()->compressReplays());
  
  /* load packs */
  LevelsManager::checkPrerequires();
  LevelsManager::instance()->makePacks(XMSession::instance()->profile(), XMSession::instance()->idRoom(), XMSession::instance()->debug());
  
  /* What to do? */
  if(m_PlaySpecificLevelFile != "") {
    try {
      LevelsManager::instance()->addExternalLevel(m_PlaySpecificLevelFile);
      m_PlaySpecificLevelId = LevelsManager::instance()->LevelByFileName(m_PlaySpecificLevelFile);
    } catch(Exception &e) {
      m_PlaySpecificLevelId = m_PlaySpecificLevelFile;
    }
  }
  if((m_PlaySpecificLevelId != "")) {
    /* ======= PLAY SPECIFIC LEVEL ======= */
    StateManager::instance()->pushState(new StatePreplaying(m_PlaySpecificLevelId, false));
    Logger::Log("Playing as '%s'...", XMSession::instance()->profile().c_str());
  }
  else if(m_PlaySpecificReplay != "") {
    /* ======= PLAY SPECIFIC REPLAY ======= */
    StateManager::instance()->pushState(new StateReplaying(m_PlaySpecificReplay));
    }
  else {
    /* display what must be displayed */
    StateMainMenu* pMainMenu = new StateMainMenu();
    StateManager::instance()->pushState(pMainMenu);
    
    /* Do we have a player profile? */
    if(XMSession::instance()->profile() == "") {
      StateManager::instance()->pushState(new StateEditProfile(pMainMenu));
    } 
    
    /* Should we show a notification box? (with important one-time info) */
    if(XMSession::instance()->notifyAtInit()) {
      StateManager::instance()->pushState(new StateMessageBox(NULL, GAMETEXT_NOTIFYATINIT, UI_MSGBOX_OK));
      XMSession::instance()->setNotifyAtInit(false);
    }
  }
  
  if (XMSession::instance()->ugly()){
    drawLib->clearGraphics();
  }
  drawFrame();
  drawLib->flushGraphics();
  
  /* Update stats */
  if(XMSession::instance()->profile() != "") {
    pDb->stats_xmotoStarted(XMSession::instance()->profile());
  }
  
  Logger::Log("UserInit ended at %.3f", GameApp::getXMTime());
}

void GameApp::manageEvent(SDL_Event* Event) {
  int ch=0;
  static int nLastMouseClickX = -100,nLastMouseClickY = -100;
  static int nLastMouseClickButton = -100;
  static float fLastMouseClickTime = 0.0f;
  int nX,nY;
  
  /* What event? */
  switch(Event->type) {
  case SDL_KEYDOWN: 
    if((Event->key.keysym.unicode&0xff80)==0) {
      ch = Event->key.keysym.unicode & 0x7F;
    }
    keyDown(Event->key.keysym.sym, Event->key.keysym.mod, ch);            
    break;
  case SDL_KEYUP: 
    keyUp(Event->key.keysym.sym, Event->key.keysym.mod);            
    break;
  case SDL_QUIT:  
    /* Force quit */
    quit();
    break;
  case SDL_MOUSEBUTTONDOWN:
    /* Pass ordinary click */
    mouseDown(Event->button.button);
    
    /* Is this a double click? */
    getMousePos(&nX,&nY);
    if(nX == nLastMouseClickX &&
       nY == nLastMouseClickY &&
       nLastMouseClickButton == Event->button.button &&
       (getXMTime() - fLastMouseClickTime) < 0.250f) {                
      
      /* Pass double click */
      mouseDoubleClick(Event->button.button);                
    }
    fLastMouseClickTime = getXMTime();
    nLastMouseClickX = nX;
    nLastMouseClickY = nY;
    nLastMouseClickButton = Event->button.button;
    
    break;
  case SDL_MOUSEBUTTONUP:
    mouseUp(Event->button.button);
    break;
    
  case SDL_ACTIVEEVENT:
    
    if((Event->active.state & SDL_APPMOUSEFOCUS) != 0) { // mouse focus
      if(m_hasKeyboardFocus == false) {
	changeFocus(Event->active.gain == 1);
      }
      m_hasMouseFocus = (Event->active.gain == 1);
    }
    
    if((Event->active.state & SDL_APPINPUTFOCUS) != 0) { // keyboard focus
      if(m_hasMouseFocus == false) {
	changeFocus(Event->active.gain == 1);
      }
      m_hasKeyboardFocus = (Event->active.gain == 1);
    }
    
    if((Event->active.state & SDL_APPACTIVE) != 0) {
      changeVisibility(Event->active.gain == 1);
      m_isIconified = (Event->active.gain == 0);
    }
    
  }
}

void GameApp::run_loop() {
  SDL_Event Event;

  while(!m_bQuit) {
    /* Handle SDL events */            
    SDL_PumpEvents();

    // wait on event if xmoto won't be update/rendered
    if(StateManager::instance()->needUpdateOrRender()) {
      while(SDL_PollEvent(&Event)) {
	manageEvent(&Event);
      }
    } else {
      if(SDL_WaitEvent(&Event) == 1) {
	manageEvent(&Event);
      }
      while(SDL_PollEvent(&Event)) {
	manageEvent(&Event);
      }
    }

    /* Update user app */
    drawFrame();

    if(XMSession::instance()->timedemo() == false) {
      _Wait();
    }
  }
}

void GameApp::run_unload() {
  if(m_pWebHighscores != NULL) {
    delete m_pWebHighscores;
  }        

  if(m_pWebLevels != NULL) {
    delete m_pWebLevels;
  }    

  if(GameRenderer::instance() != NULL) {
    GameRenderer::instance()->unprepareForNewLevel(); /* just to be sure, shutdown can happen quite hard */
    GameRenderer::instance()->shutdown();
    InputHandler::instance()->uninit();
  }

  StateManager::destroy();
  
  if(Sound::isInitialized()) {
    Sound::uninit();
  }

  GameRenderer::destroy();
  SysMessage::destroy();  

  if(drawLib != NULL) { /* save config only if drawLib was initialized */
    XMSession::instance()->save(&m_Config);
    InputHandler::instance()->saveConfig(&m_Config);
    m_Config.saveFile();
  }

  if(drawLib != NULL) {
    drawLib->unInit();
  }
    
  if(Logger::isInitialized()) {
    Logger::uninit();
  }

  InputHandler::destroy();
  LevelsManager::destroy();
  Theme::destroy();
  XMSession::destroy();

  /* Shutdown SDL */
  SDL_Quit();
}


void GameApp::_Wait()
{
  if(m_lastFrameTimeStamp < 0){
    m_lastFrameTimeStamp = getXMTimeInt();
  }

  /* Does app want us to delay a bit after the frame? */
  int currentTimeStamp        = getXMTimeInt();
  int currentFrameMinDuration = 1000/StateManager::instance()->getMaxFps();
  int lastFrameDuration       = currentTimeStamp - m_lastFrameTimeStamp;
  // late from the lasts frame is not forget
  int delta = currentFrameMinDuration - (lastFrameDuration + m_frameLate);

  // if we have toooo much late (1/10 second), let reset delta
  int maxDelta = -100;
  if(delta < maxDelta){
    delta = 0;
  }

  if(delta > 0){
    // we're in advance
    // -> sleep
    int beforeSleep = getXMTimeInt();
    SDL_Delay(delta);
    int afterSleep  = getXMTimeInt();
    int sleepTime   = afterSleep - beforeSleep;

    // now that we have sleep, see if we don't have too much sleep
    if(sleepTime >= delta){
      int tooMuchSleep = sleepTime - delta;
      m_frameLate      = tooMuchSleep;
    }
  }
  else{
    // we're late
    // -> update late time
    m_frameLate = (-delta);
  }

  // the sleeping time is not included in the next frame time
  m_lastFrameTimeStamp = getXMTimeInt();
}


  /*===========================================================================
  Update loading screen
  ===========================================================================*/
  void GameApp::_UpdateLoadingScreen(float fDone, const std::string &NextTask) {
    FontManager* v_fm;
    FontGlyph* v_fg;
    int v_border = 3;
    int v_fh;

    getDrawLib()->clearGraphics();
    getDrawLib()->resetGraphics();

    v_fm = getDrawLib()->getFontBig();
    v_fg = v_fm->getGlyph(GAMETEXT_LOADING);
    v_fh = v_fg->realHeight();
    v_fm->printString(v_fg,
		      getDrawLib()->getDispWidth()/2 - 256,
		      getDrawLib()->getDispHeight()/2 - 30,
		      MAKE_COLOR(255,255,255, 255));

    getDrawLib()->drawBox(Vector2f(getDrawLib()->getDispWidth()/2 - 256,
				   getDrawLib()->getDispHeight()/2 - 30),              
			  Vector2f(getDrawLib()->getDispWidth()/2 - 256 + (512.0f*fDone),
				   getDrawLib()->getDispHeight()/2 - 30 + v_border),
			  0,MAKE_COLOR(255,255,255,255));

    getDrawLib()->drawBox(Vector2f(getDrawLib()->getDispWidth()/2 - 256,
				   getDrawLib()->getDispHeight()/2 -30 + v_fh),              
			  Vector2f(getDrawLib()->getDispWidth()/2 - 256 + (512.0f*fDone),
				   getDrawLib()->getDispHeight()/2 - 30 + v_fh + v_border),
			  0,MAKE_COLOR(255,255,255,255));
    
    v_fm = getDrawLib()->getFontSmall();
    v_fg = v_fm->getGlyph(NextTask);
    v_fm->printString(v_fg,
		      getDrawLib()->getDispWidth()/2 - 256,
		      getDrawLib()->getDispHeight()/2 -30 + v_fh + 2,
		      MAKE_COLOR(255,255,255,255));      
    getDrawLib()->flushGraphics();
  }
