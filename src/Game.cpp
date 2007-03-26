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

#if defined(SUPPORT_WEBACCESS)
  #include <curl/curl.h>
#endif

namespace vapp {

GameApp::~GameApp() {
}

GameApp::GameApp() {
  m_pCredits = NULL;
  m_bDebugMode=false;
  m_bListLevels=false;
  m_bListReplays=false;
  m_bTimeDemo=false;
  m_bShowFrameRate=false;
  m_bEnableLevelCache=true;
  m_bEnableMenuMusic=false;
  m_bEnableInitZoom=true;
  m_autoZoom = false;
  m_autoUnZoom = false;
  m_bAutoZoomInitialized = false;
  m_bLockMotoGame = false;
  m_bCleanCache=false;
  m_bEnableDeathAnim=true;
  m_pQuitMsgBox=NULL;
  m_pNotifyMsgBox=NULL;
  m_pInfoMsgBox=NULL;
#if defined(SUPPORT_WEBACCESS)
  m_pNewLevelsAvailIcon=NULL;
  m_pWebConfEditor=NULL;
  m_pWebConfMsgBox=NULL;
#endif
  m_pNewProfileMsgBox=NULL;
  m_pDeleteProfileMsgBox=NULL;
  m_pDeleteReplayMsgBox=NULL;
  m_pSaveReplayMsgBox=NULL;
  m_pReplaysWindow=NULL;
  m_pLevelPacksWindow=NULL;
  m_pStatsReport=NULL;
  m_pLevelPackViewer=NULL;  
  m_pActiveLevelPack=NULL;
  m_pGameInfoWindow=NULL;
  m_fFrameTime = 0;
  m_fFPS_Rate = 0;
  m_b50FpsMode = false;
  m_bUglyMode = false;
  m_bTestThemeMode = false;
  m_pJustPlayReplay = NULL;
  m_updateAutomaticallyLevels = false;
  m_reloadingLevelsUser = false;

  GhostSearchStrategies[0] = GHOST_STRATEGY_MYBEST;
  GhostSearchStrategies[1] = GHOST_STRATEGY_THEBEST;
  GhostSearchStrategies[2] = GHOST_STRATEGY_BESTOFROOM;
  m_bEnableGhost = true;
  m_bShowGhostTimeDiff = true;
  m_GhostSearchStrategy = GHOST_STRATEGY_MYBEST;
  m_bEnableGhostInfo = false;
  m_bGhostMotionBlur = true;

  m_bAutosaveHighscoreReplays = true;
  m_bRecordReplays = true;
  m_bShowCursor = true;
  m_bEnableEngineSound = true;
  m_bCompressReplays = true;
  m_bBenchmark = false;
  m_bEnableContextHelp = true;     
  m_bDisplayInfosReplay = false;
  
#if defined(SUPPORT_WEBACCESS)
  m_bShowWebHighscoreInGame = false;
  m_bEnableWebHighscores = true;
  m_pWebHighscores = NULL;
  m_pWebLevels = NULL;
  m_pWebRooms = NULL;
  m_fDownloadTaskProgressLast = 0;
  m_bWebHighscoresUpdatedThisSession = false;
  m_bWebLevelsToDownload = false;
  
  m_bEnableCheckNewLevelsAtStartup  = true;
  m_bEnableCheckHighscoresAtStartup = true;
  
  m_MotoGame.setHooks(&m_MotoGameHooks);
  m_MotoGameHooks.setGameApps(this, &m_MotoGame);
#endif
  m_Renderer.setTheme(getTheme());
  m_MotoGame.setRenderer(&m_Renderer);
  
  m_bPrePlayAnim = true;
  
  m_currentPlayingList = NULL;
  m_fReplayFrameRate = 25.0;
  m_stopToUpdateReplay = false;
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
    _CreateLevelLists((UILevelList *)m_pLevelPacksWindow->getChild("LEVELPACK_TABS:ALLLEVELS_TAB:ALLLEVELS_LIST"), VPACKAGENAME_FAVORITE_LEVELS);

  }

  /*===========================================================================
  Update replays list
  ===========================================================================*/
  void GameApp::_UpdateReplaysList(void) {
    _CreateReplaysList((UIList *)m_pReplaysWindow->getChild("REPLAY_LIST"));                       
  }

#if defined(SUPPORT_WEBACCESS) 
  void GameApp::_UpdateRoomsLists(void) {
    _CreateRoomsList((UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_ROOMS_TAB:ROOMS_LIST"));
  }
#endif

  void GameApp::_UpdateThemesLists(void) {
    _CreateThemesList((UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:THEMES_LIST"));
  }

  /*===========================================================================
  Change game state
  ===========================================================================*/
  void GameApp::setState(GameState s) {
    /* This function is called to perform a controlled game state change.
       The various states are described below in the switch-statement */  
    m_State = s;
    m_bCreditsModeActive = false;
    std::string v_newMusicPlaying;

    /* Always clear context when changing state */
    m_Renderer.getGUI()->clearContext();
    
    switch(s) {
      case GS_LEVELPACK_VIEWER: {
	v_newMusicPlaying = "menu1";
          m_pLevelPackViewer->showWindow(true);
          m_pMainMenu->showWindow(true);
          
          UIList *pList = (UIList *)m_pLevelPackViewer->getChild("LEVELPACK_LEVEL_LIST");
          if(pList != NULL) {
            pList->makeActive();
          }
          setPrePlayAnim(true);
        }
        break;
      case GS_CREDITSMODE:
      case GS_REPLAYING: {
	m_stopToUpdateReplay = false;
	v_newMusicPlaying = "";
	m_Renderer.setShowEngineCounter(false);

        try {  
	  /* ghost, replay */
	  m_replayBiker = NULL;
	  
          m_bShowCursor = false;
          bool bCreditsMode = (m_State == GS_CREDITSMODE);
          m_bCreditsModeActive = bCreditsMode;
          m_State = GS_REPLAYING;
	  
	  try {
	    m_replayBiker = m_MotoGame.addReplayFromFile(m_PlaySpecificReplay,
							 &m_theme, m_theme.getPlayerTheme());
	    m_Renderer.setPlayerToFollow(m_replayBiker);
	  } catch(Exception &e) {
	    setState(m_StateAfterPlaying);
            notifyMsg(e.getMsg());
	    return;
	  }

	  /* Credits mode? */
	  if(bCreditsMode) {
	    if(m_pCredits == NULL)
	      m_pCredits = new Credits;
	    
	    m_pCredits->init(m_replayBiker->getFinishTime(),4,4,GAMETEXT_CREDITS);
	  }

	  /* Fine, open the level */
	  Level *pLevelSrc;
	  
	  try {
	    pLevelSrc = &(m_levelsManager.LevelById(m_replayBiker->levelId()));
	  } catch(Exception &e) {
	    Log("** Warning ** : level '%s' specified by replay '%s' not found",
		m_replayBiker->levelId().c_str(),m_PlaySpecificReplay.c_str());
	    
	    char cBuf[256];
	    sprintf(cBuf,GAMETEXT_LEVELREQUIREDBYREPLAY,m_replayBiker->levelId().c_str());
	    setState(m_StateAfterPlaying);
	    notifyMsg(cBuf);                        
	    return;
	  }

	  if(pLevelSrc->isXMotoTooOld()) {
	    Log("** Warning ** : level '%s' specified by replay '%s' requires newer X-Moto",m_replayBiker->levelId().c_str(),m_PlaySpecificReplay.c_str());
	    
	    char cBuf[256];
	    sprintf(cBuf,GAMETEXT_NEWERXMOTOREQUIRED,pLevelSrc->getRequiredVersion().c_str());
	    setState(m_StateAfterPlaying);
	    notifyMsg(cBuf); 
	    return;
	  }
  
	  /* Init level */    
	  m_InputHandler.resetScriptKeyHooks();
	  m_MotoGame.prePlayLevel(pLevelSrc, &m_InputHandler, NULL, false);

	  /* add the ghosts */
	  if(m_bEnableGhost) {
	    std::string v_PlayGhostReplay;
	    // add the GhostSearchStrategy ghost
	    v_PlayGhostReplay = _getGhostReplayPath(pLevelSrc->Id(), m_GhostSearchStrategy);
	    if(v_PlayGhostReplay != "") {
	      try {
		switch(m_GhostSearchStrategy) {
		case GHOST_STRATEGY_MYBEST:
		  m_MotoGame.addGhostFromFile(v_PlayGhostReplay, GAMETEXT_GHOST_BEST,
					      &m_theme, m_theme.getGhostTheme());
		  break;
		case GHOST_STRATEGY_THEBEST:
		  m_MotoGame.addGhostFromFile(v_PlayGhostReplay, GAMETEXT_GHOST_LOCAL,
					      &m_theme, m_theme.getGhostTheme());
		  break;
#if defined(SUPPORT_WEBACCESS) 
		case GHOST_STRATEGY_BESTOFROOM:
		  m_MotoGame.addGhostFromFile(v_PlayGhostReplay, m_pWebHighscores->getRoomName(),
					      &m_theme, m_theme.getGhostTheme());
		  break;
#endif
		}
	      } catch(Exception &e) {
		/* can't add the ghost, anyway */
	      }
	    }
	  }
	  /* *** */

	  m_MotoGame.setInfos(m_MotoGame.getLevelSrc()->Name() +
			      " (" + std::string(GAMETEXT_BY)  +
			      " " + m_replayBiker->playerName() + ")"); 

	  m_nFrame = 0;
	  m_Renderer.prepareForNewLevel(bCreditsMode);            
	  v_newMusicPlaying = pLevelSrc->Music();

	  /* Show help string */
	  if(!drawLib->isNoGraphics()) {
	    PlayerTimeEntry *pBestTime = m_Profiles.getBestTime(m_replayBiker->levelId());
	    PlayerTimeEntry *pBestPTime = m_Profiles.getBestPlayerTime(m_pPlayer->PlayerName,
								       m_replayBiker->levelId());
	    
	    std::string T1 = "--:--:--",T2 = "--:--:--";
	    
	    if(pBestTime != NULL)
	      T1 = formatTime(pBestTime->fFinishTime);
	    if(pBestPTime != NULL)
	      T2 = formatTime(pBestPTime->fFinishTime);
	    
	    m_Renderer.setBestTime(T1 + std::string(" / ") + T2);
	    m_Renderer.showReplayHelp(m_MotoGame.getSpeed(), pLevelSrc->isScripted() == false);
	    
	    if(m_bBenchmark || bCreditsMode) m_Renderer.setBestTime("");
	    
#if defined(SUPPORT_WEBACCESS) 
	    /* World-record stuff */
	    if(!bCreditsMode)
	      _UpdateWorldRecord(m_replayBiker->levelId());
#endif
	  }
	  m_fStartTime = getRealTime();

      } catch(Exception &e) {
	m_MotoGame.endLevel();
	setState(m_StateAfterPlaying);
	notifyMsg(splitText(e.getMsg(), 50));   
      }
        break;
    }  
    case GS_MENU: {
	v_newMusicPlaying = "menu1";

        //SDL_ShowCursor(SDL_ENABLE);
        m_bShowCursor = true;                
        
        /* The main menu, the one which is entered initially when the game 
           begins. */
        m_pMainMenu->showWindow(true);

        // enable the preplay animation
        setPrePlayAnim(true);
        break;
      }
      case GS_PREPLAYING: {
	/* because statePrestart_init() can call setState */
	if(m_bEnableMenuMusic && Sound::isEnabled()) {
	  Sound::stopMusic();
	  m_playingMusic = "";
	}
        statePrestart_init();
	return;
        break;
      }
      case GS_PLAYING: {
	m_Renderer.setShowEngineCounter(m_Config.getBool("ShowEngineCounter"));
	v_newMusicPlaying = "";

	m_bAutoZoomInitialized = false;
	Level *pLevelSrc;
				
	try {
	  pLevelSrc = &(m_levelsManager.LevelById(m_PlaySpecificLevel));
	  m_MotoGame.playLevel(pLevelSrc);
          m_State = GS_PLAYING;        
          m_nFrame = 0;
	  v_newMusicPlaying = pLevelSrc->Music();
	} catch(Exception &e) {
          Log("** Warning ** : level '%s' not found",m_PlaySpecificLevel.c_str());
	  m_MotoGame.endLevel();
          char cBuf[256];
          sprintf(cBuf,GAMETEXT_LEVELNOTFOUND,m_PlaySpecificLevel.c_str());
    setState(m_StateAfterPlaying);
          notifyMsg(cBuf);
        }
        break;
      }
      case GS_PAUSE: {
	m_MotoGame.setInfos(m_MotoGame.getLevelSrc()->Name());
	v_newMusicPlaying = m_playingMusic;
//        SDL_ShowCursor(SDL_ENABLE);
        m_bShowCursor = true;

        /* Paused from GS_PLAYING */
        break;
      }
      case GS_DEADJUST: {
	m_MotoGame.setInfos(m_MotoGame.getLevelSrc()->Name());
	v_newMusicPlaying = "";

        /* Finish replay */
        if(m_pJustPlayReplay != NULL) {
	  if(m_MotoGame.Players().size() == 1) {
	    m_pJustPlayReplay->finishReplay(false,0.0f);
	  }
	}

        /* Update stats */        
	if(m_MotoGame.Players().size() == 1) {
	  m_GameStats.died(m_pPlayer->PlayerName,m_MotoGame.getLevelSrc()->Id(),m_MotoGame.getLevelSrc()->Name(),m_MotoGame.getTime());
	}                

        /* Play the DIE!!! sound */
	try {
	  Sound::playSampleByName(m_theme.getSound("Headcrash")->FilePath(),0.3);
	} catch(Exception &e) {
	}

  m_nJustDeadShade = 0;

  if(m_bEnableDeathAnim) {
    m_MotoGame.gameMessage(GAMETEXT_JUSTDEAD_RESTART,     false, 15);
    m_MotoGame.gameMessage(GAMETEXT_JUSTDEAD_DISPLAYMENU, false, 15);
  } else {
    setState(GS_DEADMENU);
  }
        break;
      }
    case GS_DEADMENU: {
      v_newMusicPlaying = "";

      m_bShowCursor = true;
      m_pJustDeadMenu->showWindow(true);

      /* Possible exit of GS_PLAYING, when the player is dead */
      m_fCoolDownEnd = getRealTime() + 0.3f;
      break;
    }
      case GS_EDIT_PROFILES: {
	v_newMusicPlaying = "menu1";
//        SDL_ShowCursor(SDL_ENABLE);
        m_bShowCursor = true;

        /* The profile editor can work on top of the main menu, or as init
           state when there is no player profiles available */
        m_pProfileEditor->showWindow(true);
        break;
      }

#if defined(SUPPORT_WEBACCESS)
      case GS_EDIT_WEBCONFIG: {
	v_newMusicPlaying = "menu1";
        m_bShowCursor = true;
        if(m_pWebConfMsgBox != NULL) delete m_pWebConfMsgBox;
        m_pWebConfEditor->showWindow(false);
        m_pWebConfMsgBox = m_Renderer.getGUI()->msgBox(GAMETEXT_ALLOWINTERNETCONN,
                                                       (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
        break;
      }
#endif
      case GS_FINISHED: {
	m_MotoGame.setInfos(m_MotoGame.getLevelSrc()->Name());
	v_newMusicPlaying = "";

//        SDL_ShowCursor(SDL_ENABLE);
        m_bShowCursor = true;

        /* Finish replay */
	if(m_pJustPlayReplay != NULL) {
	  if(m_MotoGame.Players().size() == 1) {
	    m_pJustPlayReplay->finishReplay(true,m_MotoGame.Players()[0]->finishTime());
	  }
	}

        /* Update stats */
	/* update stats only in one player mode */
	if(m_MotoGame.Players().size() == 1) {       
	  m_GameStats.levelCompleted(m_pPlayer->PlayerName,m_MotoGame.getLevelSrc()->Id(),m_MotoGame.getLevelSrc()->Name(),m_MotoGame.Players()[0]->finishTime());
	}        

        /* A more lucky outcome of GS_PLAYING than GS_DEADMENU :) */
        m_pFinishMenu->showWindow(true);
        m_pBestTimes->showWindow(true);
        m_nFinishShade = 0;            

	if(m_MotoGame.Players().size() == 1) {
	  /* display message on finish and eventually save the replay */
        
	  /* is it a highscore ? */
	  float v_best_local_time;
	  float v_best_personal_time;
	  float v_current_time;
	  bool v_is_a_highscore;
	  bool v_is_a_personal_highscore;
	  
	  v_best_local_time = m_Profiles.getBestTime(m_MotoGame.getLevelSrc()->Id())->fFinishTime;
	  v_best_personal_time = m_Profiles.getBestPlayerTime(m_pPlayer->PlayerName,
							      m_MotoGame.getLevelSrc()->Id())->fFinishTime;
	  v_current_time = m_MotoGame.Players()[0]->finishTime();
	  
	  v_is_a_highscore = (v_current_time <= v_best_local_time);  /* = because highscore is already stored in playerdata */
	  
	  v_is_a_personal_highscore = (v_current_time <= v_best_personal_time);  /* = because highscore is already stored in playerdata */
	  
#if defined(SUPPORT_WEBACCESS) 
	  /* search a better webhighscore */
	  if(m_pWebHighscores != NULL /*&& v_is_a_highscore == true*/) {
	    WebHighscore* wh = m_pWebHighscores->getHighscoreFromLevel(m_MotoGame.getLevelSrc()->Id());
	    if(wh != NULL) {
	      try {
		v_is_a_highscore = (v_current_time < wh->getFTime());
	      } catch(Exception &e) {
		v_is_a_highscore = false; /* what to do ? more chances that it's not a highscore ;-) */
	      }
	    } else {
	      /* never highscored */
	      v_is_a_highscore = true;
	    }
	  }
#endif
	  
#if defined(SUPPORT_WEBACCESS)
	  // disable upload button
	  for(int i=0;i<m_nNumFinishMenuButtons;i++) {
	    if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_UPLOAD_HIGHSCORE) {
	      m_pFinishMenuButtons[i]->enableWindow(false);
	    }
	  }
#endif
	  
	  if(v_is_a_highscore) { /* best highscore */
	    try {
	      Sound::playSampleByName(m_theme.getSound("NewHighscore")->FilePath());
	    } catch(Exception &e) {
	    }
	    
#if defined(SUPPORT_WEBACCESS)
	    // enable upload button
	    if(m_bEnableWebHighscores) {
	      if(m_pJustPlayReplay != NULL) {
		for(int i=0;i<m_nNumFinishMenuButtons;i++) {
		  if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_UPLOAD_HIGHSCORE) {
		    m_pFinishMenuButtons[i]->enableWindow(true);
		  }
		}
	      }
	    }
#endif
	    
	    if(m_pJustPlayReplay != NULL && m_bAutosaveHighscoreReplays) {
	      String v_replayName = Replay::giveAutomaticName();
	      _SaveReplay(v_replayName);
	      m_Renderer.showMsgNewBestHighscore(v_replayName);
	    } else {
	      m_Renderer.showMsgNewBestHighscore();
	    } /* ok i officially give up on indention in x-moto :P */
	  } else {
	    if(v_is_a_personal_highscore) { /* personal highscore */
	      try {
		Sound::playSampleByName(m_theme.getSound("NewHighscore")->FilePath());
	      } catch(Exception &e) {
	      }
	      if(m_pJustPlayReplay != NULL && m_bAutosaveHighscoreReplays) {
		String v_replayName = Replay::giveAutomaticName();
		_SaveReplay(v_replayName);
		m_Renderer.showMsgNewPersonalHighscore(v_replayName);
	      } else {
		m_Renderer.showMsgNewPersonalHighscore();
	      }
	      
	    } else { /* no highscore */
	      m_Renderer.hideMsgNewHighscore();
	    }
	  }
	}
        break;
      }
    }
        
    m_fLastPhysTime = getTime() - PHYS_STEP_SIZE;

    /* manage music */
    if(m_bEnableMenuMusic && Sound::isEnabled()) {
      if(v_newMusicPlaying != m_playingMusic) {
	try {
	  if(v_newMusicPlaying == "") {
	    m_playingMusic = v_newMusicPlaying;
	    Sound::stopMusic();
	  } else {
	    m_playingMusic = v_newMusicPlaying;
	    Sound::playMusic(m_theme.getMusic(v_newMusicPlaying)->FilePath());
	  }
	} catch(Exception &e) {
	  Log("** Warning ** : PlayMusic(%s) failed", v_newMusicPlaying.c_str());
	  Sound::stopMusic();
	}
      }
    }
  }

  std::string GameApp::getConfigThemeName(ThemeChoicer *p_themeChoicer) {
    std::string v_currentThemeName = m_Config.getString("Theme");

    if(p_themeChoicer->ExistThemeName(v_currentThemeName)) {
      return v_currentThemeName;
    }
    /* theme of the config file doesn't exist */
    return THEME_DEFAULT_THEMENAME;
  }

  /*===========================================================================
  Update settings
  ===========================================================================*/
  void GameApp::_UpdateSettings(void) {
    /* Menu graphics */
    std::string s = m_Config.getString("MenuBackgroundGraphics");
    if(s == "Medium") m_MenuBackgroundGraphics = MENU_GFX_LOW;
    else if(s == "High") m_MenuBackgroundGraphics = MENU_GFX_HIGH;
    else m_MenuBackgroundGraphics = MENU_GFX_OFF;
    
    /* Game graphics */
    s = m_Config.getString("GameGraphics");
    if(s == "Low") m_Renderer.setQuality(GQ_LOW);
    else if(s == "Medium") m_Renderer.setQuality(GQ_MEDIUM);
    else if(s == "High") m_Renderer.setQuality(GQ_HIGH);
      
    /* Show mini map? && show engine counter */
    m_Renderer.setShowMinimap(m_Config.getBool("ShowMiniMap"));
    m_Renderer.setShowEngineCounter(m_Config.getBool("ShowEngineCounter"));
    
    /* Replay stuff */
    m_fReplayFrameRate = m_Config.getFloat("ReplayFrameRate");
    m_bRecordReplays = m_Config.getBool("StoreReplays");
    m_bCompressReplays = m_Config.getBool("CompressReplays");
    Replay::enableCompression(m_bCompressReplays);
    m_bAutosaveHighscoreReplays = m_Config.getBool("AutosaveHighscoreReplays");

    /* ghost */
    m_bEnableGhost        = m_Config.getBool("EnableGhost");
    m_bShowGhostTimeDiff  = m_Config.getBool("ShowGhostTimeDiff");
    m_MotoGame.setShowGhostTimeDiff(m_bShowGhostTimeDiff);
    m_GhostSearchStrategy = (enum GhostSearchStrategy) m_Config.getInteger("GhostSearchStrategy");
    m_bGhostMotionBlur = m_Config.getBool("GhostMotionBlur");
    m_Renderer.setGhostMotionBlur( m_bGhostMotionBlur );

    m_bEnableGhostInfo = m_Config.getBool("DisplayGhostInfo");
    m_Renderer.setGhostDisplayInformation(m_bEnableGhostInfo);

#if defined(SUPPORT_WEBACCESS)
    m_bEnableWebHighscores = m_Config.getBool("WebHighscores") && isNoWWW()== false;
    m_bShowWebHighscoreInGame = m_Config.getBool("ShowInGameWorldRecord");
    m_bEnableCheckNewLevelsAtStartup  = m_Config.getBool("CheckNewLevelsAtStartup");
    m_bEnableCheckHighscoresAtStartup = m_Config.getBool("CheckHighscoresAtStartup");
#endif

    /* Other settings */
    m_bEnableEngineSound = m_Config.getBool("EngineSoundEnable");
    m_bEnableContextHelp = m_Config.getBool("ContextHelp");
    m_bEnableMenuMusic = m_Config.getBool("MenuMusic");
    m_bEnableInitZoom = m_Config.getBool("InitZoom");
    m_bEnableDeathAnim = m_Config.getBool("DeathAnim");

    /* Cache? */
    m_bEnableLevelCache = m_Config.getBool("LevelCache");
    
#if defined(SUPPORT_WEBACCESS)
    /* Configure proxy */
    _ConfigureProxy();
#endif
  }
  
  /*===========================================================================
  Draw menu/title screen background
  ===========================================================================*/
  void GameApp::_DrawMenuBackground(void) {
    if(m_MenuBackgroundGraphics != MENU_GFX_OFF && !m_bUglyMode) {
      if(m_pTitleTL != NULL)
        drawLib->drawImage(Vector2f(0,0),Vector2f(drawLib->getDispWidth()/2,drawLib->getDispHeight()/2),m_pTitleTL);
      if(m_pTitleTR != NULL)
        drawLib->drawImage(Vector2f(drawLib->getDispWidth()/2,0),Vector2f(drawLib->getDispWidth(),drawLib->getDispHeight()/2),m_pTitleTR);
      if(m_pTitleBR != NULL)
        drawLib->drawImage(Vector2f(drawLib->getDispWidth()/2,drawLib->getDispHeight()/2),Vector2f(drawLib->getDispWidth(),drawLib->getDispHeight()),m_pTitleBR);
      if(m_pTitleBL != NULL)
        drawLib->drawImage(Vector2f(0,drawLib->getDispHeight()/2),Vector2f(drawLib->getDispWidth()/2,drawLib->getDispHeight()),m_pTitleBL);
    } else if(m_MenuBackgroundGraphics == MENU_GFX_OFF){
        //in Ugly mode the screen is cleared in the VApp main loop
	//this is not the case when ugly mode is off.
	//and when MENU_GFX_OFF we need to clear the screen
        drawLib->clearGraphics();
    }
  }

  /*===========================================================================
  GUI mouse hover
  ===========================================================================*/
  void GameApp::_DispatchMouseHover(void) {
    int nX,nY;
    getMousePos(&nX,&nY);
    m_Renderer.getGUI()->mouseHover(nX,nY);
  }
    
  /*===========================================================================
  Screenshooting
  ===========================================================================*/
  void GameApp::_GameScreenshot(void) {
    Img *pShot = getDrawLib()->grabScreen();      
    FileHandle *pfh;
    char cBuf[256];
    int nShot=0;
    
    /* User preference for format? must be either jpeg or png */
    std::string ShotExtension = m_Config.getString("ScreenshotFormat");
    if(ShotExtension != "jpeg" && ShotExtension != "jpg" && ShotExtension != "png") {
      Log("** Warning ** : unsupported screenshot format '%s', using png instead!",ShotExtension.c_str());
      ShotExtension = "png";
    }    
    
    while(1) {
      sprintf(cBuf,"screenshot%04d.%s",nShot,ShotExtension.c_str());
      nShot++;
      if(nShot > 9999) {
        Log("Too many screenshots!");
        return;
      }
      pfh = FS::openIFile(cBuf);
      if(pfh == NULL) break;
      else FS::closeFile(pfh);
    }
    pShot->saveFile(cBuf);
    delete pShot;
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

    if(nKey == SDLK_F9) {
      switchUglyMode(!m_bUglyMode);
      return;        
    }

    if(nKey == SDLK_F10) {
      switchTestThemeMode(!m_bTestThemeMode);
      return;        
    }

    if(nKey == SDLK_F5) {
      if(m_State == GS_MENU) {
	_SimpleMessage(GAMETEXT_RELOADINGLEVELS, &m_InfoMsgBoxRect);
	m_reloadingLevelsUser = true;
	m_levelsManager.reloadLevelsFromFiles(m_bEnableLevelCache, this);
	m_pActiveLevelPack = NULL;
	_UpdateLevelsLists();
	_SimpleMessage(GAMETEXT_RELOADINGREPLAYS, &m_InfoMsgBoxRect);
	m_ReplayList.initFromDir();
      }
    }
    
    /* If message box... */
    if(m_pQuitMsgBox) {
      if(nKey == SDLK_ESCAPE) {
        delete m_pQuitMsgBox;
        m_pQuitMsgBox = NULL;
      }    
      else
        m_Renderer.getGUI()->keyDown(nKey, mod, nChar);      
      return;
    }
    else if(m_pNotifyMsgBox) {
      if(nKey == SDLK_ESCAPE) {
        delete m_pNotifyMsgBox;
        m_pNotifyMsgBox = NULL;
      }    
      else
        m_Renderer.getGUI()->keyDown(nKey, mod, nChar);      
      return;
    }
  
    /* What state? */
    switch(m_State) {
      case GS_EDIT_PROFILES:
#if defined(SUPPORT_WEBACCESS)
      case GS_EDIT_WEBCONFIG:
#endif
      case GS_LEVEL_INFO_VIEWER:
      case GS_LEVELPACK_VIEWER:
      case GS_MENU: {
        /* The GUI wants to know about keypresses... */
        m_Renderer.getGUI()->keyDown(nKey, mod,nChar);
        break;
      }
      case GS_PAUSE:
        switch(nKey) {
          case SDLK_ESCAPE:
            /* Back to the game, please */
	  m_MotoGame.setInfos("");
	  m_pPauseMenu->showWindow(false);
	  m_State = GS_PLAYING;
	  break;
          default:
            m_Renderer.getGUI()->keyDown(nKey, mod,nChar);
            break;      
        }
        break;
      case GS_DEADJUST:
      {
  switch(nKey) {
  case SDLK_RETURN:
    m_MotoGame.clearGameMessages();
    _RestartLevel();
    break;
  case SDLK_ESCAPE:
    m_MotoGame.clearGameMessages();
    setState(GS_DEADMENU);
    break;
  }
  break;
      }
      case GS_FINISHED:
      case GS_DEADMENU:
        switch(nKey) {
          case SDLK_ESCAPE:
            if(m_pSaveReplayMsgBox == NULL) {          
              /* Out of this game, please */
              m_pFinishMenu->showWindow(false);
        m_Renderer.hideMsgNewHighscore();
              m_pBestTimes->showWindow(false);
              m_pJustDeadMenu->showWindow(false);
	      m_Renderer.setPlayerToFollow(NULL);
              m_MotoGame.endLevel();
              m_InputHandler.resetScriptKeyHooks();                         
              m_Renderer.unprepareForNewLevel();
              //setState(GS_MENU);
              setState(m_StateAfterPlaying);
            }
            else {
              if(m_State == GS_DEADMENU)
                if(getRealTime() < m_fCoolDownEnd)
                  break;
               
              m_Renderer.getGUI()->keyDown(nKey, mod,nChar);
            }
            break;
          default:
            if(m_State == GS_DEADMENU)
              if(getRealTime() < m_fCoolDownEnd)
                break;
             
            m_Renderer.getGUI()->keyDown(nKey, mod,nChar);
            break;      
        }
        break;
      case GS_REPLAYING:
        switch(nKey) {
          case SDLK_ESCAPE:
            /* Escape quits the replay */
	    m_Renderer.setPlayerToFollow(NULL);
            m_MotoGame.endLevel();
            m_InputHandler.resetScriptKeyHooks();                      
            m_Renderer.unprepareForNewLevel();
      setState(m_StateAfterPlaying);
            break;          
          case SDLK_RIGHT:
            /* Right arrow key: fast forward */
	    if(m_stopToUpdateReplay == false) {
	      m_MotoGame.fastforward(1);
	    }
            break;
          case SDLK_LEFT:
	    if(m_MotoGame.getLevelSrc()->isScripted() == false) {
	      m_MotoGame.fastrewind(1);
	      m_stopToUpdateReplay = false;
	    }
            break;
	case SDLK_F2:
	  m_Renderer.switchFollow();
	  break;
  case SDLK_SPACE:
    /* pause */
    m_MotoGame.pause();

    m_Renderer.showReplayHelp(m_MotoGame.getSpeed(),
			      m_MotoGame.getLevelSrc()->isScripted() == false
			      ); /* update help */
    break;
  case SDLK_UP:
    /* faster */
    m_MotoGame.faster();

    m_Renderer.showReplayHelp(m_MotoGame.getSpeed(),
			      m_MotoGame.getLevelSrc()->isScripted() == false
			      ); /* update help */
    break;
  case SDLK_DOWN:
    /* slower */
    m_MotoGame.slower();
    m_stopToUpdateReplay = false;
    
    m_Renderer.showReplayHelp(m_MotoGame.getSpeed(),
			      m_MotoGame.getLevelSrc()->isScripted() == false
			      ); /* update help */
    break;
        }
      break;
      case GS_PREPLAYING:
      /* any key to remove the animation */
      //switch(nKey) {
      //case SDLK_ESCAPE:
      //case SDLK_RETURN:
  m_bPrePlayAnim = false;
      //break;
      //}
      break;
      case GS_PLAYING:
        switch(nKey) {
  case SDLK_ESCAPE:
		if(isLockedMotoGame() == false) {
			/* Escape pauses */
			setState(GS_PAUSE);
			m_pPauseMenu->showWindow(true);
			m_nPauseShade = 0;
		}
		break;
	case SDLK_F2:
	  m_Renderer.switchFollow();
	  break;
  case SDLK_RETURN:
    /* retart immediatly the level */
    _RestartLevel();
    break;
  case SDLK_F5:
    _RestartLevel(true);
    break;
          default:
            /* Notify the controller */
	    for(unsigned int i=0; i<m_MotoGame.Players().size(); i++) {
	      if(m_MotoGame.Players()[i]->isDead() == false) {
		m_InputHandler.handleInput(INPUT_KEY_DOWN,nKey,mod,
					   m_MotoGame.Players()[i]->getControler(),
					   i,
					   &m_Renderer, this);
	      }
	    }
        }
        break; 
    }
  }

  /*===========================================================================
  Key up event
  ===========================================================================*/
  void GameApp::keyUp(int nKey, SDLMod mod) {
    /* What state? */
    switch(m_State) {
#if defined(SUPPORT_WEBACCESS)
      case GS_EDIT_WEBCONFIG:
#endif
      case GS_EDIT_PROFILES:
      case GS_LEVEL_INFO_VIEWER:
      case GS_FINISHED:
      case GS_DEADMENU:
      case GS_LEVELPACK_VIEWER:
      case GS_MENU:
        m_Renderer.getGUI()->keyUp(nKey, mod);
        break;
      case GS_PLAYING:
        /* Notify the controller */
	    for(unsigned int i=0; i<m_MotoGame.Players().size(); i++) {
	      if(m_MotoGame.Players()[i]->isDead() == false) {
		m_InputHandler.handleInput(INPUT_KEY_UP,nKey,mod,
					   m_MotoGame.Players()[i]->getControler(),
					   i,
					   &m_Renderer, this);
	      }
	    }
        break; 
      case GS_DEADJUST:
      {
  break;
      }
    }
  }

  /*===========================================================================
  Mouse events
  ===========================================================================*/
  void GameApp::mouseDoubleClick(int nButton) {
    switch(m_State) {
      case GS_MENU:
      case GS_PAUSE:
      case GS_DEADMENU:
      case GS_FINISHED:
      case GS_EDIT_PROFILES:
#if defined(SUPPORT_WEBACCESS)
      case GS_EDIT_WEBCONFIG:
#endif
      case GS_LEVEL_INFO_VIEWER:
      case GS_LEVELPACK_VIEWER:
        int nX,nY;        
        getMousePos(&nX,&nY);
        
        if(nButton == SDL_BUTTON_LEFT)
          m_Renderer.getGUI()->mouseLDoubleClick(nX,nY);
        
        break;
      case GS_DEADJUST:
      break;
    }
  }

  void GameApp::mouseDown(int nButton) {
    switch(m_State) {
      case GS_MENU:
      case GS_PAUSE:
      case GS_DEADMENU:
      case GS_FINISHED:
      case GS_EDIT_PROFILES:
#if defined(SUPPORT_WEBACCESS)
      case GS_EDIT_WEBCONFIG:
#endif
      case GS_LEVEL_INFO_VIEWER:
      case GS_LEVELPACK_VIEWER:
        int nX,nY;        
        getMousePos(&nX,&nY);
        
        if(nButton == SDL_BUTTON_LEFT)
          m_Renderer.getGUI()->mouseLDown(nX,nY);
        else if(nButton == SDL_BUTTON_RIGHT)
          m_Renderer.getGUI()->mouseRDown(nX,nY);
        else if(nButton == SDL_BUTTON_WHEELUP)
          m_Renderer.getGUI()->mouseWheelUp(nX,nY);
        else if(nButton == SDL_BUTTON_WHEELDOWN)        
          m_Renderer.getGUI()->mouseWheelDown(nX,nY);
        
        break;

      case GS_PLAYING:
      /* Notify the controller */
      for(unsigned int i=0; i<m_MotoGame.Players().size(); i++) {
	if(m_MotoGame.Players()[i]->isDead() == false) {
	  m_InputHandler.handleInput(INPUT_KEY_DOWN,nButton,KMOD_NONE,
				     m_MotoGame.Players()[i]->getControler(),
				     i,
				     &m_Renderer, this);
	}
      }

      break;
      case GS_DEADJUST:
      break;
    }
  }

  void GameApp::mouseUp(int nButton) {
    switch(m_State) {
      case GS_MENU:
      case GS_PAUSE:
      case GS_DEADMENU:
      case GS_FINISHED:
      case GS_EDIT_PROFILES:
#if defined(SUPPORT_WEBACCESS)
      case GS_EDIT_WEBCONFIG:
#endif
      case GS_LEVEL_INFO_VIEWER:
      case GS_LEVELPACK_VIEWER:
        int nX,nY;
        getMousePos(&nX,&nY);
        
        if(nButton == SDL_BUTTON_LEFT)
          m_Renderer.getGUI()->mouseLUp(nX,nY);
        else if(nButton == SDL_BUTTON_RIGHT)
          m_Renderer.getGUI()->mouseRUp(nX,nY);
        break;

      case GS_PLAYING:
        /* Notify the controller */
      for(unsigned int i=0; i<m_MotoGame.Players().size(); i++) {
	if(m_MotoGame.Players()[i]->isDead() == false) {
	  m_InputHandler.handleInput(INPUT_KEY_UP,nButton,KMOD_NONE,
				     m_MotoGame.Players()[i]->getControler(),
				     i,
				     &m_Renderer, this);
	}
      }


        break;
      case GS_DEADJUST:
      break;
    }
  }
      
  /*===========================================================================
  Notification popup
  ===========================================================================*/
  void GameApp::notifyMsg(std::string Msg) {
    if(m_pNotifyMsgBox != NULL) delete m_pNotifyMsgBox;
    m_pNotifyMsgBox = m_Renderer.getGUI()->msgBox(Msg,(UIMsgBoxButton)(UI_MSGBOX_OK));
  }
  
  /*===========================================================================
  Save a replay
  ===========================================================================*/
  void GameApp::_SaveReplay(const std::string &Name) {
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
      Log("** Warning ** : Failed to save replay: %s",Name.c_str());
      notifyMsg(GAMETEXT_FAILEDTOSAVEREPLAY);
    } else {
      /* Update replay list to reflect changes */
      m_ReplayList.addReplay(FS::getFileBaseName(v_outputfile));
      _UpdateReplaysList();
    }
  }

  std::string GameApp::_DetermineNextLevel(Level *pLevelSrc) {
    if(m_currentPlayingList == NULL) {
      return "";
    }

    for(int i=0;i<m_currentPlayingList->getEntries().size()-1;i++) {
      if(m_currentPlayingList->getEntries()[i]->pvUser == (void *)pLevelSrc) {
  return ((Level *)m_currentPlayingList->getEntries()[i+1]->pvUser)->Id();
      }
    }
    return ((Level *)m_currentPlayingList->getEntries()[0]->pvUser)->Id();
  }
  
  bool GameApp::_IsThereANextLevel(Level *pLevelSrc) {
    return _DetermineNextLevel(pLevelSrc) != "";
  }  

#if defined(SUPPORT_WEBACCESS)  
  void GameApp::_UpdateWorldRecord(const std::string &LevelID) {  
    m_Renderer.setWorldRecordTime("");
    
      if(m_bShowWebHighscoreInGame && m_pWebHighscores!=NULL) {
        WebHighscore *pWebHS = m_pWebHighscores->getHighscoreFromLevel(LevelID);
        if(pWebHS != NULL) {
          m_Renderer.setWorldRecordTime(pWebHS->getRoom()->getRoomName() + ": " + 
                                        pWebHS->getTime() + 
                                        std::string(" (") + pWebHS->getPlayerName() + std::string(")"));
        } 
        else {
    m_Renderer.setWorldRecordTime(m_pWebHighscores->getRoomName() + ": " + 
          GAMETEXT_WORLDRECORDNA
          );
        }                
      }
  }
#endif  

#if defined(SUPPORT_WEBACCESS)  
  void GameApp::_UpdateWebHighscores(bool bSilent) {
    if(!bSilent) {
      _SimpleMessage(GAMETEXT_DLHIGHSCORES,&m_InfoMsgBoxRect);
    }

    m_bWebHighscoresUpdatedThisSession = true;
    
    /* Try downloading the highscores */
    m_pWebHighscores->setWebsiteURL(m_Config.getString("WebHighscoresURL"));
    m_pWebHighscores->update();
  }
#endif

#if defined(SUPPORT_WEBACCESS)  
  void GameApp::_UpdateWebLevels(bool bSilent, bool bEnableWeb) {
    if(!bSilent) {
      _SimpleMessage(GAMETEXT_DLLEVELSCHECK,&m_InfoMsgBoxRect);
    }

    /* Try download levels list */
    if(m_pWebLevels == NULL) {
      m_pWebLevels = new WebLevels(this,&m_ProxySettings);
    }
    m_pWebLevels->setURL(m_Config.getString("WebLevelsURL"));
    Log("WWW: Checking for new or updated levels...");
    m_pWebLevels->update(bEnableWeb);
    
    int nULevels=0,nUBytes=0;
    m_pWebLevels->getUpdateInfo(&nUBytes,&nULevels);
    m_bWebLevelsToDownload = nULevels!=0;
  }
#endif

#if defined(SUPPORT_WEBACCESS)  
  void GameApp::_UpdateWebThemes(bool bSilent) {
    if(!bSilent) {
      _SimpleMessage(GAMETEXT_DLTHEMESLISTCHECK,&m_InfoMsgBoxRect);
    }  

    m_themeChoicer->setURL(m_Config.getString("WebThemesURL"));

    Log("WWW: Checking for new or updated themes...");

    try {
      m_DownloadingInformation = "";
      m_themeChoicer->updateFromWWW();
      _UpdateThemesLists();
    } catch(Exception &e) {
      /* file probably doesn't exist */
      Log("** Warning ** : Failed to analyse web-themes file");   
    }
  }    
#endif

#if defined(SUPPORT_WEBACCESS) 
  void GameApp::_UpdateWebRooms(bool bSilent) {
    if(!bSilent) {
      _SimpleMessage(GAMETEXT_DLROOMSLISTCHECK,&m_InfoMsgBoxRect);
    }  

    m_pWebRooms->setURL(m_Config.getString("WebRoomsURL"));

    Log("WWW: Checking for rooms list...");

    try {
      m_pWebRooms->update();
    } catch(Exception &e) {
      /* file probably doesn't exist */
      Log("** Warning ** : Failed to analyse update webrooms list");    
    }
  }
#endif

#if defined(SUPPORT_WEBACCESS)
  void GameApp::_UpdateWebTheme(ThemeChoice* pThemeChoice, bool bNotify) {
    if(m_themeChoicer->isUpdatableThemeFromWWW(pThemeChoice) == false) {
      if(bNotify) {
  notifyMsg(GAMETEXT_UNUPDATABLETHEMEONWEB);
      }
      return;
    }

    m_DownloadingInformation = "";
    m_DownloadingMessage = std::string(GAMETEXT_DLTHEME) + "\n\n ";
    m_themeChoicer->setURLBase(m_Config.getString("WebThemesURLBase"));

    try {
      Log("WWW: Downloading a theme...");
      clearCancelAsSoonAsPossible();
      m_themeChoicer->updateThemeFromWWW(pThemeChoice);
      _UpdateThemesLists();
      reloadTheme(); /* reload the theme */
      if(bNotify) {
  notifyMsg(GAMETEXT_THEMEUPTODATE);
      }
    } catch(Exception &e) {
      /* file probably doesn't exist */
      Log("** Warning ** : Failed to update theme ", pThemeChoice->ThemeName().c_str());    
      if(bNotify) {
  notifyMsg(GAMETEXT_FAILEDGETSELECTEDTHEME);
      }
      return;
    }
  }
#endif

#if defined(SUPPORT_WEBACCESS)  
  void GameApp::_UpgradeWebHighscores() {
    /* Upgrade high scores */
    try {
      m_pWebHighscores->upgrade();      
    } catch(Exception &e) {
      /* file probably doesn't exist */
      Log("** Warning ** : Failed to analyse web-highscores file");   
    }
  }

  void GameApp::_UpgradeWebRooms(bool bUpdateMenus) {
    /* Upgrade high scores */
    try {
      m_pWebRooms->upgrade();   
      if(bUpdateMenus) {
  _UpdateRoomsLists();
      }
    } catch(Exception &e) {
      /* file probably doesn't exist */
      Log("** Warning ** : Failed to analyse webrooms file");   
    }
  }
#endif

#if defined(SUPPORT_WEBACCESS)  
  /*===========================================================================
  Extra WWW levels
  ===========================================================================*/
  void GameApp::_DownloadExtraLevels(void) {
    #if defined(SUPPORT_WEBACCESS)
      /* Download extra levels */
      m_DownloadingInformation = "";
      m_DownloadingMessage = std::string(GAMETEXT_DLLEVELS) + "\n\n ";

      if(m_pWebLevels != NULL) {
        try {                  
          Log("WWW: Downloading levels...");
          clearCancelAsSoonAsPossible();
          m_pWebLevels->upgrade();
    m_bWebLevelsToDownload = false;
        } 
        catch(Exception &e) {
          Log("** Warning ** : Unable to download extra levels [%s]",e.getMsg().c_str());
  
          if(m_pInfoMsgBox != NULL) {
            delete m_pInfoMsgBox;
            m_pInfoMsgBox = NULL;
          }
          notifyMsg(GAMETEXT_FAILEDDLLEVELS);
        }

        /* Got some new levels... load them! */
        Log("Loading new and updated levels...");
				m_pActiveLevelPack = NULL; /* the active level pack could no more exists after update */
				m_levelsManager.updateLevelsFromLvl(m_pWebLevels->getNewDownloadedLevels(),
																						m_pWebLevels->getUpdatedDownloadedLevels(),
																						m_pWebLevels->getUpdatedDownloadedLevelIds(),
																						m_bEnableLevelCache);

         /* Update level lists */
				_UpdateLevelsLists();
      }
    #endif
  }
#endif

#if defined(SUPPORT_WEBACCESS)  
  void GameApp::_CheckForExtraLevels(void) {
      /* Check for extra levels */
      try {
        _SimpleMessage(GAMETEXT_CHECKINGFORLEVELS);
      
        if(m_pWebLevels == NULL) {
    m_pWebLevels = new WebLevels(this,&m_ProxySettings);
  }
        m_pWebLevels->setURL(m_Config.getString("WebLevelsURL"));
        
        Log("WWW: Checking for new or updated levels...");
        clearCancelAsSoonAsPossible();
        m_pWebLevels->update();     
        int nULevels=0,nUBytes=0;
        m_pWebLevels->getUpdateInfo(&nUBytes,&nULevels);
  m_bWebLevelsToDownload = nULevels!=0;

        Log("WWW: %d new or updated level%s found",nULevels,nULevels==1?"":"s");

        if(nULevels == 0) {
          notifyMsg(GAMETEXT_NONEWLEVELS);
        }        
        else {
          /* Ask user whether he want to download levels or snot */
          if(m_pInfoMsgBox == NULL) {
            char cBuf[256];
            
            sprintf(cBuf,nULevels==1?GAMETEXT_NEWLEVELAVAIL:
                                     GAMETEXT_NEWLEVELSAVAIL,nULevels);
            m_pInfoMsgBox = m_Renderer.getGUI()->msgBox(cBuf,(UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
          }
        }
      } 
      catch(Exception &e) {
        Log("** Warning ** : Unable to check for extra levels [%s]",e.getMsg().c_str());
        if(m_pInfoMsgBox != NULL) {
          delete m_pInfoMsgBox;
          m_pInfoMsgBox = NULL;
        }
        notifyMsg(GAMETEXT_FAILEDCHECKLEVELS);
      } 
  }
#endif  

  void GameApp::_RestartLevel(bool i_reloadLevel) {
		lockMotoGame(false);

    /* Update stats */        
    if(m_MotoGame.Players().size() == 1) {
      m_GameStats.levelRestarted(m_pPlayer->PlayerName,m_MotoGame.getLevelSrc()->Id(),m_MotoGame.getLevelSrc()->Name(),m_MotoGame.getTime());
    }  

		m_Renderer.setPlayerToFollow(NULL);
		m_MotoGame.endLevel();

    m_InputHandler.resetScriptKeyHooks();           
    m_Renderer.unprepareForNewLevel();

    if(i_reloadLevel) {
      try {
	Level *v_lvl = &(m_levelsManager.LevelById(m_PlaySpecificLevel));
	v_lvl->loadXML();
	v_lvl->rebuildCache();
      } catch(Exception &e) {
	throw Exception("Unable to reload the level");
      }
    }

    setState(GS_PREPLAYING);   
  }

  /*===========================================================================
  WWWAppInterface implementation
  ===========================================================================*/
#if defined(SUPPORT_WEBACCESS)
        
  bool GameApp::shouldLevelBeUpdated(const std::string &LevelID) {
    if(m_updateAutomaticallyLevels) {
      return true;
    }

    /* Hmm... ask user whether this level should be updated */
    bool bRet = true;
    
    Level *pLevel;

    pLevel = &(m_levelsManager.LevelById(LevelID));
    if(pLevel != NULL) {
      bool bDialogBoxOpen = true;
      
      char cBuf[1024];
      sprintf(cBuf,(std::string(GAMETEXT_WANTTOUPDATELEVEL) + "\n(%s)").c_str(),pLevel->Name().c_str(),
              pLevel->FileName().c_str());
      UIMsgBox *pMsgBox = m_Renderer.getGUI()->msgBox(cBuf,(UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO|UI_MSGBOX_YES_FOR_ALL));
      
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

        _DrawMenuBackground();
        _DispatchMouseHover();
        
        m_Renderer.getGUI()->paint();
        
        UIRect TempRect;

  if(m_pCursor != NULL) {        
    int nMX,nMY;
    getMousePos(&nMX,&nMY);      
    drawLib->drawImage(Vector2f(nMX-2,nMY-2),Vector2f(nMX+30,nMY+30),m_pCursor);
  }

        drawLib->flushGraphics();
      }
      
      delete pMsgBox;
      setTaskProgress(m_fDownloadTaskProgressLast);
    }  
    return bRet;        
  }
        
  void GameApp::setTaskProgress(float fPercent) {
    int nBarHeight = 15;
    m_fDownloadTaskProgressLast = fPercent;
    readEvents();
    
    _DrawMenuBackground();
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

    UIFont *v_font = m_Renderer.getSmallFont();
    if(v_font != NULL) {
      UITextDraw::printRaw(v_font,m_InfoMsgBoxRect.nX+13,m_InfoMsgBoxRect.nY+
         m_InfoMsgBoxRect.nHeight-nBarHeight-4,m_DownloadingInformation,MAKE_COLOR(255,255,255,128));
    }
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
  
  std::string GameApp::levelPathForUpdate(const std::string &p_LevelId) {
    return m_levelsManager.LevelById(p_LevelId).PathForUpdate();
  }
  
  std::string GameApp::levelMD5Sum(const std::string &LevelID) {
    return m_levelsManager.LevelById(LevelID).Checksum();
  }
    
  bool GameApp::doesLevelExist(const std::string &p_LevelId) {
    return m_levelsManager.doesLevelExist(p_LevelId);
  }

#endif

#if defined(SUPPORT_WEBACCESS)  
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
#endif
  
  std::string GameApp::_getGhostReplayPath_bestOfThePlayer(std::string p_levelId, float &p_time) {
    std::vector<ReplayInfo *> *Replays = m_ReplayList.findReplays(m_pPlayer->PlayerName,
                  p_levelId);
    std::string res;

    p_time = -1.0;
    res = "";

    for(int i=0; i<Replays->size(); i++) {
      if((*Replays)[i]->fFinishTime != -1.0 &&
   ((*Replays)[i]->fFinishTime < p_time || p_time == -1)
   )
  {
    p_time = (*Replays)[i]->fFinishTime;
    res = std::string("Replays/") + (*Replays)[i]->Name + std::string(".rpl");
  }
    }

    delete Replays;
    return res;
  }

#if defined(SUPPORT_WEBACCESS)
  std::string GameApp::_getGhostReplayPath_bestOfTheRoom(std::string p_levelId, float &p_time) {
    std::vector<ReplayInfo *> *Replays = m_ReplayList.findReplays("",
                  p_levelId);
    std::string res;

    p_time = -1.0;
    res = "";

    if(m_pWebHighscores != NULL) {
      WebHighscore* v_hs;
      v_hs = m_pWebHighscores->getHighscoreFromLevel(p_levelId);
      if(v_hs != NULL) {
  String v_replay_name = v_hs->getReplayName();
  int i=0;

  p_time = v_hs->getFTime();

  /* search if the replay is already downloaded */
  bool found = false;
  while(i < Replays->size() && found == false) {
    if((*Replays)[i]->Name == v_replay_name) {
      found = true;
    }
    i++;
  }
  if(found) {
    res = std::string("Replays/") + v_replay_name + std::string(".rpl");
  } else {
    if(m_bEnableWebHighscores) {
      /* download the replay */
      try {
        _SimpleMessage(GAMETEXT_DLGHOST,&m_InfoMsgBoxRect);
        v_hs->download();
        res = std::string("Replays/") + v_replay_name + std::string(".rpl");
        m_ReplayList.addReplay(v_replay_name);
        _UpdateReplaysList();
      } catch(Exception &e) {
        /* do nothing */
      }
    }
  }
      }
    }

    delete Replays;
    return res;
  }
#endif

  std::string GameApp::_getGhostReplayPath_bestOfLocal(std::string p_levelId, float &p_time) {
    std::vector<ReplayInfo *> *Replays = m_ReplayList.findReplays("", p_levelId);
    std::string res;
    std::vector<PlayerProfile *> v_profiles = m_Profiles.getProfiles();

    p_time = -1.0;
    res = "";

    for(int i=0; i<Replays->size(); i++) {
      if((*Replays)[i]->fFinishTime != -1.0 &&
   ((*Replays)[i]->fFinishTime < p_time ||
    p_time == -1)
   )
  {
    /* the player must be one of the profile */
    for(int j=0; j<v_profiles.size(); j++) {
      if(v_profiles[j]->PlayerName == (*Replays)[i]->Player) {
        p_time = (*Replays)[i]->fFinishTime;
        res = std::string("Replays/") + (*Replays)[i]->Name + std::string(".rpl");
      }
    }
  }
    }
    delete Replays;
    return res;
  }

  std::string GameApp::_getGhostReplayPath(std::string p_levelId,
             GhostSearchStrategy p_strategy) 
  {
    std::string res = "";
    float v_fFinishTime;
    std::string v_player_res;
    float v_player_fFinishTime;

    switch(p_strategy) {
    case GHOST_STRATEGY_MYBEST:
      res = _getGhostReplayPath_bestOfThePlayer(p_levelId, v_fFinishTime);
      break;
      
    case GHOST_STRATEGY_THEBEST:
      res = _getGhostReplayPath_bestOfLocal(p_levelId, v_fFinishTime);
      break;
      
#if defined(SUPPORT_WEBACCESS)    
    case GHOST_STRATEGY_BESTOFROOM:     
      v_player_res = _getGhostReplayPath_bestOfThePlayer(p_levelId, v_player_fFinishTime);
      res = _getGhostReplayPath_bestOfTheRoom(p_levelId, v_fFinishTime);
      if(v_player_fFinishTime > 0.0 && 
   (v_fFinishTime < 0.0 || v_player_fFinishTime < v_fFinishTime)) {
  res = v_player_res;
      }
      break;
#endif      

    }

    return res;
  }

#if defined(SUPPORT_WEBACCESS) 
  void GameApp::_UploadHighscore(std::string p_replayname) {
    try {
      bool v_msg_status_ok;
      std::string v_msg;
      clearCancelAsSoonAsPossible();
      m_DownloadingInformation = "";
      m_DownloadingMessage = GAMETEXT_UPLOADING_HIGHSCORE;
      FSWeb::uploadReplay(FS::getUserDir() + "/Replays/" + p_replayname + ".rpl",
        m_Config.getString("WebHighscoreUploadIdRoom"),
        m_Config.getString("WebHighscoreUploadLogin"),
        m_Config.getString("WebHighscoreUploadPassword"),
        m_Config.getString("WebHighscoreUploadURL"),
        this,
        &m_ProxySettings,
        v_msg_status_ok,
        v_msg);
      if(v_msg_status_ok) {
  notifyMsg(v_msg);
      } else {
  notifyMsg(std::string(GAMETEXT_UPLOAD_HIGHSCORE_WEB_WARNING_BEFORE) + "\n" + v_msg);
      }
    } catch(Exception &e) {
      notifyMsg(GAMETEXT_UPLOAD_HIGHSCORE_ERROR);
    }
  }
#endif

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
    vapp::Color v_color;

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
    m_bUglyMode = bUgly;
    if(bUgly == false) {
      SDL_ShowCursor(SDL_DISABLE);        
    } else {
      SDL_ShowCursor(SDL_ENABLE);
    }
    m_Renderer.setUglyMode(bUgly);
  }

  void GameApp::switchTestThemeMode(bool mode) {
    m_bTestThemeMode = mode;
    m_Renderer.setTestThemeMode(m_bTestThemeMode);
  }

  void GameApp::setPrePlayAnim(bool pEnabled) {
    m_bPrePlayAnim = pEnabled;
  }

  void GameApp::statePrestart_init() {
    //        SDL_ShowCursor(SDL_DISABLE);
    m_bShowCursor = false;
      
    /* Initialize controls */
    m_InputHandler.configure(&m_Config);
      
    /* Default playing state */
    m_fLastFrameTime = 0.0f;
    m_fLastPerfStateTime = 0.0f;
      
    /* We need a profile */
    if(m_pPlayer == NULL) {
      Log("** Warning ** : no player profile selected, use -profile option");
      throw Exception("no player");
    }
      
    /* Find the level */
    Level *pLevelSrc;
    try {
      pLevelSrc = &(m_levelsManager.LevelById(m_PlaySpecificLevel));
    } catch(Exception &e) {
      Log("** Warning ** : level '%s' not found",m_PlaySpecificLevel.c_str());
      char cBuf[256];
      sprintf(cBuf,GAMETEXT_LEVELNOTFOUND,m_PlaySpecificLevel.c_str());
      setState(m_StateAfterPlaying);
      notifyMsg(cBuf);
      return;
    }

    if(pLevelSrc->isXMotoTooOld()) {
      Log("** Warning ** : level '%s' requires newer X-Moto",m_PlaySpecificLevel.c_str());
  
      char cBuf[256];
      sprintf(cBuf,GAMETEXT_NEWERXMOTOREQUIRED,pLevelSrc->getRequiredVersion().c_str());
      setState(m_StateAfterPlaying);
      notifyMsg(cBuf);     
      return;
    }

    /* Start playing right away */     
    m_InputHandler.resetScriptKeyHooks();

      if(m_pJustPlayReplay != NULL) delete m_pJustPlayReplay;
      m_pJustPlayReplay = NULL;
      
      if(m_bRecordReplays) {
	m_pJustPlayReplay = new Replay;
	m_pJustPlayReplay->createReplay("Latest.rpl",pLevelSrc->Id(),m_pPlayer->PlayerName, m_fReplayFrameRate,sizeof(SerializedBikeState));
      }
      
      try {
	m_MotoGame.prePlayLevel(pLevelSrc, &m_InputHandler, m_pJustPlayReplay, true);
	m_MotoGame.setInfos("");
	
	/* add the players */
	int v_nbPlayer = getNumberOfPlayersToPlay();
	Log("Preplay level for %i player(s)", v_nbPlayer);

	/* add at least one player for the camera */
	m_Renderer.setPlayerToFollow(m_MotoGame.addPlayerBiker(pLevelSrc->PlayerStart(), DD_RIGHT,
							       &m_theme, m_theme.getPlayerTheme(),
							       getColorFromPlayerNumber(0),
							       getUglyColorFromPlayerNumber(0)));

	for(int i=1; i<v_nbPlayer; i++) {
	  m_MotoGame.addPlayerBiker(pLevelSrc->PlayerStart(), DD_RIGHT,
				    &m_theme, m_theme.getPlayerTheme(),
				    getColorFromPlayerNumber(i),
				    getUglyColorFromPlayerNumber(i));
	}
	/* */

	/* add the ghosts */
	if(m_bEnableGhost) {
	  std::string v_PlayGhostReplay;
	  
	  // add the GhostSearchStrategy ghost
	  v_PlayGhostReplay = _getGhostReplayPath(pLevelSrc->Id(), m_GhostSearchStrategy);
	  if(v_PlayGhostReplay != "") {
	    try {
	      switch(m_GhostSearchStrategy) {
	      case GHOST_STRATEGY_MYBEST:
		m_MotoGame.addGhostFromFile(v_PlayGhostReplay, GAMETEXT_GHOST_BEST,
					    &m_theme, m_theme.getGhostTheme());
		break;
	      case GHOST_STRATEGY_THEBEST:
		m_MotoGame.addGhostFromFile(v_PlayGhostReplay, GAMETEXT_GHOST_LOCAL,
					    &m_theme, m_theme.getGhostTheme());
		break;
#if defined(SUPPORT_WEBACCESS) 
	      case GHOST_STRATEGY_BESTOFROOM:
		m_MotoGame.addGhostFromFile(v_PlayGhostReplay, m_pWebHighscores->getRoomName(),
					    &m_theme, m_theme.getGhostTheme());
		break;
#endif
	      }
	    } catch(Exception &e) {
	      /* can't add the ghost, anyway */
	    }
	  }

	}
      } catch(Exception &e) {
	Log(std::string("** Warning ** : failed to initialize level\n" + e.getMsg()).c_str());
	m_MotoGame.endLevel();
	setState(m_StateAfterPlaying);
	notifyMsg(splitText(e.getMsg(), 50));
	return;
      }

    m_State = GS_PREPLAYING;
    
    PlayerTimeEntry *pBestTime = m_Profiles.getBestTime(m_PlaySpecificLevel);
    PlayerTimeEntry *pBestPTime = m_Profiles.getBestPlayerTime(m_pPlayer->PlayerName,m_PlaySpecificLevel);
    
    std::string T1 = "--:--:--",T2 = "--:--:--";
    if(pBestTime != NULL)
    T1 = formatTime(pBestTime->fFinishTime);
    if(pBestPTime != NULL)
    T2 = formatTime(pBestPTime->fFinishTime);
    
    m_Renderer.setBestTime(T1 + std::string(" / ") + T2);
    m_Renderer.hideReplayHelp();
    
    /* World-record stuff */
#if defined(SUPPORT_WEBACCESS) 
    _UpdateWorldRecord(m_PlaySpecificLevel);
#endif
    /* Prepare level */
    m_Renderer.prepareForNewLevel();
    prestartAnimation_init();
  }
  
  void GameApp::statePrestart_step() {
    prestartAnimation_step();
  }
  
  void GameApp::zoomAnimation1_init() {
    m_fPrePlayStartTime = getRealTime();
    m_fPrePlayStartInitZoom = m_Renderer.getCurrentZoom();  // because the man can change ugly mode while the animation
    m_fPrePlayStartCameraX = m_Renderer.getCameraPositionX();
    m_fPrePlayStartCameraY = m_Renderer.getCameraPositionY();

    m_zoomX = (2.0 * ((float)drawLib->getDispWidth() / (float)drawLib->getDispHeight())) / (m_MotoGame.getLevelSrc()->RightLimit() - m_MotoGame.getLevelSrc()->LeftLimit() + 2*PRESTART_ANIMATION_MARGIN_SIZE);
    m_zoomY = 2.0 /(m_MotoGame.getLevelSrc()->TopLimit() - m_MotoGame.getLevelSrc()->BottomLimit()+2*PRESTART_ANIMATION_MARGIN_SIZE);
		
    if (m_zoomX > m_zoomY){
      float visibleHeight,cameraStartHeight;
			
      m_zoomU=m_zoomX;
      static_time = (m_MotoGame.getLevelSrc()->TopLimit() - m_MotoGame.getLevelSrc()->BottomLimit()) / (2.0/m_zoomU);
			
      visibleHeight = 2.0/m_zoomU;
      cameraStartHeight= visibleHeight/2.0;
			
      m_fPreCameraStartX = (m_MotoGame.getLevelSrc()->RightLimit() + m_MotoGame.getLevelSrc()->LeftLimit())/2;
      m_fPreCameraStartY = m_MotoGame.getLevelSrc()->TopLimit() - cameraStartHeight + PRESTART_ANIMATION_MARGIN_SIZE;
      m_fPreCameraFinalX = (m_MotoGame.getLevelSrc()->RightLimit() + m_MotoGame.getLevelSrc()->LeftLimit())/2;
      m_fPreCameraFinalY = m_MotoGame.getLevelSrc()->BottomLimit() + cameraStartHeight - PRESTART_ANIMATION_MARGIN_SIZE;
			
      if ( fabs(m_fPreCameraStartY - m_fPrePlayStartCameraY) > fabs(m_fPreCameraFinalY - m_fPrePlayStartCameraY)) {
	float f;
	f = m_fPreCameraFinalY;
	m_fPreCameraFinalY = m_fPreCameraStartY;
	m_fPreCameraStartY = f;
      }
			
    }else {
      float visibleWidth,cameraStartLeft;
			
      m_zoomU=m_zoomY;
      static_time = (m_MotoGame.getLevelSrc()->RightLimit() - m_MotoGame.getLevelSrc()->LeftLimit()) / ((2.0 * ((float)drawLib->getDispWidth() / (float)drawLib->getDispHeight()))/m_zoomU);
      
      visibleWidth = (2.0 * ((float)drawLib->getDispWidth() / (float)drawLib->getDispHeight()))/m_zoomU;
      cameraStartLeft = visibleWidth/2.0;
			
      m_fPreCameraStartX = m_MotoGame.getLevelSrc()->RightLimit() - cameraStartLeft + PRESTART_ANIMATION_MARGIN_SIZE;
      m_fPreCameraStartY = (m_MotoGame.getLevelSrc()->BottomLimit() + m_MotoGame.getLevelSrc()->TopLimit())/2;
      m_fPreCameraFinalX = m_MotoGame.getLevelSrc()->LeftLimit() + cameraStartLeft - PRESTART_ANIMATION_MARGIN_SIZE;
      m_fPreCameraFinalY = (m_MotoGame.getLevelSrc()->BottomLimit() + m_MotoGame.getLevelSrc()->TopLimit())/2;
   
      if ( fabs(m_fPreCameraStartX - m_fPrePlayStartCameraX) > fabs(m_fPreCameraFinalX - m_fPrePlayStartCameraX)) {
	float f;
	f = m_fPreCameraFinalX;
	m_fPreCameraFinalX = m_fPreCameraStartX;
	m_fPreCameraStartX = f;
      }
    } 
  }
	
  bool GameApp::zoomAnimation1_step() {
    if(getRealTime() > m_fPrePlayStartTime + static_time + PRESTART_ANIMATION_TIME) {
      return false;
    }
    if(getRealTime() > m_fPrePlayStartTime + static_time){
      float zx, zy, zz;

      zz = logf(PRESTART_ANIMATION_CURVE * ((PRESTART_ANIMATION_TIME + static_time - getRealTime() + m_fPrePlayStartTime) / (PRESTART_ANIMATION_TIME)) + 1.0) / LOGF_PRE_ANIM_TIME_ADDED_ONE * (m_fPrePlayStartInitZoom - m_zoomU);
			
      m_Renderer.setZoom(m_fPrePlayStartInitZoom - zz);
			
      zx = (PRESTART_ANIMATION_TIME + static_time - getRealTime() + m_fPrePlayStartTime)
      / (PRESTART_ANIMATION_TIME) 
      * (m_fPrePlayStartCameraX - m_fPrePlayCameraLastX);
      zy =  (PRESTART_ANIMATION_TIME + static_time - getRealTime() + m_fPrePlayStartTime)
      / (PRESTART_ANIMATION_TIME) 
      * (m_fPrePlayStartCameraY - m_fPrePlayCameraLastY);
		
      m_Renderer.setCameraPosition(m_fPrePlayStartCameraX-zx, m_fPrePlayStartCameraY-zy);
    } else {
      float zx,zy;
        
      m_Renderer.setZoom(m_zoomU);
        
      zx  = (static_time - getRealTime() + m_fPrePlayStartTime) / (static_time) 
      * (m_fPreCameraStartX - m_fPreCameraFinalX); 

      zy = (static_time - getRealTime() + m_fPrePlayStartTime) / (static_time) 
      * (m_fPreCameraStartY - m_fPreCameraFinalY);
				
      m_Renderer.setCameraPosition( m_fPreCameraStartX  - zx, m_fPreCameraStartY - zy);
				
      m_fPrePlayCameraLastX= m_fPreCameraStartX - zx;
      m_fPrePlayCameraLastY= m_fPreCameraStartY - zy;
    }
    return true;
  }

  void GameApp::zoomAnimation1_abort() {
    m_Renderer.setZoom(m_fPrePlayStartInitZoom); // because the man can change ugly mode while the animation
    m_Renderer.setCameraPosition(m_fPrePlayStartCameraX, m_fPrePlayStartCameraY);
  }

  void GameApp::zoomAnimation2_init() {
    zoomAnimation1_init();
    fAnimPlayStartZoom = m_Renderer.getCurrentZoom(); 
    fAnimPlayStartCameraX = m_Renderer.getCameraPositionX();
    fAnimPlayStartCameraY = m_Renderer.getCameraPositionY();
    fAnimPlayFinalZoom = m_zoomU;
    fAnimPlayFinalCameraX1 = m_fPreCameraStartX;
    fAnimPlayFinalCameraY1 = m_fPreCameraStartY;
    fAnimPlayFinalCameraX2 = m_fPreCameraFinalX;
    fAnimPlayFinalCameraY2 = m_fPreCameraFinalY;
    m_fPrePlayStartTime = getRealTime();
  }

  bool GameApp::zoomAnimation2_step() {
    if(getRealTime() > m_fPrePlayStartTime + INPLAY_ANIMATION_TIME) {
      float zx, zy;
      zx = (fAnimPlayFinalCameraX1 - fAnimPlayFinalCameraX2) * (sin((getRealTime() - m_fPrePlayStartTime - INPLAY_ANIMATION_TIME) * 2 * 3.1415927 / INPLAY_ANIMATION_SPEED - 3.1415927/2) + 1) / 2;
      zy = (fAnimPlayFinalCameraY1 - fAnimPlayFinalCameraY2) * (sin((getRealTime() - m_fPrePlayStartTime - INPLAY_ANIMATION_TIME) * 2 * 3.1415927 / INPLAY_ANIMATION_SPEED - 3.1415927/2) + 1) / 2;
      m_Renderer.setCameraPosition(fAnimPlayFinalCameraX1 - zx,fAnimPlayFinalCameraY1 - zy);
      return true;
    }
    if(getRealTime() > m_fPrePlayStartTime){
      float zx, zy, zz, coeff;
      coeff = (getRealTime() - m_fPrePlayStartTime) / (INPLAY_ANIMATION_TIME);
      zx = coeff * (fAnimPlayStartCameraX - fAnimPlayFinalCameraX1);
      zy = coeff * (fAnimPlayStartCameraY - fAnimPlayFinalCameraY1);
      zz = coeff * (fAnimPlayStartZoom - fAnimPlayFinalZoom);
			
      m_Renderer.setZoom(fAnimPlayStartZoom - zz);
      m_Renderer.setCameraPosition(fAnimPlayStartCameraX - zx,fAnimPlayStartCameraY - zy);
    }
		
    return true;
  }

  void GameApp::zoomAnimation2_init_unzoom(){
    m_fPrePlayStartTime = getRealTime();
    fAnimPlayFinalZoom = fAnimPlayStartZoom;
    fAnimPlayStartZoom = m_Renderer.getCurrentZoom();
    fAnimPlayFinalCameraX1 = fAnimPlayStartCameraX;
    fAnimPlayFinalCameraY1 = fAnimPlayStartCameraY;
    fAnimPlayStartCameraX = m_Renderer.getCameraPositionX();
    fAnimPlayStartCameraY = m_Renderer.getCameraPositionY();
  }

  bool GameApp::zoomAnimation2_unstep() {
    if(getRealTime() > m_fPrePlayStartTime + INPLAY_ANIMATION_TIME) {
      return false;
    }
    if(getRealTime() > m_fPrePlayStartTime){
      float zx, zy, zz, coeff;
      coeff = (getRealTime() - m_fPrePlayStartTime) / (INPLAY_ANIMATION_TIME);
      zx = coeff * (fAnimPlayStartCameraX - fAnimPlayFinalCameraX1);
      zy = coeff * (fAnimPlayStartCameraY - fAnimPlayFinalCameraY1);
      zz = coeff * (fAnimPlayStartZoom - fAnimPlayFinalZoom);
			
      m_Renderer.setZoom(fAnimPlayStartZoom - zz);
      m_Renderer.setCameraPosition(fAnimPlayStartCameraX - zx,fAnimPlayStartCameraY - zy);
      return true;
    }
    return false;
  }

  void GameApp::zoomAnimation2_abort() {
    m_Renderer.setZoom(fAnimPlayFinalZoom);
    m_Renderer.setCameraPosition(fAnimPlayFinalCameraX1,fAnimPlayFinalCameraY1);
  }

  void GameApp::prestartAnimation_init() {
    if(m_bPrePlayAnim && m_bEnableInitZoom && m_bUglyMode == false) {
      m_MotoGame.gameMessage(m_MotoGame.getLevelSrc()->Name(), false, PRESTART_ANIMATION_LEVEL_MSG_DURATION);
      zoomAnimation1_init();
    } else {
      setState(GS_PLAYING);
    }
  }
  
  void GameApp::prestartAnimation_step() {
    if(m_bPrePlayAnim && m_bEnableInitZoom && m_bUglyMode == false) {
      if(zoomAnimation1_step() == false) {
        setPrePlayAnim(false); // disable anim
	zoomAnimation1_abort();
        setState(GS_PLAYING);
      }
    } else { /* animation has been rupted */
      setPrePlayAnim(false); // disable anim
      zoomAnimation1_abort();
      setState(GS_PLAYING);
    }
    m_MotoGame.updateGameMessages();
  }

  int GameApp::getNumberOfFinishedLevelsOfPack(LevelsPack *i_pack) {
    int n = 0;
    for(int i=0; i<i_pack->Levels().size(); i++) {
      if(m_Profiles.isLevelCompleted(m_pPlayer->PlayerName, i_pack->Levels()[i]->Id())) {
        n++;
      }
    }
    return n;
  }

  void GameApp::_UpdateLevelsLists() {
    std::string v_levelPack;
    bool v_isset;

    v_isset = (m_pActiveLevelPack != NULL);
    if(v_isset) {
      v_levelPack = m_pActiveLevelPack->Name();

      /* remove reference to levels packs*/
      m_pActiveLevelPack = NULL;
    }
    
    if(m_pPlayer != NULL) {
      m_levelsManager.rebuildPacks(
#if defined(SUPPORT_WEBACCESS)
				   m_pWebHighscores,
				   m_pWebLevels,
#endif
				   m_bDebugMode,
				   m_pPlayer->PlayerName, &m_Profiles, &m_GameStats);
    }    

    if(v_isset) {
      m_pActiveLevelPack = &(m_levelsManager.LevelsPackByName(v_levelPack));
    }

    _UpdateLevelPackList();
    if(v_isset) {
      _UpdateLevelPackLevelList();
    }
    _UpdateLevelLists();

#if defined(SUPPORT_WEBACCESS)
    /* update new levels tab */
    m_pPlayNewLevelsList->clear();
    if(m_pPlayNewLevelsList != NULL) {
      for(int i=0; i<m_levelsManager.NewLevels().size(); i++) {
	m_pPlayNewLevelsList->addLevel(m_levelsManager.NewLevels()[i], m_pPlayer, &m_Profiles, m_pWebHighscores, std::string("New: "));
      }

      for(int i=0; i<m_levelsManager.UpdatedLevels().size(); i++) {
	m_pPlayNewLevelsList->addLevel(m_levelsManager.UpdatedLevels()[i], m_pPlayer, &m_Profiles, m_pWebHighscores, std::string("Updated: "));
      }
    }
#endif
  }

  void GameApp::reloadTheme() {
    try {
      m_theme.load(m_themeChoicer->getFileName(getConfigThemeName(m_themeChoicer)));
    } catch(Exception &e) {
      /* unable to load the theme, load the default one */
      m_theme.load(m_themeChoicer->getFileName(THEME_DEFAULT_THEMENAME));
    }
  }

  void GameApp::setAutoZoom(bool bValue) {
    m_autoZoom = bValue;
  }
  
  void GameApp::setAutoUnZoom(bool bValue) {
    m_autoUnZoom = bValue;
  }
  
  void GameApp::autoZoom() {
    if(m_autoZoom) {
      if(m_bAutoZoomInitialized == false) {
	lockMotoGame(true);
	zoomAnimation2_init();
	m_bAutoZoomInitialized = true;
      } else {
	if(zoomAnimation2_step() == false) {
	  m_autoZoom = false;
	}

	if(m_MotoGame.getNbRemainingStrawberries() > 0) {
	  if(m_Renderer.SizeMultOfEntitiesToTake() <= 5.0) {
	    m_Renderer.setSizeMultOfEntitiesToTake(m_Renderer.SizeMultOfEntitiesToTake() + 0.05);
	  }
	} else {
	  if(m_Renderer.SizeMultOfEntitiesWhichMakeWin() <= 5.0) {
	    m_Renderer.setSizeMultOfEntitiesWhichMakeWin(m_Renderer.SizeMultOfEntitiesWhichMakeWin() + 0.05);
	  }
	}
      }
    } else if (m_autoUnZoom) {
      if(m_bAutoZoomInitialized == true) {
	zoomAnimation2_init_unzoom();
	m_bAutoZoomInitialized = false;
      } else {
	if(zoomAnimation2_unstep() == false) { 
	  m_bAutoZoomInitialized = false;
	  m_autoUnZoom = false;
	  lockMotoGame(false);

	  if(m_MotoGame.getNbRemainingStrawberries() > 0) {
	    m_Renderer.setSizeMultOfEntitiesToTake(1.0);
	  } else {
	    m_Renderer.setSizeMultOfEntitiesWhichMakeWin(1.0);
	  }

	} else {
	  if(m_MotoGame.getNbRemainingStrawberries() > 0) {
	    if(m_Renderer.SizeMultOfEntitiesToTake() > 1.0) {
	      m_Renderer.setSizeMultOfEntitiesToTake(m_Renderer.SizeMultOfEntitiesToTake() - 0.05);
	    }
	  } else {
	    if(m_Renderer.SizeMultOfEntitiesWhichMakeWin() > 1.0) {
	      m_Renderer.setSizeMultOfEntitiesWhichMakeWin(m_Renderer.SizeMultOfEntitiesWhichMakeWin() - 0.05);
	    }
	  }
	}
      }
    }
  }

  void GameApp::lockMotoGame(bool bLock) {
    m_bLockMotoGame = bLock;
  }
  
  bool GameApp::isLockedMotoGame() const {
    return m_bLockMotoGame;
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
    m_Renderer.initCamera();
    m_MotoGame.addPenalityTime(900); /* 15 min of penality for that ! */
  }

  void GameApp::loadLevelHook(std::string i_level, int i_percentage) {
    std::ostringstream v_percentage;
    v_percentage << i_percentage;
    v_percentage << "%";
    
    if(m_reloadingLevelsUser == false) {
      Texture *pLoadingScreen;
      Sprite* pSprite;
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "Loading");
      
      if(pSprite == NULL) {
	return;
      }
      
      try {
	pLoadingScreen = pSprite->getTexture(false, true);
      } catch(Exception &e) {
	return;
      }
      
      _UpdateLoadingScreen((1.0f/9.0f) * 4,pLoadingScreen, std::string(GAMETEXT_INDEX_CREATION) + std::string("\n") + v_percentage.str() + std::string(", ") + i_level);
    } else {
      _SimpleMessage(GAMETEXT_RELOADINGLEVELS + std::string("\n") + v_percentage.str(), &m_InfoMsgBoxRect);
    }
  }

  bool GameApp::creditsModeActive() {
    return m_bCreditsModeActive;
  }
}
