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
#include "helpers/Random.h"

#include <curl/curl.h>
#include "XMSession.h"
#include "XMArgs.h"
#include "drawlib/DrawLib.h"
#include "Packager.h"
#include "helpers/SwapEndian.h"
#include "helpers/Text.h"
#include "SysMessage.h"
#include "gui/specific/GUIXMoto.h"
#include "Credits.h"
#include "Replay.h"

#include "states/StateManager.h"
#include "states/StateEditProfile.h"
#include "states/StateReplaying.h"
#include "states/StatePreplayingReplay.h"
#include "states/StatePreplayingGame.h"
#include "states/StateMainMenu.h"
#include "states/StateMessageBox.h"

#include "UserConfig.h"
#include "Renderer.h"
#include "net/thread/ServerThread.h"
#include "net/NetClient.h"
#include <SDL_net.h>

#define MOUSE_DBCLICK_TIME 0.250f

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
      LogError((std::string("Exception: ") + e.getMsg()).c_str());
    }    

    printf("fatal exception : %s\n", e.getMsg().c_str());        
    SDL_Quit(); /* make sure SDL shuts down gracefully */
    
#if defined(WIN32)
    char cBuf[1024];
    snprintf(cBuf, 1024, "Fatal exception occured: %s\n"
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

  /* overwrite default by command line */
  if(v_xmArgs.isOptDefaultTheme()) {
    XMDefault::DefaultTheme = v_xmArgs.getOpt_defaultTheme_theme();
  }
  /**/

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
  XMSession::createDefaultConfig(m_userConfig);
  m_userConfig->loadFile();
  XMSession::instance()->load(m_userConfig); /* overload default session by userConfig */
  XMSession::instance()->load(&v_xmArgs); /* overload default session by xmargs     */
  Logger::setVerbose(XMSession::instance()->isVerbose()); /* apply verbose mode */

  LogInfo(std::string("X-Moto " + XMBuild::getVersionString(true)).c_str());
  LogInfo("compiled at "__DATE__" "__TIME__);
  if(SwapEndian::bigendien) {
    LogInfo("Systeme is bigendien");
  } else {
    LogInfo("Systeme is littleendien");
  }
  LogInfo("User directory: %s", FS::getUserDir().c_str());
  LogInfo("Data directory: %s", FS::getDataDir().c_str());

  initNetwork();

  if(v_xmArgs.isOptListLevels() || v_xmArgs.isOptListReplays() || v_xmArgs.isOptReplayInfos()) {
    v_useGraphics = false;
  }

  // init not so random numbers
  NotSoRandom::init();

  _InitWin(v_useGraphics);

  /* Init the window */
  if(v_useGraphics) {
    /* init drawLib */
    drawLib = DrawLib::DrawLibFromName(XMSession::instance()->drawlib());

    if(drawLib == NULL) {
      throw Exception("Drawlib not initialized");
    }

    SysMessage::instance()->setDrawLib(drawLib);
    
    drawLib->setNoGraphics(v_useGraphics == false);
    drawLib->setDontUseGLExtensions(XMSession::instance()->glExts() == false);

    drawLib->init(XMSession::instance()->resolutionWidth(), XMSession::instance()->resolutionHeight(), XMSession::instance()->bpp(), XMSession::instance()->windowed());
    /* drawlib can change the final resolution if it fails, then, reinit session one's */
    XMSession::instance()->setResolutionWidth(drawLib->getDispWidth());
    XMSession::instance()->setResolutionHeight(drawLib->getDispHeight());
    XMSession::instance()->setBpp(drawLib->getDispBPP());
    XMSession::instance()->setWindowed(drawLib->getWindowed());
    LogInfo("Resolution: %ix%i (%i bpp)", XMSession::instance()->resolutionWidth(), XMSession::instance()->resolutionHeight(), XMSession::instance()->bpp());
    /* */
    
    if(!drawLib->isNoGraphics()) {        
      drawLib->setDrawDims(XMSession::instance()->resolutionWidth(), XMSession::instance()->resolutionHeight(),
			   XMSession::instance()->resolutionWidth(), XMSession::instance()->resolutionHeight());
    }
  }

  /* database -- require drawlib initialized */
  /* thus, resolution/bpp/windowed parameters cannot be stored in the db (or some minor modifications are required) */
  xmDatabase* pDb = xmDatabase::instance("main");
  pDb->init(DATABASE_FILE,
	    XMSession::instance()->profile() == "" ? std::string("") : XMSession::instance()->profile(),
	    FS::getDataDir(), FS::getUserDir(), FS::binCheckSum(),
	    v_xmArgs.isOptNoDBDirsCheck() == false,
	    v_useGraphics ? this : NULL);
  if(XMSession::instance()->sqlTrace()) {
    pDb->setTrace(XMSession::instance()->sqlTrace());
  }
  XMSession::instance()->loadProfile(XMSession::instance()->profile(), pDb);
  XMSession::instance()->load(&v_xmArgs); /* overload default session by xmargs     */
  LogInfo("SiteKey: %s", XMSession::instance()->sitekey().c_str());

#ifdef USE_GETTEXT
  std::string v_locale = Locales::init(XMSession::instance()->language());
  LogInfo("Locales set to '%s' (directory '%s')", v_locale.c_str(), LOCALESDIR);
#endif

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

  /* load theme */
  if(pDb->themes_isIndexUptodate() == false) {
    ThemeChoicer::initThemesFromDir(pDb);
  }
  try {
    reloadTheme();
  } catch(Exception &e) {
    /* if the theme cannot be loaded, try to reload from files */
    /* perhaps that the xm.db comes from an other computer */
    LogWarning("Theme cannot be reload, try to update themes into the database");
    ThemeChoicer::initThemesFromDir(pDb);
    reloadTheme();
  }
  
  /* load levels */
  if(pDb->levels_isIndexUptodate() == false) {
      LevelsManager::instance()->reloadLevelsFromLvl(pDb, v_useGraphics ? this : NULL);
  }
  LevelsManager::instance()->reloadExternalLevels(pDb, v_useGraphics ? this : NULL);
  
  /* Update replays */
  if(pDb->replays_isIndexUptodate() == false) {
    initReplaysFromDir(pDb);
  }
  
  /* List replays? */  
  if(v_xmArgs.isOptListReplays()) {
    pDb->replays_print();
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
  
  /* Should we clean non www levels ? */
  if(v_xmArgs.isOptCleanNoWWWLevels()) {
    pDb->levels_cleanNoWWWLevels();
  }  

  /* -listlevels? */
  if(v_xmArgs.isOptListLevels()) {
    LevelsManager::instance()->printLevelsList(xmDatabase::instance("main"));
    quit();
    return;
  }
  
  /* requires graphics now */
  if(v_useGraphics == false) {
    quit();
    return;
  }
  
  _UpdateLoadingScreen();

  /* Find all files in the textures dir and load them */     
  UITexture::setApp(this);
  UIWindow::setDrawLib(getDrawLib());

  // init physics
  // if(dInitODE2(0) == 0) { /* erreur */} ; // ode 0.10
  dInitODE();

  /* Initialize renderer */
  GameRenderer::instance()->init(drawLib);
  
  /* build handler */
  InputHandler::instance()->init(m_userConfig, pDb, XMSession::instance()->profile(), XMSession::instance()->enableJoysticks());
  Replay::enableCompression(XMSession::instance()->compressReplays());
  
  /* load packs */
  LevelsManager::checkPrerequires();
  LevelsManager::instance()->makePacks(XMSession::instance()->profile(), XMSession::instance()->idRoom(0), XMSession::instance()->debug(), xmDatabase::instance("main"));

  /* Update stats */
  if(XMSession::instance()->profile() != "") {
    pDb->stats_xmotoStarted(XMSession::instance()->sitekey(), XMSession::instance()->profile());
  }

  /* try to not run sql at the same time you enter in the main menu (a thread to compute packs is run (concurrency)) */

  /* What to do? */
  if(m_PlaySpecificLevelFile != "") {
    try {
      LevelsManager::instance()->addExternalLevel(m_PlaySpecificLevelFile, xmDatabase::instance("main"));
      m_PlaySpecificLevelId = LevelsManager::instance()->LevelByFileName(m_PlaySpecificLevelFile, xmDatabase::instance("main"));
    } catch(Exception &e) {
      m_PlaySpecificLevelId = m_PlaySpecificLevelFile;
    }
  }
  if((m_PlaySpecificLevelId != "")) {
    /* ======= PLAY SPECIFIC LEVEL ======= */
    StateManager::instance()->pushState(new StatePreplayingGame(m_PlaySpecificLevelId, false));
    LogInfo("Playing as '%s'...", XMSession::instance()->profile().c_str());
  }
  else if(m_PlaySpecificReplay != "") {
    /* ======= PLAY SPECIFIC REPLAY ======= */
    StateManager::instance()->pushState(new StatePreplayingReplay(m_PlaySpecificReplay, false));
    }
  else {
    /* display what must be displayed */
    StateManager::instance()->pushState(new StateMainMenu());
  }

  LogInfo("UserInit ended at %.3f", GameApp::getXMTime());
}

void GameApp::manageEvent(SDL_Event* Event) {
  static int nLastMouseClickX = -100,nLastMouseClickY = -100;
  static int nLastMouseClickButton = -100;
  static float fLastMouseClickTime = 0.0f;
  int nX,nY;
  std::string utf8Char;  

  if(Event->type == SDL_KEYDOWN || Event->type == SDL_KEYUP) {
    /* don't allow simple modifier key */
    if(Event->key.keysym.sym == SDLK_RSHIFT ||
       Event->key.keysym.sym == SDLK_LSHIFT ||
       Event->key.keysym.sym == SDLK_RCTRL  ||
       Event->key.keysym.sym == SDLK_LCTRL  ||
       Event->key.keysym.sym == SDLK_RALT   ||
       Event->key.keysym.sym == SDLK_LALT   ||
       Event->key.keysym.sym == SDLK_RMETA  ||
       Event->key.keysym.sym == SDLK_LMETA
       ) {
      return;
    }
  }

  /* What event? */
  switch(Event->type) {
  case SDL_KEYDOWN:
    utf8Char = unicode2utf8(Event->key.keysym.unicode);
    StateManager::instance()->xmKey(INPUT_DOWN, XMKey(Event->key.keysym.sym, Event->key.keysym.mod, utf8Char));
    break;
  case SDL_KEYUP:
    utf8Char = unicode2utf8(Event->key.keysym.unicode);
    StateManager::instance()->xmKey(INPUT_UP, XMKey(Event->key.keysym.sym, Event->key.keysym.mod, utf8Char));
    break;
  case SDL_QUIT:  
    /* Force quit */
    quit();
    break;
  case SDL_MOUSEBUTTONDOWN:    
    /* Is this a double click? */
    getMousePos(&nX,&nY);
    if(nX == nLastMouseClickX &&
       nY == nLastMouseClickY &&
       nLastMouseClickButton == Event->button.button &&
       (getXMTime() - fLastMouseClickTime) < MOUSE_DBCLICK_TIME) {
      
      /* Pass double click */
      StateManager::instance()->xmKey(INPUT_DOWN, XMKey(Event->button.button, 2));
    } else {
      /* Pass ordinary click */
      StateManager::instance()->xmKey(INPUT_DOWN, XMKey(Event->button.button));
    }
    fLastMouseClickTime = getXMTime();
    nLastMouseClickX = nX;
    nLastMouseClickY = nY;
    nLastMouseClickButton = Event->button.button;
    
    break;
  case SDL_MOUSEBUTTONUP:
    StateManager::instance()->xmKey(INPUT_UP, XMKey(Event->button.button));
    break;

  case SDL_JOYAXISMOTION:
  StateManager::instance()->xmKey(InputHandler::instance()->joystickAxisSens(Event->jaxis.value),
				  XMKey(InputHandler::instance()->getJoyId(Event->jbutton.which),
					Event->jaxis.axis, Event->jaxis.value));
  break;
  
  case SDL_JOYBUTTONDOWN:
  StateManager::instance()->xmKey(INPUT_DOWN, XMKey(InputHandler::instance()->getJoyId(Event->jbutton.which), Event->jbutton.button));
  break;
  
  case SDL_JOYBUTTONUP:
  StateManager::instance()->xmKey(INPUT_UP, XMKey(InputHandler::instance()->getJoyId(Event->jbutton.which), Event->jbutton.button));
  break;
  
  case SDL_ACTIVEEVENT:
    
    if((Event->active.state & SDL_APPMOUSEFOCUS) != 0) { // mouse focus
      if(m_hasKeyboardFocus == false) {
	StateManager::instance()->changeFocus(Event->active.gain == 1);
      }
      m_hasMouseFocus = (Event->active.gain == 1);
    }
    
    if((Event->active.state & SDL_APPINPUTFOCUS) != 0) { // keyboard focus
      if(m_hasMouseFocus == false) {
	StateManager::instance()->changeFocus(Event->active.gain == 1);
      }
      m_hasKeyboardFocus = (Event->active.gain == 1);
    }
    
    if((Event->active.state & SDL_APPACTIVE) != 0) {
      StateManager::instance()->changeVisibility(Event->active.gain == 1);
      m_isIconified = (Event->active.gain == 0);
    }
  }
}

void GameApp::run_loop() {
  SDL_Event Event;

  while(m_bQuit == false) {
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
  if(Logger::isInitialized()) {
    LogInfo("UserUnload started at %.3f", GameApp::getXMTime());
  }

  if(m_pWebHighscores != NULL) {
    delete m_pWebHighscores;
  }        

  if(m_pWebLevels != NULL) {
    delete m_pWebLevels;
  }    

  if(GameRenderer::instance() != NULL) {
    GameRenderer::instance()->unprepareForNewLevel(); /* just to be sure, shutdown can happen quite hard */
    GameRenderer::instance()->shutdown();
    InputHandler::instance()->uninit(); // uinit the input, but you can still save the config
  }

  StateManager::destroy();
  
  if(Sound::isInitialized()) {
    Sound::uninit();
  }

  dCloseODE(); // uninit ODE

  uninitNetwork();

  GameRenderer::destroy();
  SysMessage::destroy();  

  if(Logger::isInitialized()) {
    LogDebug("UserUnload saveConfig at %.3f", GameApp::getXMTime());
  }
  if(drawLib != NULL) { /* save config only if drawLib was initialized */
    XMSession::instance()->save(m_userConfig, xmDatabase::instance("main"));
    InputHandler::instance()->saveConfig(m_userConfig, xmDatabase::instance("main"), XMSession::instance()->profile());
    m_userConfig->saveFile();
  }

  if(Logger::isInitialized()) {
    LogDebug("UserUnload saveConfig ended at %.3f", GameApp::getXMTime());
  }

  if(drawLib != NULL) {
    drawLib->unInit();
  }

  InputHandler::destroy();
  LevelsManager::destroy();
  Theme::destroy();
  XMSession::destroy();

  if(Logger::isInitialized()) {
    LogInfo("UserUnload ended at %.3f", GameApp::getXMTime());
  }

  /* Shutdown SDL */
  SDL_Quit();
    
  if(Logger::isInitialized()) {
    Logger::uninit();
  }

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
  void GameApp::_UpdateLoadingScreen(const std::string &NextTask) {
    FontManager* v_fm;
    FontGlyph* v_fg;
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
    
    if(NextTask != "") {
      v_fm = getDrawLib()->getFontSmall();
      v_fg = v_fm->getGlyph(NextTask);
      v_fm->printString(v_fg,
			getDrawLib()->getDispWidth()/2 - 256,
			getDrawLib()->getDispHeight()/2 -30 + v_fh + 2,
			MAKE_COLOR(255,255,255,255));      
    }
    getDrawLib()->flushGraphics();
  }


void GameApp::initNetwork() {
  if(SDLNet_Init()==-1) {
    throw Exception(SDLNet_GetError());
  }

  // start server
  if(XMSession::instance()->serverStartAtStartup() || true) {
    m_serverThread = new ServerThread();
    m_serverThread->startThread();
  }

  // start client
  SDL_Delay(1000); // wait 1s for tests only
  try {
    NetClient::instance()->connect("127.0.0.1", XMSession::instance()->clientServerPort());
  } catch(Exception &e) {
    LogError("Unable to connect to the server");
  }
}

void GameApp::uninitNetwork() {
  // stop the client
  if(NetClient::instance()->isConnected()) {
    NetClient::instance()->disconnect();
  }
  NetClient::destroy();

  // stop the server
  if(m_serverThread != NULL) {
    if(m_serverThread->isThreadRunning()) {
      m_serverThread->askThreadToEnd();
      m_serverThread->waitForThreadEnd();
    }
    delete m_serverThread;
  }

  SDLNet_Quit();
}
