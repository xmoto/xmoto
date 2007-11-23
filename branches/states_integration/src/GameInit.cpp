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
#include "db/xmDatabase.h";
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

#include "states/StateManager.h"
#include "states/StateEditProfile.h"
#include "states/StateReplaying.h"
#include "states/StatePreplaying.h"
#include "states/StateMainMenu.h"
#include "states/StateMessageBox.h"

#define DATABASE_FILE FS::getUserDirUTF8() + "/" + "xm.db"

#if defined(WIN32)
int SDL_main(int nNumArgs,char **ppcArgs) {
#else
int main(int nNumArgs,char **ppcArgs) {
#endif
  /* Start application */
  try {     
    /* Setup basic info */
    GameApp vapp;

    vapp.run(nNumArgs,ppcArgs);
  }
  catch (Exception &e) {
    if(Logger::isInitialized()) {
      Logger::Log((std::string("Exception: ") + e.getMsg()).c_str());
    }    

    printf("fatal exception : %s\n",e.getMsg().c_str());        
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

  void GameApp::run(int nNumArgs,char **ppcArgs) {
    XMArguments v_xmArgs;
    GameState* pState;

    /* check args */
    try {
      v_xmArgs.parse(nNumArgs, ppcArgs);
    } catch (Exception &e) {
      printf("syntax error : %s\n", e.getMsg().c_str());
      v_xmArgs.help(nNumArgs >= 1 ? ppcArgs[0] : "xmoto");
      return; /* abort */
    }

    /* help */
    if(v_xmArgs.isOptHelp()) {
      v_xmArgs.help(nNumArgs >= 1 ? ppcArgs[0] : "xmoto");
      return;
    }

    /* init sub-systems */
    SwapEndian::Swap_Init();
    srand(time(NULL));

    /* package / unpackage */
    if(v_xmArgs.isOptPack()) {
      Packager::go(v_xmArgs.getOpt_pack_bin() == "" ? "xmoto.bin" : v_xmArgs.getOpt_pack_bin(),
		   v_xmArgs.getOpt_pack_dir() == "" ? "."         : v_xmArgs.getOpt_pack_dir());
      return;
    }
    if(v_xmArgs.isOptUnPack()) {
      Packager::goUnpack(v_xmArgs.getOpt_unpack_bin() == "" ? "xmoto.bin" : v_xmArgs.getOpt_unpack_bin(),
			 v_xmArgs.getOpt_unpack_dir() == "" ? "."         : v_xmArgs.getOpt_unpack_dir(),
			 v_xmArgs.getOpt_unpack_noList() == false);
      return;
    }
    /* ***** */

    if(v_xmArgs.isOptConfigPath()) {
      FS::init("xmoto", "xmoto.bin", "xmoto.log", v_xmArgs.getOpt_configPath_path());
    } else {
      FS::init("xmoto", "xmoto.bin", "xmoto.log");
    }
    Logger::init(FS::getUserDir() + "/xmoto.log");

    /* load config file */
    createDefaultConfig();
    m_Config.loadFile();

    /* load session */
    m_xmsession->load(&m_Config); /* overload default session by userConfig */
    m_xmsession->load(&v_xmArgs); /* overload default session by xmargs     */

    /* apply verbose mode */
    Logger::setVerbose(m_xmsession->isVerbose());

#ifdef USE_GETTEXT
    std::string v_locale = Locales::init(m_Config.getString("Language"));
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
      m_xmsession->setUseGraphics(false);
    }

    _InitWin(m_xmsession->useGraphics());

    if(m_xmsession->useGraphics()) {
      /* init drawLib */
      drawLib = DrawLib::DrawLibFromName(m_xmsession->drawlib());

      if(drawLib == NULL) {
	throw Exception("Drawlib not initialized");
      }

      m_Renderer = new GameRenderer(drawLib);
      m_Renderer->setTheme(&m_theme);
      m_MotoGame.setRenderer(m_Renderer);
      m_sysMsg = new SysMessage(drawLib);

      drawLib->setNoGraphics(m_xmsession->useGraphics() == false);
      drawLib->setDontUseGLExtensions(m_xmsession->glExts() == false);

      /* Init! */
      drawLib->init(m_xmsession->resolutionWidth(), m_xmsession->resolutionHeight(), m_xmsession->bpp(), m_xmsession->windowed(), &m_theme);
      /* drawlib can change the final resolution if it fails, then, reinit session one's */
      m_xmsession->setResolutionWidth(drawLib->getDispWidth());
      m_xmsession->setResolutionHeight(drawLib->getDispHeight());
      m_xmsession->setBpp(drawLib->getDispBPP());
      m_xmsession->setWindowed(drawLib->getWindowed());
      Logger::Log("Resolution: %ix%i (%i bpp)", m_xmsession->resolutionWidth(), m_xmsession->resolutionHeight(), m_xmsession->bpp());
      /* */

      if(!drawLib->isNoGraphics()) {        
	drawLib->setDrawDims(m_xmsession->resolutionWidth(), m_xmsession->resolutionHeight(),
			     m_xmsession->resolutionWidth(), m_xmsession->resolutionHeight());
      }
    }

    /* Now perform user init */
    userInit(&v_xmArgs);

    /* Enter the main loop */
    while(!m_bQuit) {
      /* Handle SDL events */            
      SDL_PumpEvents();
        
      SDL_Event Event;
      while(SDL_PollEvent(&Event)) {
	int ch=0;
	static int nLastMouseClickX = -100,nLastMouseClickY = -100;
	static int nLastMouseClickButton = -100;
	static float fLastMouseClickTime = 0.0f;
	int nX,nY;

	/* What event? */
	switch(Event.type) {
	case SDL_KEYDOWN: 
	  if((Event.key.keysym.unicode&0xff80)==0) {
	    ch = Event.key.keysym.unicode & 0x7F;
	  }
	  keyDown(Event.key.keysym.sym, Event.key.keysym.mod, ch);            
	  break;
	case SDL_KEYUP: 
	  keyUp(Event.key.keysym.sym, Event.key.keysym.mod);            
	  break;
	case SDL_QUIT:  
	  /* Force quit */
	  quit();
	  break;
	case SDL_MOUSEBUTTONDOWN:
	  /* Pass ordinary click */
	  mouseDown(Event.button.button);
              
	  /* Is this a double click? */
	  getMousePos(&nX,&nY);
	  if(nX == nLastMouseClickX &&
	     nY == nLastMouseClickY &&
	     nLastMouseClickButton == Event.button.button &&
	     (getXMTime() - fLastMouseClickTime) < 0.250f) {                
	    
	    /* Pass double click */
	    mouseDoubleClick(Event.button.button);                
	  }
	  fLastMouseClickTime = getXMTime();
	  nLastMouseClickX = nX;
	  nLastMouseClickY = nY;
	  nLastMouseClickButton = Event.button.button;
	  
	  break;
	case SDL_MOUSEBUTTONUP:
	  mouseUp(Event.button.button);
	  break;
	}

      }

      /* Update user app */
      drawFrame();

       _Wait();
    }
    
    /* Shutdown */
    _Uninit();
  }

void GameApp::_Wait()
{
  if(m_lastFrameTimeStamp < 0){
    m_lastFrameTimeStamp = getXMTimeInt();
  }

  /* Does app want us to delay a bit after the frame? */
  int currentTimeStamp        = getXMTimeInt();
  int currentFrameMinDuration = 1000/m_stateManager->getMaxFps();
  int lastFrameDuration       = currentTimeStamp - m_lastFrameTimeStamp;
  // late from the lasts frame is not forget
  int delta = currentFrameMinDuration - (lastFrameDuration + m_frameLate);

  if(delta > 0){
    // we're in advance
    // -> sleep
    int beforeSleep = getXMTimeInt();
    SDL_Delay(delta);
    int afterSleep  = getXMTimeInt();
    int sleepTime   = afterSleep - beforeSleep;

    // now that we have sleep, see if we don't have too much sleep
    if(sleepTime > delta){
      int tooMuchSleep = sleepTime - delta;
      m_frameLate      = tooMuchSleep;
    } else{
      m_frameLate = 0;
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
  
  /*===========================================================================
  Initialize game
  ===========================================================================*/
  void GameApp::userInit(XMArguments* v_xmArgs) {
    /* Reset timers */
    m_fLastPerfStateTime = 0.0f;
    
    /* And stuff */
    m_nJustDeadShade = 0;
    
    /* Init some config */
    _UpdateSettings();

    /* Init sound system */
    Sound::init(m_xmsession);
      
    /* Init renderer */
    if(m_xmsession->useGraphics()) {
      switchUglyMode(m_xmsession->ugly());
      switchTestThemeMode(m_xmsession->testTheme());
      m_Renderer->setParent( (GameApp *)this );
      m_Renderer->setGameObject( &m_MotoGame );        
      m_Renderer->setDebug(m_xmsession->debug());

      m_Renderer->setGhostMotionBlur(m_xmsession->ghostMotionBlur());
    }    

    /* Tell collision system whether we want debug-info or not */
    m_MotoGame.getCollisionHandler()->setDebug(m_xmsession->debug());
    
    /* Data time! */
    Logger::Log("Loading data...");

    if(m_xmsession->useGraphics()) {
      if(m_xmsession->gDebug()) m_Renderer->loadDebugInfo(m_xmsession->gDebugFile());
    }

    /* database */
    m_db = new xmDatabase(DATABASE_FILE,
			  m_xmsession->profile() == "" ? std::string("") : m_xmsession->profile(),
			  FS::getDataDir(), FS::getUserDir(), FS::binCheckSum(),
			  m_xmsession->useGraphics() ? this : NULL);
    if(m_xmsession->sqlTrace()) {
      m_db->setTrace(m_xmsession->sqlTrace());
    }
    /* set the room name ; set to WR if it cannot be determined */
    m_WebHighscoresRoomName = "WR";
    char **v_result;
    unsigned int nrow;
    v_result = m_db->readDB("SELECT name "
			    "FROM webrooms "
			    "WHERE id_room=" + m_xmsession->idRoom() + ";",
			    nrow);
    if(nrow == 1) {
      m_WebHighscoresRoomName = m_db->getResult(v_result, 1, 0, 0);
    }
    m_db->read_DB_free(v_result);

    /* load theme */
    m_themeChoicer = new ThemeChoicer();
    if(m_db->themes_isIndexUptodate() == false) {
      m_themeChoicer->initThemesFromDir(m_db);
    }
    try {
      reloadTheme();
    } catch(Exception &e) {
      /* if the theme cannot be loaded, try to reload from files */
      /* perhaps that the xm.db comes from an other computer */
      Logger::Log("** warning ** : Theme cannot be reload, try to update themes into the database");
      m_themeChoicer->initThemesFromDir(m_db);
      reloadTheme();
    }

    /* load levels */
    if(m_db->levels_isIndexUptodate() == false) {
      m_levelsManager.reloadLevelsFromLvl(m_db, m_xmsession->useGraphics() ? this : NULL);
    }
    m_levelsManager.reloadExternalLevels(m_db, m_xmsession->useGraphics() ? this : NULL);

    /* Update replays */
    if(m_db->replays_isIndexUptodate() == false) {
      initReplaysFromDir();
    }

    /* List replays? */  
    if(v_xmArgs->isOptListReplays()) {
      char **v_result;
      unsigned int nrow;

      printf("\nReplay                    Level                     Player\n");
      printf("-----------------------------------------------------------------------\n");

      v_result = m_db->readDB("SELECT a.name, a.id_profile, b.name "
			      "FROM replays AS a INNER JOIN levels AS b "
			      "ON a.id_level = b.id_level;", nrow);
      if(nrow == 0) {
	printf("(none)\n");
      } else {
	for(unsigned int i=0; i<nrow; i++) {
	  //m_db->getResult(v_result, 4, i, 0)
	  printf("%-25s %-25s %-25s\n",
		 m_db->getResult(v_result, 3, i, 0),
		 m_db->getResult(v_result, 3, i, 2),
		 m_db->getResult(v_result, 3, i, 1)
		 );
	}
      }
      m_db->read_DB_free(v_result);
      quit();
      return;
    }
    
    if(v_xmArgs->isOptReplayInfos()) {
      Replay v_replay;
      std::string v_levelId;
      std::string v_player;
      
      v_levelId = v_replay.openReplay(v_xmArgs->getOpt_replayInfos_file(), v_player, true);
      if(v_levelId == "") {
	throw Exception("Invalid replay");
      }
      
      quit();
      return;	
    }
    
    if(v_xmArgs->isOptLevelID()) {
      m_PlaySpecificLevelId = v_xmArgs->getOpt_levelID_id();
    }
    if(v_xmArgs->isOptLevelFile()) {
      m_PlaySpecificLevelFile = v_xmArgs->getOpt_levelFile_file();
    }
    if(v_xmArgs->isOptReplay()) {
      m_PlaySpecificReplay = v_xmArgs->getOpt_replay_file();
    }

    if(m_xmsession->useGraphics()) {  
      _UpdateLoadingScreen((1.0f/9.0f) * 0,GAMETEXT_LOADINGSOUNDS);
      
      /* Load sounds */
      try {
	for(unsigned int i=0; i<m_theme.getSoundsList().size(); i++) {
	  Sound::loadSample(m_theme.getSoundsList()[i]->FilePath());
	}
      } catch(Exception &e) {
	Logger::Log("*** Warning *** : %s\n", e.getMsg().c_str());
	/* hum, not cool */
      }
	
      Logger::Log(" %d sound%s loaded",Sound::getNumSamples(),Sound::getNumSamples()==1?"":"s");

      /* Find all files in the textures dir and load them */     
      UITexture::setApp(this);
      UIWindow::setDrawLib(getDrawLib());

      _UpdateLoadingScreen((1.0f/9.0f) * 2,GAMETEXT_LOADINGMENUGRAPHICS);
    }
        
    /* Should we clean the level cache? (can also be done when disabled) */
    if(v_xmArgs->isOptCleanCache()) {
      LevelsManager::cleanCache();
    }

    /* -listlevels? */
    if(v_xmArgs->isOptListLevels()) {
      m_levelsManager.printLevelsList(m_db);
     quit();
     return;
    }
        
    /* requires graphics now */
    if(m_xmsession->useGraphics() == false) {
      return;
    }

    /* Initialize renderer */
    _UpdateLoadingScreen((1.0f/9.0f) * 6,GAMETEXT_INITRENDERER);
    m_Renderer->init();

    /* build handler */
    m_InputHandler.init(&m_Config);
    
    /* load packs */
    LevelsManager::checkPrerequires();
    m_levelsManager.makePacks(m_db,
			      m_xmsession->profile(),
			      m_xmsession->idRoom(),
			      m_xmsession->debug());     

    /* Initialize menu system */
    _InitMenus();

    /* What to do? */
    if(m_PlaySpecificLevelFile != "") {
      try {
	m_levelsManager.addExternalLevel(m_db, m_PlaySpecificLevelFile);
	m_PlaySpecificLevelId = m_levelsManager.LevelByFileName(m_db, m_PlaySpecificLevelFile);
      } catch(Exception &e) {
	m_PlaySpecificLevelId = m_PlaySpecificLevelFile;
      }
    }
    if((m_PlaySpecificLevelId != "")) {
      /* ======= PLAY SPECIFIC LEVEL ======= */
      StatePreplaying::setPlayAnimation(true);
      m_stateManager->pushState(new StatePreplaying(this, m_PlaySpecificLevelId));
      Logger::Log("Playing as '%s'...", m_xmsession->profile().c_str());
    }
    else if(m_PlaySpecificReplay != "") {
      /* ======= PLAY SPECIFIC REPLAY ======= */
      m_stateManager->pushState(new StateReplaying(this, m_PlaySpecificReplay));
    }
    else {
      /* final initialisation */
      Logger::Log("UserPreInit ended at %.3f", GameApp::getXMTime());

      /* display what must be displayed */
      StateMainMenu* pMainMenu = new StateMainMenu(this);
      m_stateManager->pushState(pMainMenu);

      /* Do we have a player profile? */
      if(m_xmsession->profile() == "") {
	m_stateManager->pushState(new StateEditProfile(this, pMainMenu));
      } 
 
      /* Should we show a notification box? (with important one-time info) */
      if(m_Config.getBool("NotifyAtInit")) {
	m_stateManager->pushState(new StateMessageBox(NULL, this, GAMETEXT_NOTIFYATINIT, UI_MSGBOX_OK));
	m_Config.setBool("NotifyAtInit", false); 
      }
    }

    if (isUglyMode()){
      drawLib->clearGraphics();
    }
    drawFrame();
    drawLib->flushGraphics();

    /* final initialisation so that xmoto seems to be loaded fastly */

    /* Update stats */
    if(m_xmsession->profile() != "") {
      m_db->stats_xmotoStarted(m_xmsession->profile());
    }

    Logger::Log("UserInit ended at %.3f", GameApp::getXMTime());
  }

  /*===========================================================================
  Shutdown game
  ===========================================================================*/
  void GameApp::userShutdown(void) {  
    if(m_pWebHighscores != NULL)
    delete m_pWebHighscores;
        
    if(m_pWebLevels != NULL)
    delete m_pWebLevels;
    
    if(m_xmsession->useGraphics()) {
      m_Renderer->unprepareForNewLevel(); /* just to be sure, shutdown can happen quite hard */
      m_Renderer->shutdown();
      m_InputHandler.uninit();
    }
    
    delete m_themeChoicer;
    
    if(m_pJustPlayReplay != NULL)
    delete m_pJustPlayReplay;
    
    m_xmsession->save(&m_Config);
    m_InputHandler.saveConfig(&m_Config);

    Sound::uninit();

    delete m_Renderer;
    delete m_sysMsg;

    m_Config.saveFile();
  }  
  
  /*===========================================================================
  Create the default config
  ===========================================================================*/
  void GameApp::createDefaultConfig(void) {
    m_Config.createVar( "Language" , "");
    m_Config.createVar( "Theme",                  THEME_DEFAULT_THEMENAME);    

    /* Display */
    m_Config.createVar( "DisplayWidth",           "800" );
    m_Config.createVar( "DisplayHeight",          "600" );
    m_Config.createVar( "DisplayBPP",             "32" );
    m_Config.createVar( "DisplayWindowed",        "false" );
    m_Config.createVar( "MenuGraphics",           "High" );
    m_Config.createVar( "GameGraphics",           "High" );
    m_Config.createVar( "DrawLib",                "OPENGL" );
        
    /* Audio */
    m_Config.createVar( "AudioEnable",            "true" );
    m_Config.createVar( "AudioSampleRate",        "22050" );
    m_Config.createVar( "AudioSampleBits",        "16" );
    m_Config.createVar( "AudioChannels",          "Mono" );
    m_Config.createVar( "EngineSoundEnable",      "true" );

    /* Controls */
    m_Config.createVar( "ControllerMode1",        "Keyboard" );
    m_Config.createVar( "KeyDrive1",              "Up" );
    m_Config.createVar( "KeyBrake1",              "Down" );
    m_Config.createVar( "KeyFlipLeft1",           "Left" );
    m_Config.createVar( "KeyFlipRight1",          "Right" );
    m_Config.createVar( "KeyChangeDir1",          "Space" );
    m_Config.createVar( "ControllerMode2",        "Keyboard" );
    m_Config.createVar( "KeyDrive2",              "A" );
    m_Config.createVar( "KeyBrake2",              "Q" );
    m_Config.createVar( "KeyFlipLeft2",           "Z" );
    m_Config.createVar( "KeyFlipRight2",          "E" );
    m_Config.createVar( "KeyChangeDir2",          "W" );
    m_Config.createVar( "ControllerMode3",        "Keyboard" );
    m_Config.createVar( "KeyDrive3",              "R" );
    m_Config.createVar( "KeyBrake3",              "F" );
    m_Config.createVar( "KeyFlipLeft3",           "T" );
    m_Config.createVar( "KeyFlipRight3",          "Y" );
    m_Config.createVar( "KeyChangeDir3",          "V" );
    m_Config.createVar( "ControllerMode4",        "Keyboard" );
    m_Config.createVar( "KeyDrive4",              "Y" );
    m_Config.createVar( "KeyBrake4",              "H" );
    m_Config.createVar( "KeyFlipLeft4",           "U" );
    m_Config.createVar( "KeyFlipRight4",          "I" );
    m_Config.createVar( "KeyChangeDir4",          "N" );
    
    m_Config.createVar( "AutosaveHighscoreReplays", "true");

    #if defined(ENABLE_ZOOMING)
      m_Config.createVar( "KeyZoomIn",              "Pad 7" );
      m_Config.createVar( "KeyZoomOut",             "Pad 9" );
      m_Config.createVar( "KeyZoomInit",            "Home" );
      m_Config.createVar( "KeyCameraMoveXUp",       "Pad 6" );
      m_Config.createVar( "KeyCameraMoveXDown",     "Pad 4" );
      m_Config.createVar( "KeyCameraMoveYUp",       "Pad 8" );
      m_Config.createVar( "KeyCameraMoveYDown",     "Pad 2" );
    #endif
     
    /* joystick */
    m_Config.createVar( "JoyIdx1",                "0" );
    m_Config.createVar( "JoyAxisPrim1",           "1" );
    m_Config.createVar( "JoyAxisPrimMax1",        "32760" );
    m_Config.createVar( "JoyAxisPrimMin1",        "-32760" );
    m_Config.createVar( "JoyAxisPrimUL1",         "1024" );
    m_Config.createVar( "JoyAxisPrimLL1",         "-1024" );
    m_Config.createVar( "JoyAxisSec1",            "0" );
    m_Config.createVar( "JoyAxisSecMax1",         "32760" );
    m_Config.createVar( "JoyAxisSecMin1",         "-32760" );
    m_Config.createVar( "JoyAxisSecUL1",          "1024" );
    m_Config.createVar( "JoyAxisSecLL1",          "-1024" );
    m_Config.createVar( "JoyButtonChangeDir1",    "0" );

    /* Misc */
    m_Config.createVar( "DefaultProfile",         "" );
    m_Config.createVar( "ScreenshotFormat",       "png" );
    m_Config.createVar( "NotifyAtInit",           "true" );
    m_Config.createVar( "ShowMiniMap",            "true" );
    m_Config.createVar( "ShowEngineCounter",      "false" );

    m_Config.createVar( "StoreReplays",           "true" );
    m_Config.createVar( "ReplayFrameRate",        "25" );
    m_Config.createVar( "CompressReplays",        "true" );
    m_Config.createVar( "ContextHelp",            "true" );
    m_Config.createVar( "MenuMusic",              "true" );    
    m_Config.createVar( "InitZoom",               "true" );
    m_Config.createVar( "DeathAnim",              "true" );

    m_Config.createVar( "WebHighscores",            "false" );
    m_Config.createVar( "CheckHighscoresAtStartup", "true" );
    m_Config.createVar( "CheckNewLevelsAtStartup",  "true" );
    m_Config.createVar( "ShowInGameWorldRecord",    "false" );
    m_Config.createVar( "WebConfAtInit",            "true" );
    
    /* Webstuff */
    m_Config.createVar( "WebHighscoresURL",       DEFAULT_WEBHIGHSCORES_URL );
    m_Config.createVar( "WebLevelsURL",           DEFAULT_WEBLEVELS_URL);
    m_Config.createVar( "WebThemesURL",           DEFAULT_WEBTHEMES_URL);
    m_Config.createVar( "WebThemesURLBase",       DEFAULT_WEBTHEMES_SPRITESURLBASE);
    m_Config.createVar( "WebHighscoresIdRoom",     DEFAULT_WEBROOM_ID);

    /* Proxy */
    m_Config.createVar( "ProxyType",              "" ); /* (blank), HTTP, SOCKS4, or SOCKS5 */
    m_Config.createVar( "ProxyServer",            "" ); /* (may include user/pass and port) */
    m_Config.createVar( "ProxyPort",              "-1" );
    m_Config.createVar( "ProxyAuthUser",          "" ); 
    m_Config.createVar( "ProxyAuthPwd",           "" );

    /* auto upload */
    m_Config.createVar( "WebHighscoreUploadURL", DEFAULT_UPLOADREPLAY_URL);
    m_Config.createVar( "WebHighscoreUploadLogin"    , "");
    m_Config.createVar( "WebHighscoreUploadPassword" , ""); 

    m_Config.createVar( "EnableGhost"        , "true");
    m_Config.createVar( "GhostStrategy_MYBEST", "true");
    m_Config.createVar( "GhostStrategy_THEBEST", "false");
    m_Config.createVar( "GhostStrategy_BESTOFROOM", "false");
    m_Config.createVar( "ShowGhostTimeDiff"  , "true");
    m_Config.createVar( "DisplayGhostInfo"   , "false");
    m_Config.createVar( "HideGhosts"   , "false");
    m_Config.createVar( "GhostMotionBlur"    , "true" );

    /* quick start button */
    m_Config.createVar("QSQualityMIN",    "1");
    m_Config.createVar("QSDifficultyMIN", "1");
    m_Config.createVar("QSQualityMAX",    "5");
    m_Config.createVar("QSDifficultyMAX", "5");

    /* multi */
    m_Config.createVar("MultiStopWhenOneFinishes" , "true");
  }

