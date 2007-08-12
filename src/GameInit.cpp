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
#include "VDraw.h"

#define DATABASE_FILE FS::getUserDirUTF8() + "/" + "xm.db"

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
    Sprite* pSprite;

    switchUglyMode(m_xmsession->ugly());
    switchTestThemeMode(m_xmsession->testTheme());

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

    /* Init sound system */
    if(m_xmsession->useGraphics()) {
      Sound::init(&m_Config);
      if(!Sound::isEnabled()) {
      }    
    }
      
    /* Init renderer */
    m_Renderer.setParent( (App *)this );
    m_Renderer.setGameObject( &m_MotoGame );        
    m_Renderer.setDebug(m_xmsession->debug());

    m_Renderer.setGhostMotionBlur( m_bGhostMotionBlur );
    
    /* Tell collision system whether we want debug-info or not */
    m_MotoGame.getCollisionHandler()->setDebug(m_xmsession->debug());
    
    /* Data time! */
    Logger::Log("Loading data...");

    if(m_xmsession->gDebug()) m_Renderer.loadDebugInfo(m_xmsession->gDebugFile());

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
    int nrow;
    v_result = m_db->readDB("SELECT name "
			    "FROM webrooms "
			    "WHERE id_room=" + m_WebHighscoresIdRoom + ";",
			    nrow);
    if(nrow == 1) {
      m_WebHighscoresRoomName = m_db->getResult(v_result, 1, 0, 0);
    }
    m_db->read_DB_free(v_result);

    /* load theme */
    m_themeChoicer = new ThemeChoicer(
              this,
              &m_ProxySettings
              );
    m_themeChoicer->setURLBase(m_Config.getString("WebThemesURLBase"));
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

    /* Update stats */
    if(m_xmsession->profile() != "")
      m_db->stats_xmotoStarted(m_xmsession->profile());

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
      
      if(Sound::isEnabled()) {
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
      }

      _UpdateLoadingScreen((1.0f/9.0f) * 1,GAMETEXT_INITTEXT);
          
      /* Find all files in the textures dir and load them */     
      UITexture::setApp(this);
      UIWindow::setDrawLib(getDrawLib());
      m_sysMsg.setDrawLib(getDrawLib());

      _UpdateLoadingScreen((1.0f/9.0f) * 2,GAMETEXT_LOADINGMENUGRAPHICS);
        
      /* Load title screen textures + cursor + stuff */
      m_pTitleBL = NULL;
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "TitleBL");
      if(pSprite != NULL) {
        m_pTitleBL = pSprite->getTexture(false, true, FM_LINEAR);
      }

      m_pTitleBR = NULL;
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "TitleBR");
      if(pSprite != NULL) {
        m_pTitleBR = pSprite->getTexture(false, true, FM_LINEAR);
      }

      m_pTitleTL = NULL;
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "TitleTL");
      if(pSprite != NULL) {
        m_pTitleTL = pSprite->getTexture(false, true, FM_LINEAR);
      }

      m_pTitleTR = NULL;
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "TitleTR");
      if(pSprite != NULL) {
        m_pTitleTR = pSprite->getTexture(false, true, FM_LINEAR);
      }

      m_pCursor = NULL;
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "Cursor");
      if(pSprite != NULL) {
        m_pCursor = pSprite->getTexture(false, true, FM_LINEAR);
      }

      /* Fetch highscores from web? */
      if(m_pWebHighscores != NULL) delete m_pWebHighscores;
      m_pWebHighscores = new WebRoom(&m_ProxySettings);      
      m_pWebHighscores->setWebsiteInfos(m_WebHighscoresIdRoom,
					m_WebHighscoresURL);
      
    if(m_xmsession->www() && m_PlaySpecificLevelFile == "" && m_PlaySpecificReplay == "") {  
      bool bSilent = true;
      try {
	if(m_bEnableCheckHighscoresAtStartup) {
	  _UpdateLoadingScreen((1.0f/9.0f) * 3,GAMETEXT_DLHIGHSCORES);      
	  _UpdateWebHighscores(bSilent);
	  _UpgradeWebHighscores();
	}
      } catch(Exception &e) {
	/* No internet connection, probably... (just use the latest times, if any) */
	Logger::Log("** Warning ** : Failed to update web-highscores [%s]",e.getMsg().c_str());              
	if(!bSilent)
	  notifyMsg(GAMETEXT_FAILEDDLHIGHSCORES + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW);
      }
      
      if(m_bEnableCheckNewLevelsAtStartup) {
	try {
	  _UpdateLoadingScreen((1.0f/9.0f) * 4,GAMETEXT_DLLEVELSCHECK);      
	  _UpdateWebLevels(bSilent);       
	} catch(Exception &e) {
	  Logger::Log("** Warning ** : Failed to update web-levels [%s]",e.getMsg().c_str());              
	  if(!bSilent)
	    notifyMsg(GAMETEXT_FAILEDDLHIGHSCORES + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW);
	}
      }
      
    }

      if(m_pWebRooms != NULL) delete m_pWebRooms;
      m_pWebRooms = new WebRooms(&m_ProxySettings);
      
    }

    /* Test level cache directory */
    if(m_xmsession->useGraphics()) {  
      _UpdateLoadingScreen((1.0f/9.0f) * 5,GAMETEXT_LOADINGLEVELS);
    }
    LevelsManager::checkPrerequires();
    m_levelsManager.makePacks(m_db,
			      m_xmsession->profile(),
			      m_Config.getString("WebHighscoresIdRoom"),
			      m_xmsession->debug());     

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

    if(m_xmsession->useGraphics()) {  
      _UpdateLoadingScreen((1.0f/9.0f) * 6,GAMETEXT_INITRENDERER);
    }    

    if(m_xmsession->useGraphics()) {
      /* Initialize renderer */
      m_Renderer.init();
      _UpdateLoadingScreen((1.0f/9.0f) * 7,GAMETEXT_INITMENUS);
      
      /* Initialize menu system */
      _InitMenus();    
      _UpdateLoadingScreen((1.0f/9.0f) * 8,GAMETEXT_UPDATINGLEVELS);

      _UpdateLevelsLists();
      _UpdateLoadingScreen((1.0f/9.0f) * 9,GAMETEXT_INITINPUT);      
      
      /* Init input system */
      m_InputHandler.init(&m_Config);
    }
        
    /* What to do? */
    if(m_PlaySpecificLevelFile != "") {
      try {
	m_levelsManager.addExternalLevel(m_db, m_PlaySpecificLevelFile);
	m_PlaySpecificLevelId = m_levelsManager.LevelByFileName(m_db, m_PlaySpecificLevelFile);
      } catch(Exception &e) {
	m_PlaySpecificLevelId = m_PlaySpecificLevelFile;
      }
    }
    if((m_PlaySpecificLevelId != "") && m_xmsession->useGraphics()) {
      /* ======= PLAY SPECIFIC LEVEL ======= */
      m_StateAfterPlaying = GS_MENU;
      setState(GS_PREPLAYING);
      Logger::Log("Playing as '%s'...", m_xmsession->profile().c_str());
    }
    else if(m_PlaySpecificReplay != "") {
      /* ======= PLAY SPECIFIC REPLAY ======= */
      m_StateAfterPlaying = GS_MENU;
      setState(GS_REPLAYING);
    }
    else {
      /* Graphics? */
      if(m_xmsession->useGraphics() == false)
        throw Exception("menu requires graphics");
        
      /* Do we have a player profile? */
      if(m_xmsession->profile() == "") {
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

    Logger::Log("UserInit ended at %.3f", App::getTime());
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
    
    if(m_xmsession->useGraphics()) {
      m_Renderer.unprepareForNewLevel(); /* just to be sure, shutdown can happen quite hard */
      m_Renderer.shutdown();
      m_InputHandler.uninit();
    }
    
    delete m_themeChoicer;
    
    if(m_pJustPlayReplay != NULL)
    delete m_pJustPlayReplay;
    
    if(m_xmsession->profile() != "") 
    m_Config.setString("DefaultProfile", m_xmsession->profile());
    
    Sound::uninit();
    
    if(m_xmsession->useGraphics()) {
      m_Config.setInteger("QSQualityMIN",    m_pQuickStart->getQualityMIN());
      m_Config.setInteger("QSDifficultyMIN", m_pQuickStart->getDifficultyMIN());
      m_Config.setInteger("QSQualityMAX",    m_pQuickStart->getQualityMAX());
      m_Config.setInteger("QSDifficultyMAX", m_pQuickStart->getDifficultyMAX());
    }

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

    /* quick start button */
    m_Config.createVar("QSQualityMIN",    "1");
    m_Config.createVar("QSDifficultyMIN", "1");
    m_Config.createVar("QSQualityMAX",    "5");
    m_Config.createVar("QSDifficultyMAX", "5");

    /* multi */
    m_Config.createVar("MultiStopWhenOneFinishes" , "true");
  }

