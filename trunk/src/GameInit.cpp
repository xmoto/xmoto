/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

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
#include "Game.h"
#include "VFileIO.h"
#include "Sound.h"
#include "PhysSettings.h"
#include "Input.h"
#include "db/xmDatabase.h";

  #include <curl/curl.h>

#define DATABASE_FILE vapp::FS::getUserDir() + "/" + "xm.db"

namespace vapp {

  /*===========================================================================
  Pre-initialize game
  ===========================================================================*/
  void GameApp::userPreInit(void) {
    /* Config */
    _CreateDefaultConfig();
    m_Config.loadFile();
  }
  
  /*===========================================================================
  Select display mode
  ===========================================================================*/
  void GameApp::selectDisplayMode(int *pnWidth,int *pnHeight,int *pnBPP,bool *pbWindowed) {
    *pnWidth = m_Config.getInteger("DisplayWidth");
    *pnHeight = m_Config.getInteger("DisplayHeight");
    *pnBPP = m_Config.getInteger("DisplayBPP");
    *pbWindowed = m_Config.getBool("DisplayWindowed");
  }  
  
  std::string GameApp::selectDrawLibMode() {
    return m_Config.getString("DrawLib");
  }

  /*===========================================================================
  Update loading screen
  ===========================================================================*/
  void GameApp::_UpdateLoadingScreen(float fDone,Texture *pLoadingScreen,const std::string &NextTask) {
    if(pLoadingScreen != NULL) {
      getDrawLib()->clearGraphics();
      getDrawLib()->resetGraphics();
      getDrawLib()->drawImage(Vector2f(getDrawLib()->getDispWidth()/2 - 256,getDrawLib()->getDispHeight()/2 - 40),
                Vector2f(getDrawLib()->getDispWidth()/2 + 256,getDrawLib()->getDispHeight()/2 + 40),
                pLoadingScreen,MAKE_COLOR(255,255,255,255));
      getDrawLib()->drawBox(Vector2f(getDrawLib()->getDispWidth()/2 + 256 - (512.0f*(1-fDone)),getDrawLib()->getDispHeight()/2 - 40),              
              Vector2f(getDrawLib()->getDispWidth()/2 + 256,getDrawLib()->getDispHeight()/2 - 25),              
              0,MAKE_COLOR(0,0,0,128));
      getDrawLib()->drawBox(Vector2f(getDrawLib()->getDispWidth()/2 + 256 - (512.0f*(1-fDone)),getDrawLib()->getDispHeight()/2 + 25),              
              Vector2f(getDrawLib()->getDispWidth()/2 + 256,getDrawLib()->getDispHeight()/2 + 40),              
              0,MAKE_COLOR(0,0,0,128));
             
      getDrawLib()->drawText(Vector2f(getDrawLib()->getDispWidth()/2 - 256,getDrawLib()->getDispHeight()/2 + 40 + 3),NextTask);
              
      getDrawLib()->flushGraphics();
    }
  }
  
  /*===========================================================================
  Initialize game
  ===========================================================================*/
  void GameApp::userInit(void) {
    Sprite* pSprite;

    switchUglyMode(m_bUglyMode);
    switchTestThemeMode(m_bTestThemeMode);

    /* Reset timers */
    m_fLastFrameTime = 0.0f;
    m_fLastPerfStateTime = 0.0f;
    m_fLastPhysTime = getTime() - PHYS_STEP_SIZE;
    
    /* And stuff */
    m_nPauseShade = 0;
    m_nJustDeadShade = 0;
    m_nFinishShade = 0;
    
    /* Init some config */
    _UpdateSettings();
    
    /* load theme */
    m_themeChoicer = new ThemeChoicer(
              this,
              &m_ProxySettings
              );
    reloadTheme();

    /* Select profile */
    m_profile = "";
    if(m_ForceProfile != "") {
      m_profile = m_ForceProfile;
    }
    if(m_profile == "") {
      m_profile = m_Config.getString("DefaultProfile");
    }     

    /* Init sound system */
    if(!getDrawLib()->isNoGraphics()) {
      Sound::init(&m_Config);
      if(!Sound::isEnabled()) {
      }    
    }
              
    /* Init renderer */
    m_Renderer.setParent( (App *)this );
    m_Renderer.setGameObject( &m_MotoGame );        
    m_Renderer.setDebug( m_bDebugMode );

    m_Renderer.setGhostMotionBlur( m_bGhostMotionBlur );
    
    /* Tell collision system whether we want debug-info or not */
    m_MotoGame.getCollisionHandler()->setDebug( m_bDebugMode );
    
    /* Data time! */
    Log("Loading data...");

    if(m_GraphDebugInfoFile != "") m_Renderer.loadDebugInfo(m_GraphDebugInfoFile);

    m_loadingScreen = NULL;
    if(!getDrawLib()->isNoGraphics()) {    
      /* Show loading screen */
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "Loading");

      if(pSprite != NULL) {
	m_loadingScreen = pSprite->getTexture(false, true);
      }
    }

    /* database */
    m_db = new xmDatabase(DATABASE_FILE,
			  m_profile == "" ? std::string("") : m_profile,
			  getDrawLib()->isNoGraphics() ? NULL : this);
    if(m_sqlTrace) {
      m_db->setTrace(m_sqlTrace);
    }

    /* Update stats */
    if(m_profile != "")
      m_db->stats_xmotoStarted(m_profile);

    /* load levels */
    if(m_db->levels_isIndexUptodate() == false) {
      m_levelsManager.reloadLevelsFromLvl(m_db, this);
    }
    m_levelsManager.reloadExternalLevels(m_db, this);

    /* Update replays */
    
    if(m_db->replays_isIndexUptodate() == false) {
      initReplaysFromDir();
    }
    
    /* List replays? */  
    if(m_bListReplays) {
      char **v_result;
      int nrow;

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
    
    if(m_bDisplayInfosReplay) {
      Replay v_replay;
      std::string v_levelId;
      std::string v_player;
      
      v_levelId = v_replay.openReplay(m_InfosReplay, v_player, true);
      if(v_levelId == "") {
	throw Exception("Invalid replay");
      }
      
      quit();
      return;	
    }
    
    if(!getDrawLib()->isNoGraphics()) {  
      _UpdateLoadingScreen((1.0f/9.0f) * 0,m_loadingScreen,GAMETEXT_LOADINGSOUNDS);
      
      if(Sound::isEnabled()) {
        /* Load sounds */
	try {
	  for(unsigned int i=0; i<m_theme.getSoundsList().size(); i++) {
	    Sound::loadSample(m_theme.getSoundsList()[i]->FilePath());
	  }
	} catch(Exception &e) {
	  Log("*** Warning *** : %s\n", e.getMsg().c_str());
	  /* hum, not cool */
	}
	
        Log(" %d sound%s loaded",Sound::getNumSamples(),Sound::getNumSamples()==1?"":"s");
      }

      _UpdateLoadingScreen((1.0f/9.0f) * 1,m_loadingScreen,GAMETEXT_INITTEXT);
          
      /* Find all files in the textures dir and load them */     
      UITextDraw::initTextDrawing(this);
      UITexture::setApp(this);
      m_sysMsg.setFont(UITextDraw::getFont("MFont"));

      _UpdateLoadingScreen((1.0f/9.0f) * 3,m_loadingScreen,GAMETEXT_LOADINGMENUGRAPHICS);
        
      /* Load title screen textures + cursor + stuff */
      m_pTitleBL = NULL;
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "TitleBL");
      if(pSprite != NULL) {
        m_pTitleBL = pSprite->getTexture(false, true);
      }

      m_pTitleBR = NULL;
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "TitleBR");
      if(pSprite != NULL) {
        m_pTitleBR = pSprite->getTexture(false, true);
      }

      m_pTitleTL = NULL;
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "TitleTL");
      if(pSprite != NULL) {
        m_pTitleTL = pSprite->getTexture(false, true);
      }

      m_pTitleTR = NULL;
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "TitleTR");
      if(pSprite != NULL) {
        m_pTitleTR = pSprite->getTexture(false, true);
      }

      m_pCursor = NULL;
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "Cursor");
      if(pSprite != NULL) {
        m_pCursor = pSprite->getTexture(false, true, FM_LINEAR);
      }

      m_pNewLevelsAvailIcon = NULL;
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "NewLevelsAvailable");
      if(pSprite != NULL) {
        m_pNewLevelsAvailIcon = pSprite->getTexture(false, true, FM_LINEAR);
      }

      /* Fetch highscores from web? */
      if(m_pWebHighscores != NULL) delete m_pWebHighscores;
      m_pWebHighscores = new WebRoom(&m_ProxySettings);      
      m_pWebHighscores->setWebsiteInfos(m_WebHighscoresIdRoom,
					m_WebHighscoresURL);
      
    if(m_bEnableWebHighscores && m_PlaySpecificLevelFile == "" && m_PlaySpecificReplay == "") {  
      bool bSilent = true;
      try {
	if(m_bEnableCheckHighscoresAtStartup) {
	  _UpdateLoadingScreen((1.0f/9.0f) * 6,m_loadingScreen,GAMETEXT_DLHIGHSCORES);      
	  _UpdateWebHighscores(bSilent);
	  _UpgradeWebHighscores();
	}
      } catch(Exception &e) {
	/* No internet connection, probably... (just use the latest times, if any) */
	Log("** Warning ** : Failed to update web-highscores [%s]",e.getMsg().c_str());              
	if(!bSilent)
	  notifyMsg(GAMETEXT_FAILEDDLHIGHSCORES);
      }
      
      if(m_bEnableCheckNewLevelsAtStartup) {
	try {
	  _UpdateLoadingScreen((1.0f/9.0f) * 6,m_loadingScreen,GAMETEXT_DLLEVELSCHECK);      
	  _UpdateWebLevels(bSilent);       
	} catch(Exception &e) {
	  Log("** Warning ** : Failed to update web-levels [%s]",e.getMsg().c_str());              
	  if(!bSilent)
	    notifyMsg(GAMETEXT_FAILEDDLHIGHSCORES);
	}
      }
      
    }

      if(m_pWebRooms != NULL) delete m_pWebRooms;
      m_pWebRooms = new WebRooms(&m_ProxySettings);
      
    }

    /* Test level cache directory */
    if(!getDrawLib()->isNoGraphics()) {  
      _UpdateLoadingScreen((1.0f/9.0f) * 4,m_loadingScreen,GAMETEXT_LOADINGLEVELS);
    }
    LevelsManager::checkPrerequires();
    m_levelsManager.makePacks(m_db,
			      m_profile,
			      m_Config.getString("WebHighscoresIdRoom"),
			      m_bDebugMode);     

    /* Should we clean the level cache? (can also be done when disabled) */
    if(m_bCleanCache) {
      LevelsManager::cleanCache();
    }

    /* -listlevels? */
    if(m_bListLevels) {
      m_levelsManager.printLevelsList(m_db);
    }

    if(!getDrawLib()->isNoGraphics()) {  
      _UpdateLoadingScreen((1.0f/9.0f) * 5,m_loadingScreen,GAMETEXT_INITRENDERER);
    }    

    if(m_bListLevels) {
      quit();
      return;
    }

    if(!getDrawLib()->isNoGraphics()) {
      /* Initialize renderer */
      m_Renderer.init();
      _UpdateLoadingScreen((1.0f/9.0f) * 7,m_loadingScreen,GAMETEXT_INITMENUS);
      
      /* Initialize menu system */
      _InitMenus();    
      _UpdateLoadingScreen((1.0f/9.0f) * 8,m_loadingScreen,GAMETEXT_UPDATINGLEVELS);

      _UpdateLevelsLists();
      _UpdateLoadingScreen((1.0f/9.0f) * 9,m_loadingScreen,GAMETEXT_INITINPUT);      
      
      /* Init input system */
      m_InputHandler.init(&m_Config);
    }
        
    /* What to do? */
    if(m_PlaySpecificLevelFile != "") {
      try {
	m_levelsManager.addExternalLevel(m_db, m_PlaySpecificLevelFile);
	m_PlaySpecificLevel = m_levelsManager.LevelByFileName(m_db, m_PlaySpecificLevelFile);
      } catch(Exception &e) {
	m_PlaySpecificLevel = m_PlaySpecificLevelFile;
      }
    }
    if((m_PlaySpecificLevel != "") && !getDrawLib()->isNoGraphics()) {
      /* ======= PLAY SPECIFIC LEVEL ======= */
      m_StateAfterPlaying = GS_MENU;
      setState(GS_PREPLAYING);
      Log("Playing as '%s'...", m_profile.c_str());
    }
    else if(m_PlaySpecificReplay != "") {
      /* ======= PLAY SPECIFIC REPLAY ======= */
      m_StateAfterPlaying = GS_MENU;
      setState(GS_REPLAYING);
    }
    else {
      /* Graphics? */
      if(getDrawLib()->isNoGraphics())
        throw Exception("menu requires graphics");
        
      /* Do we have a player profile? */
      if(m_profile == "") {
        setState(GS_EDIT_PROFILES);
      }
      else if(m_Config.getBool("WebConfAtInit")) {
        /* We need web-config */
        _InitWebConf();
        setState(GS_EDIT_WEBCONFIG);
      }
      else {
        /* Enter the menu */
        setState(GS_MENU);
      }
    }

    Log("UserInit ended at %.3f", App::getTime());
  }
    
  /*===========================================================================
  Shutdown game
  ===========================================================================*/
  void GameApp::userShutdown(void) {  
      if(m_pWebHighscores != NULL)
        delete m_pWebHighscores;
        
      if(m_pWebLevels != NULL)
        delete m_pWebLevels;

      if(m_pWebRooms != NULL)
      delete m_pWebRooms;  
  
    if(m_pCredits != NULL)
      delete m_pCredits;
  
    if(!getDrawLib()->isNoGraphics()) {
      m_Renderer.unprepareForNewLevel(); /* just to be sure, shutdown can happen quite hard */
      m_Renderer.shutdown();
      m_InputHandler.uninit();
    }

    delete m_themeChoicer;
    
    if(m_pJustPlayReplay != NULL)
      delete m_pJustPlayReplay;

    if(m_profile != "") 
      m_Config.setString("DefaultProfile", m_profile);

    Sound::uninit();

    m_Config.saveFile();

    if(!getDrawLib()->isNoGraphics()) {
      UITextDraw::uninitTextDrawing();  
    }
  }  
  
  void GameApp::PlaySpecificLevel(std::string i_level) {
    m_PlaySpecificLevel = i_level;
          
    /* If it is a plain number, it's for a internal level */
    bool v_isANumber =  true;
    for(int i=0; i<m_PlaySpecificLevel.length(); i++) {
      if(m_PlaySpecificLevel[i] < '0' || m_PlaySpecificLevel[i] > '9') {
	v_isANumber = false;
      }
    }
    if(v_isANumber) {
      int nNum = atoi(m_PlaySpecificLevel.c_str());
      if(nNum > 0) {
	char cBuf[256];
	sprintf(cBuf,"_iL%02d_",nNum-1);
	m_PlaySpecificLevel = cBuf;
      }
    }
  }

  void GameApp::PlaySpecificReplay(std::string i_replay) {
    m_PlaySpecificReplay = i_replay;
  }

  void GameApp::PlaySpecificLevelFile(std::string i_levelFile) {
    m_PlaySpecificLevelFile = i_levelFile;
  }

  /*===========================================================================
  Handle a command-line passed argument
  ===========================================================================*/
  void GameApp::parseUserArgs(std::vector<std::string> &UserArgs) {
    /* Look through them... */
    for(int i=0;i<UserArgs.size();i++) {
      if(UserArgs[i] == "-replay") {
        if(i+1<UserArgs.size()) {
          PlaySpecificReplay(UserArgs[i+1]);
        }
        else
          throw SyntaxError("no replay specified");        
        i++;
      }
      else if(UserArgs[i] == "-level") {
        if(i+1<UserArgs.size()) {
	  PlaySpecificLevel(UserArgs[i+1]);
        } else
	throw SyntaxError("no level specified");        
        i++;
      }
      else if(UserArgs[i] == "-levelFile") {
        if(i+1<UserArgs.size()) {
	  PlaySpecificLevelFile(UserArgs[i+1]);
        } else
	throw SyntaxError("no level file specified");        
        i++;
      }
      else if(UserArgs[i] == "-debug") {
        m_bDebugMode = true;
      }
      else if(UserArgs[i] == "-sqlTrace") {
	m_sqlTrace = true;
      }
      else if(UserArgs[i] == "-profile") {
        if(i+1<UserArgs.size())
          m_ForceProfile = UserArgs[i+1];
        else
          throw SyntaxError("no profile specified");        
        i++;
      }
      else if(UserArgs[i] == "-gdebug") {
        if(i+1<UserArgs.size())
          m_GraphDebugInfoFile = UserArgs[i+1];
        else
          throw SyntaxError("no debug file specified");        
        i++;
      }      
      else if(UserArgs[i] == "-listlevels") {
        m_bListLevels = true;
	m_useGraphics = false;
      }
      else if(UserArgs[i] == "-listreplays") {
        m_bListReplays = true;
	m_useGraphics = false;
      }
      else if(UserArgs[i] == "-timedemo") {
        m_bTimeDemo = true;
      }
      else if(UserArgs[i] == "-fps") {
        m_bShowFrameRate = true;
      }
      else if(UserArgs[i] == "-ugly") {
        m_bUglyMode = true;
      }
      else if(UserArgs[i] == "-testTheme") {
        m_bTestThemeMode = true;
      }
      else if(UserArgs[i] == "-benchmark") {
        m_bBenchmark = true;
      }
      else if(UserArgs[i] == "-cleancache") {
        m_bCleanCache = true;
     } else if(UserArgs[i] == "-replayInfos") {
       if(i+1<UserArgs.size()) {
	 m_useGraphics = false;
	 m_bDisplayInfosReplay = true;
	 m_InfosReplay = UserArgs[i+1];
       } else
       throw SyntaxError("no replay specified");        
       i++;
     } else {
       /* check if the parameter is a file */
       std::string v_extension = FS::getFileExtension(UserArgs[i]);

       /* replays */
       if(v_extension == "rpl") {
	   PlaySpecificReplay(UserArgs[i]);
       } else if(v_extension == "lvl") {
	 PlaySpecificLevelFile(UserArgs[i]);
       } else {
	 /* unknown extension */
	 throw SyntaxError("Invalid argument");
       }
     }
    }
  }

  /*===========================================================================
  Show some extra user arg help
  ===========================================================================*/
  void GameApp::helpUserArgs(void) {
    printf("\t-level ID\n\t\tStart playing the given level right away.\n");
    printf("\t-replay NAME\n\t\tPlayback replay with the given name.\n");    
    printf("\t-debug\n\t\tEnable debug mode.\n");
    printf("\t-profile NAME\n\t\tUse this player profile.\n");
    printf("\t-listlevels\n\t\tOutputs a list of all installed levels.\n");
    printf("\t-listreplays\n\t\tOutputs a list of all replays.\n");
    printf("\t-timedemo\n\t\tNo delaying, maximum framerate.\n");
    printf("\t-fps\n\t\tDisplay framerate.\n");
    printf("\t-ugly\n\t\tEnable 'ugly' mode, suitable for computers without\n");
             printf("\t\ta good OpenGL-enabled video card.\n");
    printf("\t-testTheme\n\t\tDisplay forms around the theme to check it.\n");
    printf("\t-benchmark\n\t\tOnly meaningful when combined with -replay\n");
    printf("\t\tand -timedemo. Useful to determine the graphics\n");
             printf("\t\tperformance.\n");
    printf("\t-cleancache\n\t\tDeletes the content of the level cache.\n");
  }  
  
  /*===========================================================================
  Create the default config
  ===========================================================================*/
  void GameApp::_CreateDefaultConfig(void) {
    m_Config.createVar( "Theme",                  THEME_DEFAULT_THEMENAME);    

    /* Display */
    m_Config.createVar( "DisplayWidth",           "800" );
    m_Config.createVar( "DisplayHeight",          "600" );
    m_Config.createVar( "DisplayBPP",             "32" );
    m_Config.createVar( "DisplayWindowed",        "false" );
    m_Config.createVar( "MenuBackgroundGraphics", "High" );
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
    
    m_Config.createVar( "AutosaveHighscoreReplays", "true");

    #if defined(ENABLE_ZOOMING)
      m_Config.createVar( "KeyZoomIn",              "PageUp" );
      m_Config.createVar( "KeyZoomOut",             "PageDown" );
      m_Config.createVar( "KeyZoomInit",            "Home" );
      m_Config.createVar( "KeyCameraMoveXUp",       "Pad 6" );
      m_Config.createVar( "KeyCameraMoveXDown",     "Pad 4" );
      m_Config.createVar( "KeyCameraMoveYUp",       "Pad 8" );
      m_Config.createVar( "KeyCameraMoveYDown",     "Pad 2" );
      m_Config.createVar( "KeyAutoZoom",            "Pad 5" );
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
    m_Config.createVar( "WebRoomsURL",            DEFAULT_WEBROOMS_URL);
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
    m_Config.createVar( "GhostSearchStrategy", "0");
    m_Config.createVar( "ShowGhostTimeDiff"  , "true");
    m_Config.createVar( "DisplayGhostInfo"   , "false");
    m_Config.createVar( "GhostMotionBlur"    , "true" );
  }
  
}

