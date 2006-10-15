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

#if defined(SUPPORT_WEBACCESS)
  #include <curl/curl.h>
#endif

namespace vapp {

  /* CRY! */
  extern InputHandler *m_pActiveInputHandler;
  
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
    _CreateLevelLists((UILevelList *)m_pLevelPacksWindow->getChild("LEVELPACK_TABS:ALLLEVELS_TAB:ALLLEVELS_LIST"));

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
    
    /* Always clear context when changing state */
    m_Renderer.getGUI()->clearContext();
    
    switch(s) {
      case GS_LEVELPACK_VIEWER: {
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
	      try {
	        std::string LevelID;

	        //SDL_ShowCursor(SDL_DISABLE);
	        m_bShowCursor = false;
	        bool bCreditsMode = (m_State == GS_CREDITSMODE);
	        m_bCreditsModeActive = bCreditsMode;
	        m_State = GS_REPLAYING;
      	  
	        /* Open a replay for input */
	        if(m_pReplay != NULL) delete m_pReplay;
	        m_pReplay = new Replay;

	        LevelID = m_pReplay->openReplay(m_PlaySpecificReplay,&m_fCurrentReplayFrameRate,m_ReplayPlayerName);
	        if(LevelID == "") {
	          Log("** Warning ** : No valid level identifier could be extracted from the replay: %s",m_PlaySpecificReplay.c_str());
	          char cBuf[256];
	          sprintf(cBuf,GAMETEXT_REPLAYNOTFOUND,m_PlaySpecificReplay.c_str());
	          setState(GS_MENU);
	          notifyMsg(cBuf);
	          // throw Exception("invalid replay");
	        }
	        else {
	          /* Credits mode? */
	          if(bCreditsMode) {
	            if(m_pCredits == NULL)
            		m_pCredits = new Credits;
	      
	            m_pCredits->init(m_pReplay->getFinishTime(),4,4,GAMETEXT_CREDITS);
	          }
	    
	          /* Fine, open the level */
	          LevelSrc *pLevelSrc = _FindLevelByID(LevelID);
	          if(pLevelSrc == NULL) {
	            Log("** Warning ** : level '%s' specified by replay '%s' not found",LevelID.c_str(),m_PlaySpecificReplay.c_str());
      	      
	            char cBuf[256];
	            sprintf(cBuf,GAMETEXT_LEVELREQUIREDBYREPLAY,LevelID.c_str());
	            setState(GS_MENU);
	            notifyMsg(cBuf);                        
	          }
	          else if(pLevelSrc->isXMotoTooOld()) {
	            Log("** Warning ** : level '%s' specified by replay '%s' requires newer X-Moto",LevelID.c_str(),m_PlaySpecificReplay.c_str());
      	      
	            char cBuf[256];
	            sprintf(cBuf,GAMETEXT_NEWERXMOTOREQUIRED,pLevelSrc->getRequiredVersion().c_str());
	            setState(GS_MENU);
	            notifyMsg(cBuf);                        
	          }
	          else {    
	            /* Init level */    
	            m_InputHandler.resetScriptKeyHooks();                                   
	            m_MotoGame.prePlayLevel(NULL, pLevelSrc, NULL, true);
	            m_nFrame = 0;
	            m_Renderer.prepareForNewLevel(bCreditsMode);            
      	      
	            /* Show help string */
	            if(!isNoGraphics()) {
		            PlayerTimeEntry *pBestTime = m_Profiles.getBestTime(LevelID);
		            PlayerTimeEntry *pBestPTime = m_Profiles.getBestPlayerTime(m_pPlayer->PlayerName,LevelID);
            		
		            std::string T1 = "--:--:--",T2 = "--:--:--";
            		
		            if(pBestTime != NULL)
		              T1 = formatTime(pBestTime->fFinishTime);
		            if(pBestPTime != NULL)
		              T2 = formatTime(pBestPTime->fFinishTime);
            		
		            m_Renderer.setBestTime(T1 + std::string(" / ") + T2);
		            m_Renderer.showReplayHelp(m_pReplay->getSpeed(),!_IsReplayScripted(m_pReplay));
		
             		if(m_bBenchmark || bCreditsMode) m_Renderer.setBestTime("");
		
#if defined(SUPPORT_WEBACCESS) 
		            /* World-record stuff */
		            if(!bCreditsMode)
		              _UpdateWorldRecord(LevelID);
#endif
      	      }
	      
	            m_fStartTime = getRealTime();
	          }          
	        }
	      } catch(Exception &e) {
	        setState(GS_MENU);
	        notifyMsg(splitText(e.getMsg(), 50));	  
	      }
        break;
      }  
      case GS_MENU: {
        //SDL_ShowCursor(SDL_ENABLE);
        m_bShowCursor = true;                
        
        /* Any replays to get rid off? */
        if(m_pReplay != NULL) delete m_pReplay;
        m_pReplay = NULL;

        /* The main menu, the one which is entered initially when the game 
           begins. */
        m_pMainMenu->showWindow(true);

        // enable the preplay animation
        setPrePlayAnim(true);
                  
        break;
      }
      case GS_PREPLAYING: {
        statePrestart_init();
        break;
      }
      case GS_PLAYING: {
        LevelSrc *pLevelSrc = _FindLevelByID(m_PlaySpecificLevel);
        if(pLevelSrc != NULL) {
          m_MotoGame.playLevel(m_pGhostReplay, pLevelSrc, false);
          m_State = GS_PLAYING;        
          m_nFrame = 0;
        } else {
          Log("** Warning ** : level '%s' not found",m_PlaySpecificLevel.c_str());
          char cBuf[256];
          sprintf(cBuf,GAMETEXT_LEVELNOTFOUND,m_PlaySpecificLevel.c_str());
          setState(GS_MENU);
          notifyMsg(cBuf);
        }
        break;
      }
      case GS_PAUSE: {
//        SDL_ShowCursor(SDL_ENABLE);
        m_bShowCursor = true;

        /* Paused from GS_PLAYING */
        break;
      }
      case GS_DEADJUST: {
        /* Finish replay */
        if(m_pReplay != NULL) m_pReplay->finishReplay(false,0.0f);

        /* Update stats */        
        m_GameStats.died(m_pPlayer->PlayerName,m_MotoGame.getLevelSrc()->getID(),m_MotoGame.getLevelSrc()->getLevelInfo()->Name,m_MotoGame.getTime());
                
        /* Play the DIE!!! sound */
        Sound::playSampleByName("Sounds/Headcrash.ogg",0.3);

	m_nJustDeadShade = 0;
	m_MotoGame.gameMessage(GAMETEXT_JUSTDEAD_RESTART,     false, 15);
	m_MotoGame.gameMessage(GAMETEXT_JUSTDEAD_DISPLAYMENU, false, 15);

        break;
      }
    case GS_DEADMENU: {
      m_bShowCursor = true;
      m_pJustDeadMenu->showWindow(true);

      /* Possible exit of GS_PLAYING, when the player is dead */
      m_fCoolDownEnd = getRealTime() + 0.3f;
      break;
    }
      case GS_EDIT_PROFILES: {
//        SDL_ShowCursor(SDL_ENABLE);
        m_bShowCursor = true;

        /* The profile editor can work on top of the main menu, or as init
           state when there is no player profiles available */
        m_pProfileEditor->showWindow(true);
        break;
      }

#if defined(SUPPORT_WEBACCESS)
      case GS_EDIT_WEBCONFIG: {
        m_bShowCursor = true;
        if(m_pWebConfMsgBox != NULL) delete m_pWebConfMsgBox;
        m_pWebConfEditor->showWindow(false);
        m_pWebConfMsgBox = m_Renderer.getGUI()->msgBox(GAMETEXT_ALLOWINTERNETCONN,
                                                       (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
        break;
      }
#endif
      case GS_FINISHED: {
//        SDL_ShowCursor(SDL_ENABLE);
        m_bShowCursor = true;

        /* Finish replay */
        if(m_pReplay != NULL) m_pReplay->finishReplay(true,m_MotoGame.getFinishTime());

        /* Update stats */        
        m_GameStats.levelCompleted(m_pPlayer->PlayerName,m_MotoGame.getLevelSrc()->getID(),m_MotoGame.getLevelSrc()->getLevelInfo()->Name,m_MotoGame.getFinishTime());
        
        /* A more lucky outcome of GS_PLAYING than GS_DEADMENU :) */
        m_pFinishMenu->showWindow(true);
        m_pBestTimes->showWindow(true);
        m_nFinishShade = 0;            

        /* display message on finish and eventually save the replay */
        
        /* is it a highscore ? */
        float v_best_local_time;
        float v_best_personal_time;
        float v_current_time;
        bool v_is_a_highscore;
        bool v_is_a_personal_highscore;
        
        v_best_local_time = m_Profiles.getBestTime(m_MotoGame.getLevelSrc()->getID())->fFinishTime;
        v_best_personal_time = m_Profiles.getBestPlayerTime(m_pPlayer->PlayerName,
                        m_MotoGame.getLevelSrc()->getID())->fFinishTime;
        v_current_time = m_MotoGame.getFinishTime();

        v_is_a_highscore = (v_current_time <= v_best_local_time);  /* = because highscore is already stored in playerdata */

        v_is_a_personal_highscore = (v_current_time <= v_best_personal_time);  /* = because highscore is already stored in playerdata */

#if defined(SUPPORT_WEBACCESS) 
        /* search a better webhighscore */
        if(m_pWebHighscores != NULL /*&& v_is_a_highscore == true*/) {
          WebHighscore* wh = m_pWebHighscores->getHighscoreFromLevel(m_MotoGame.getLevelSrc()->getID());
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
          Sound::playSampleByName("Sounds/NewHighscore.ogg");

#if defined(SUPPORT_WEBACCESS)
        // enable upload button
        if(m_bEnableWebHighscores) {
          if(m_pReplay != NULL) {
            for(int i=0;i<m_nNumFinishMenuButtons;i++) {
              if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_UPLOAD_HIGHSCORE) {
                m_pFinishMenuButtons[i]->enableWindow(true);
              }
            }
          }
        }
#endif
    
        if(m_pReplay != NULL && m_bAutosaveHighscoreReplays) {
          String v_replayName = Replay::giveAutomaticName();
          _SaveReplay(v_replayName);
          m_Renderer.showMsgNewBestHighscore(v_replayName);
        } else {
          m_Renderer.showMsgNewBestHighscore();
        } /* ok i officially give up on indention in x-moto :P */
        } else {
          if(v_is_a_personal_highscore) { /* personal highscore */
            Sound::playSampleByName("Sounds/NewHighscore.ogg");
            if(m_pReplay != NULL && m_bAutosaveHighscoreReplays) {
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
        break;
      }
    }
        
    m_fLastPhysTime = getTime() - PHYS_STEP_SIZE;
  }

  std::string GameApp::getConfigThemeName(ThemeChoicer *p_themeChoicer) {
    if(p_themeChoicer->ExistThemeName(m_currentThemeName)) {
      return m_currentThemeName;
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
      
    /* Show mini map? */
    m_bShowMiniMap = m_Config.getBool("ShowMiniMap");
    
    /* show engine counter */
    m_bShowEngineCounter = m_Config.getBool("ShowEngineCounter");
    
    /* Replay stuff */
    m_fReplayFrameRate = m_Config.getFloat("ReplayFrameRate");
    m_bRecordReplays = m_Config.getBool("StoreReplays");
    m_bCompressReplays = m_Config.getBool("CompressReplays");
    Replay::enableCompression(m_bCompressReplays);
    m_bAutosaveHighscoreReplays = m_Config.getBool("AutosaveHighscoreReplays");

#if defined(ALLOW_GHOST)
    /* ghost */
    m_bEnableGhost        = m_Config.getBool("EnableGhost");
    m_bShowGhostTimeDiff  = m_Config.getBool("ShowGhostTimeDiff");
    m_MotoGame.setShowGhostTimeDiff(m_bShowGhostTimeDiff);
    m_GhostSearchStrategy = (enum GhostSearchStrategy) m_Config.getInteger("GhostSearchStrategy");
    m_bGhostMotionBlur = m_Config.getBool("GhostMotionBlur");
    m_Renderer.setGhostMotionBlur( m_bGhostMotionBlur );
#endif

#if defined(SUPPORT_WEBACCESS)
    m_bEnableWebHighscores = m_Config.getBool("WebHighscores") && isNoWWW()== false;
    m_bShowWebHighscoreInGame = m_Config.getBool("ShowInGameWorldRecord");
    m_bEnableGhostInfo = m_Config.getBool("DisplayGhostInfo");
    m_bEnableCheckNewLevelsAtStartup  = m_Config.getBool("CheckNewLevelsAtStartup");
    m_bEnableCheckHighscoresAtStartup = m_Config.getBool("CheckHighscoresAtStartup");
#endif

    /* Other settings */
    m_bEnableEngineSound = m_Config.getBool("EngineSoundEnable");
    m_bEnableContextHelp = m_Config.getBool("ContextHelp");
    m_bEnableMenuMusic = m_Config.getBool("MenuMusic");
    m_currentThemeName   = m_Config.getString("Theme");
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
        drawImage(Vector2f(0,0),Vector2f(getDispWidth()/2,getDispHeight()/2),m_pTitleTL);
      if(m_pTitleTR != NULL)
        drawImage(Vector2f(getDispWidth()/2,0),Vector2f(getDispWidth(),getDispHeight()/2),m_pTitleTR);
      if(m_pTitleBR != NULL)
        drawImage(Vector2f(getDispWidth()/2,getDispHeight()/2),Vector2f(getDispWidth(),getDispHeight()),m_pTitleBR);
      if(m_pTitleBL != NULL)
        drawImage(Vector2f(0,getDispHeight()/2),Vector2f(getDispWidth()/2,getDispHeight()),m_pTitleBL);
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
    Img *pShot = grabScreen();      
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
  void GameApp::keyDown(int nKey,int nChar) {
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
    
    /* If message box... */
    if(m_pQuitMsgBox) {
      if(nKey == SDLK_ESCAPE) {
        delete m_pQuitMsgBox;
        m_pQuitMsgBox = NULL;
      }    
      else
        m_Renderer.getGUI()->keyDown(nKey,nChar);      
      return;
    }
    else if(m_pNotifyMsgBox) {
      if(nKey == SDLK_ESCAPE) {
        delete m_pNotifyMsgBox;
        m_pNotifyMsgBox = NULL;
      }    
      else
        m_Renderer.getGUI()->keyDown(nKey,nChar);      
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
        m_Renderer.getGUI()->keyDown(nKey,nChar);
        break;
      }
      case GS_PAUSE:
        switch(nKey) {
          case SDLK_ESCAPE:
            /* Back to the game, please */
            m_pPauseMenu->showWindow(false);
            m_State = GS_PLAYING;
            break;
          default:
            m_Renderer.getGUI()->keyDown(nKey,nChar);
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
#if defined(ALLOW_GHOST) 
        /* hide ghost */
        m_MotoGame.setGhostActive(false);
#endif 
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
               
              m_Renderer.getGUI()->keyDown(nKey,nChar);
            }
            break;
          default:
            if(m_State == GS_DEADMENU)
              if(getRealTime() < m_fCoolDownEnd)
                break;
             
            m_Renderer.getGUI()->keyDown(nKey,nChar);
            break;      
        }
        break;
      case GS_REPLAYING:
        switch(nKey) {
          case SDLK_ESCAPE:
            /* Escape quits the replay */
            m_MotoGame.endLevel();
            m_InputHandler.resetScriptKeyHooks();                      
            m_Renderer.unprepareForNewLevel();
            setState(GS_MENU);            
            break;          
          case SDLK_RIGHT:
            /* Right arrow key: fast forward */
            if(m_pReplay != NULL)
              m_pReplay->fastforward(1);
            break;
          case SDLK_LEFT:
            /* Left arrow key: rewind */
              if(_IsReplayScripted(m_pReplay) == false) {
                m_Renderer.clearAllParticles();
                m_Renderer.skipBackTime(1);
                m_pReplay->fastrewind(1);
              }
            break;
  case SDLK_SPACE:
    /* pause */
    if(m_pReplay != NULL) {
      m_pReplay->pause();
      m_Renderer.showReplayHelp(m_pReplay->getSpeed(),
              ! _IsReplayScripted(m_pReplay)); /* update help */
    }
    break;
  case SDLK_UP:
    /* faster */
    if(m_pReplay != NULL) {
      m_pReplay->faster();
      m_Renderer.showReplayHelp(m_pReplay->getSpeed(),
              ! _IsReplayScripted(m_pReplay)); /* update help */
    }
    break;
  case SDLK_DOWN:
    /* slower */
    if(_IsReplayScripted(m_pReplay) == false ||
       m_pReplay->getSpeed() >= REPLAY_SPEED_INCREMENT) {
      m_pReplay->slower();
      m_Renderer.showReplayHelp(m_pReplay->getSpeed(),
              ! _IsReplayScripted(m_pReplay)); /* update help */
    }
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
    /* Escape pauses */
    setState(GS_PAUSE);
    m_pPauseMenu->showWindow(true);
    m_nPauseShade = 0;
    break;
  case SDLK_RETURN:
    /* retart immediatly the level */
    _RestartLevel();
    break;
          default:
            /* Notify the controller */
            m_InputHandler.handleInput(INPUT_KEY_DOWN,nKey,m_MotoGame.getBikeController(), &m_Renderer);
        }
        break; 
    }
  }

  /*===========================================================================
  Key up event
  ===========================================================================*/
  void GameApp::keyUp(int nKey) {
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
        m_Renderer.getGUI()->keyUp(nKey);
        break;
      case GS_PLAYING:
        /* Notify the controller */
        m_InputHandler.handleInput(INPUT_KEY_UP,nKey,m_MotoGame.getBikeController(), &m_Renderer);
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
      m_InputHandler.handleInput(INPUT_KEY_DOWN,nButton,m_MotoGame.getBikeController(), &m_Renderer);
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
        m_InputHandler.handleInput(INPUT_KEY_UP,nButton,m_MotoGame.getBikeController(), &m_Renderer);
        break;
      case GS_DEADJUST:
      break;
    }
  }
    
  /*===========================================================================
  Find a level by ID
  ===========================================================================*/
  LevelSrc *GameApp::_FindLevelByID(std::string ID) {
    /* Look through all level sources... */
    //printf("LOOKING FOR [%s]\n",ID.c_str());
    for(int i=0;i<m_nNumLevels;i++) {
      //printf("  .. is it [%s]?\n",m_Levels[i].getID().c_str());
      if(m_Levels[i].getID() == ID) return &m_Levels[i];
    }
    return NULL; /* nothing */
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

  /*===========================================================================
  Level packs
  ===========================================================================*/
  void GameApp::_UpdateLevelPackManager(LevelSrc *pLevelSrc) {
      /* Already a known level pack? */
      LevelPack *pPack = _FindLevelPackByName(pLevelSrc->getLevelPack());
      if(pPack == NULL) {
        /* No, register it */
        pPack = new LevelPack;
        pPack->Name = pLevelSrc->getLevelPack();
        m_LevelPacks.push_back(pPack);
        
        /* Set default hints */
        pPack->bShowTimes = true;
        pPack->bShowWebTimes = true;
        
        /* Try to find a hints file for this level pack */
        std::vector<std::string> LpkFiles = FS::findPhysFiles("Levels/*.lpk",true);
        for(int i=0;i<LpkFiles.size();i++) {
          XMLDocument XML; 
          XML.readFromFile(LpkFiles[i]);
          TiXmlDocument *pDoc = XML.getLowLevelAccess();
          
          if(pDoc != NULL) {
            TiXmlElement *pLpkHintsElem = pDoc->FirstChildElement("lpkhints");
            if(pLpkHintsElem != NULL) {
              /* For this level pack? */
              const char *pcFor = pLpkHintsElem->Attribute("for");
              if(pcFor != NULL && pPack->Name == pcFor) {
                /* Yup. Extract hints */
                for(TiXmlElement *pHintElem = pLpkHintsElem->FirstChildElement("hint");
                    pHintElem != NULL; pHintElem=pHintElem->NextSiblingElement("hint")) {
                  /* Check for known hints... */
                  const char *pc;

                  pc = pHintElem->Attribute("show_times");
                  if(pc != NULL) {
                    pPack->bShowTimes = atoi(pc)==1;
                  }

                  pc = pHintElem->Attribute("show_wtimes");
                  if(pc != NULL) {
                    pPack->bShowWebTimes = atoi(pc)==1;
                  }
                }
              }              
            }
          }
        }
      }
      
      /* Add level to pack */
      pPack->Levels.push_back(pLevelSrc);
  }

  LevelPack *GameApp::_FindLevelPackByName(const std::string &Name) {
    /* Look for level pack in list */
    for(int i=0;i<m_LevelPacks.size();i++) {
      if(m_LevelPacks[i]->Name == Name) return m_LevelPacks[i];
    }
    return NULL;
  }
 
  std::string GameApp::_DetermineNextLevel(LevelSrc *pLevelSrc) {
    UIList *pList = NULL;

    UIList *pPlayNewLevelsList = (UIList *)m_pLevelPacksWindow->getChild("LEVELPACK_TABS:NEWLEVELS_TAB:NEWLEVELS_LIST");
    UIList *pPlayAllLevelsList = (UIList *)m_pLevelPacksWindow->getChild("LEVELPACK_TABS:ALLLEVELS_TAB:ALLLEVELS_LIST");
    UIWindow *pPlayNewLevelsTab = (UIList *)m_pLevelPacksWindow->getChild("LEVELPACK_TABS:NEWLEVELS_TAB");
    UIWindow *pPlayAllLevelsTab = (UIList *)m_pLevelPacksWindow->getChild("LEVELPACK_TABS:ALLLEVELS_TAB");

    if(pPlayNewLevelsTab->isHidden() == false) {
      pList = pPlayNewLevelsList;
    }
    if(pPlayAllLevelsTab->isHidden() == false) {
      pList = pPlayAllLevelsList;
    }
    
    /* use lists */
    if(pList != NULL) {
      for(int i=0;i<pList->getEntries().size()-1;i++) {
	if(pList->getEntries()[i]->pvUser == (void *)pLevelSrc) {
	  return ((LevelSrc *)pList->getEntries()[i+1]->pvUser)->getID();
	}
      }
      return "";
    }

    /* no list opened, find the next one of the pack */
    LevelPack *pPack = _FindLevelPackByName(pLevelSrc->getLevelPack());
    if(pPack != NULL) {
      /* Determine next then */
      for(int j=0; j<pPack->Levels.size()-1; j++) {
	if(pPack->Levels[j] == pLevelSrc) {
	  return pPack->Levels[j+1]->getID();
	}
      }
      return "";
    }

    return "";
  }
  
  bool GameApp::_IsThereANextLevel(LevelSrc *pLevelSrc) {
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
      _SimpleMessage(GAMETEXT_DLHIGHSCORES,&m_DownloadMsgBoxRect);
    }

    m_bWebHighscoresUpdatedThisSession = true;
    
    /* Try downloading the highscores */
    m_pWebHighscores->setWebsiteURL(m_Config.getString("WebHighscoresURL"));
    m_pWebHighscores->update();
  }
#endif

#if defined(SUPPORT_WEBACCESS)  
  void GameApp::_UpdateWebLevels(bool bSilent) {
    if(!bSilent) {
      _SimpleMessage(GAMETEXT_DLLEVELSCHECK,&m_DownloadMsgBoxRect);
    }

    /* Try download levels list */
    if(m_pWebLevels == NULL) {
      m_pWebLevels = new WebLevels(this,&m_ProxySettings);
    }
    m_pWebLevels->setURL(m_Config.getString("WebLevelsURL"));
    Log("WWW: Checking for new or updated levels...");
    m_pWebLevels->update();
    
    int nULevels=0,nUBytes=0;
    m_pWebLevels->getUpdateInfo(&nUBytes,&nULevels);
    m_bWebLevelsToDownload = nULevels!=0;
  }
#endif

#if defined(SUPPORT_WEBACCESS)  
  void GameApp::_UpdateWebThemes(bool bSilent) {
    if(!bSilent) {
      _SimpleMessage(GAMETEXT_DLTHEMESLISTCHECK,&m_DownloadMsgBoxRect);
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
      _SimpleMessage(GAMETEXT_DLROOMSLISTCHECK,&m_DownloadMsgBoxRect);
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
  
          if(m_pDownloadMsgBox != NULL) {
            delete m_pDownloadMsgBox;
            m_pDownloadMsgBox = NULL;
          }
          notifyMsg(GAMETEXT_FAILEDDLLEVELS);
          return;
        }

        /* Got some new levels... load them! */
        const std::vector<std::string> LvlFiles = m_pWebLevels->getNewDownloadedLevels();
        
        Log("Loading new levels...");
        int nOldNum = m_nNumLevels;
        _LoadLevels(LvlFiles);
        Log(" %d new level%s loaded",m_nNumLevels-nOldNum,(m_nNumLevels-nOldNum)==1?"":"s");
        
        /* Add new levels to GUI list */
        if(m_pPlayNewLevelsList != NULL) {
          for(int i=nOldNum;i<m_nNumLevels;i++) {
            m_pPlayNewLevelsList->addLevel(m_Levels+i, m_pPlayer, &m_Profiles, m_pWebHighscores, std::string("New: "));
          }
        }
        
        /* Updated levels? */
        const std::vector<std::string> UpdatedLvlFiles = m_pWebLevels->getUpdatedDownloadedLevels();
        
        Log("Reloading updated levels...");
        int nReloaded = 0;
        int nTooOldXMoto = 0;
        
        for(int i=0;i<UpdatedLvlFiles.size();i++) {
          try {
            /* Find levels by file names */
            for(int j=0;j<m_nNumLevels;j++) {
              if(m_Levels[j].getFileName() == UpdatedLvlFiles[i]) {
                /* Found it... */
                m_Levels[j].load(m_bEnableLevelCache);
                
                /* Failed to load due to old xmoto? */  
                if(m_Levels[j].isXMotoTooOld())
                  nTooOldXMoto++;
                              
                /* Add it to list of new levels as "updated" */
                if(m_pPlayNewLevelsList != NULL) {
                  m_pPlayNewLevelsList->addLevel(m_Levels+j, m_pPlayer, &m_Profiles, m_pWebHighscores, std::string("Updated: "));
                }
                
                nReloaded++;
                break;
              }
            }
          }
          catch(Exception &e) {
            Log("** Warning ** : Problem updating '%s' (%s)",UpdatedLvlFiles[i].c_str(),e.getMsg().c_str());            
          }
        }
        
        Log(" %d reloaded",nReloaded);        
        if(nTooOldXMoto > 0)
          Log(" %d not reloaded (%d due to outdated X-Moto)",UpdatedLvlFiles.size() - nReloaded,nTooOldXMoto);        
        else
          Log(" %d not reloaded",UpdatedLvlFiles.size() - nReloaded);        
        
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
          if(m_pDownloadMsgBox == NULL) {
            char cBuf[256];
            
            sprintf(cBuf,nULevels==1?GAMETEXT_NEWLEVELAVAIL:
                                     GAMETEXT_NEWLEVELSAVAIL,nULevels);
            m_pDownloadMsgBox = m_Renderer.getGUI()->msgBox(cBuf,(UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
          }
        }
      } 
      catch(Exception &e) {
        Log("** Warning ** : Unable to check for extra levels [%s]",e.getMsg().c_str());
        if(m_pDownloadMsgBox != NULL) {
          delete m_pDownloadMsgBox;
          m_pDownloadMsgBox = NULL;
        }
        notifyMsg(GAMETEXT_FAILEDCHECKLEVELS);
      } 
  }
#endif  

  void GameApp::_RestartLevel() {
    /* Update stats */        
    m_GameStats.levelRestarted(m_pPlayer->PlayerName,m_MotoGame.getLevelSrc()->getID(),m_MotoGame.getLevelSrc()->getLevelInfo()->Name,m_MotoGame.getTime());
  
#if defined(ALLOW_GHOST) 
    /* hide ghost */
    m_MotoGame.setGhostActive(false);
#endif
    m_MotoGame.endLevel();
    m_InputHandler.resetScriptKeyHooks();           
    m_Renderer.unprepareForNewLevel();
    setState(GS_PREPLAYING);   
  }

  /*===========================================================================
  WWWAppInterface implementation
  ===========================================================================*/
#if defined(SUPPORT_WEBACCESS)
        
  bool GameApp::shouldLevelBeUpdated(const std::string &LevelID) {
    /* Hmm... ask user whether this level should be updated */
    bool bRet = true;
    
    LevelSrc *pLevel = _FindLevelByID(LevelID);
    if(pLevel != NULL) {
      bool bDialogBoxOpen = true;
      
      char cBuf[1024];
      sprintf(cBuf,(std::string(GAMETEXT_WANTTOUPDATELEVEL) + "\n(%s)").c_str(),pLevel->getLevelInfo()->Name.c_str(),
              pLevel->getFileName().c_str());
      UIMsgBox *pMsgBox = m_Renderer.getGUI()->msgBox(cBuf,(UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
      
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
          if(Button != UI_MSGBOX_YES) {
            bRet = false;
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
    drawImage(Vector2f(nMX-2,nMY-2),Vector2f(nMX+30,nMY+30),m_pCursor);
  }

        SDL_GL_SwapBuffers();            
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
    _SimpleMessage(m_DownloadingMessage,&m_DownloadMsgBoxRect,true);
    
    drawBox(Vector2f(m_DownloadMsgBoxRect.nX+10,m_DownloadMsgBoxRect.nY+
                                                   m_DownloadMsgBoxRect.nHeight-
                                                   nBarHeight*2),
            Vector2f(m_DownloadMsgBoxRect.nX+m_DownloadMsgBoxRect.nWidth-10,
                     m_DownloadMsgBoxRect.nY+m_DownloadMsgBoxRect.nHeight-nBarHeight),
            0,MAKE_COLOR(0,0,0,255),0);
            
                
    drawBox(Vector2f(m_DownloadMsgBoxRect.nX+10,m_DownloadMsgBoxRect.nY+
                                                   m_DownloadMsgBoxRect.nHeight-
                                                   nBarHeight*2),
            Vector2f(m_DownloadMsgBoxRect.nX+10+((m_DownloadMsgBoxRect.nWidth-20)*(int)fPercent)/100,
                     m_DownloadMsgBoxRect.nY+m_DownloadMsgBoxRect.nHeight-nBarHeight),
            0,MAKE_COLOR(255,0,0,255),0);

    UIFont *v_font = m_Renderer.getSmallFont();
    if(v_font != NULL) {
      UITextDraw::printRaw(v_font,m_DownloadMsgBoxRect.nX+13,m_DownloadMsgBoxRect.nY+
         m_DownloadMsgBoxRect.nHeight-nBarHeight-4,m_DownloadingInformation,MAKE_COLOR(255,255,255,128));
    }
    SDL_GL_SwapBuffers();            
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
  
  bool GameApp::doesLevelExist(const std::string &LevelID) {
    return _FindLevelByID(LevelID) != NULL;
  }
  
  std::string GameApp::levelPathForUpdate(const std::string &p_LevelId) {
    LevelSrc *pLevel = _FindLevelByID(p_LevelId);
    if(pLevel != NULL) {
      /* If level path is not absolute, this level is not updatable */
      if(FS::isPathAbsolute(pLevel->getFileName()))
        return pLevel->getFileName();
    }
    return "";
  }
  
  std::string GameApp::levelMD5Sum(const std::string &LevelID) {
    LevelSrc *pLevel = _FindLevelByID(LevelID);
    if(pLevel != NULL) {
      return pLevel->getLevelCheckSum();
    }

    /* Nothing */
    return "";
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

    bool GameApp::_IsReplayScripted(Replay *p_pReplay) {
      LevelSrc *pLevelSrc;

      if(p_pReplay != NULL) {
  pLevelSrc = _FindLevelByID(p_pReplay->getLevelId());
  if(pLevelSrc != NULL) {
    return pLevelSrc->isScripted();
  }
      }
      return false; /* throw exception ? */
    }

#if defined(ALLOW_GHOST)
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
	      _SimpleMessage(GAMETEXT_DLGHOST,&m_DownloadMsgBoxRect);
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
#endif    

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
    m_pActiveInputHandler = &m_InputHandler;
      
    /* Default playing state */
    m_fLastFrameTime = 0.0f;
    m_fLastPerfStateTime = 0.0f;
    m_fLastStateSerializationTime = -100.0f; /* loong time ago :) */
      
    /* We need a profile */
    if(m_pPlayer == NULL) {
      Log("** Warning ** : no player profile selected, use -profile option");
      throw Exception("no player");
    }
      
    /* Find the level */
    LevelSrc *pLevelSrc = _FindLevelByID(m_PlaySpecificLevel);
    if(pLevelSrc == NULL) {
      Log("** Warning ** : level '%s' not found",m_PlaySpecificLevel.c_str());
  
      char cBuf[256];
      sprintf(cBuf,GAMETEXT_LEVELNOTFOUND,m_PlaySpecificLevel.c_str());
      setState(GS_MENU);
      notifyMsg(cBuf);
      //          throw Exception("no level");
    }
    else if(pLevelSrc->isXMotoTooOld()) {
      Log("** Warning ** : level '%s' requires newer X-Moto",m_PlaySpecificLevel.c_str());
  
      char cBuf[256];
      sprintf(cBuf,GAMETEXT_NEWERXMOTOREQUIRED,pLevelSrc->getRequiredVersion().c_str());
      setState(GS_MENU);
      notifyMsg(cBuf);                        
    }
    else {
#if defined(ALLOW_GHOST)
      m_MotoGame.setGhostActive(false);

      /* Ghost replay */
      if(m_bEnableGhost) {
	std::string v_PlayGhostReplay;
	v_PlayGhostReplay = _getGhostReplayPath(pLevelSrc->getID(),  m_GhostSearchStrategy);
    
	if(v_PlayGhostReplay != "") {
	  std::string v_GhostReplayPlayerName;
	  float v_ghostReplayFrameRate;
	  std::string GhostLevelID;
      
	  /* hope the man will not replace the existing replay */
	  /* i want just to be sure that levelid is the same */
	  String v_levelIdInGhost = "";
	  if(m_pGhostReplay != NULL) {
	    v_levelIdInGhost = m_pGhostReplay->getLevelId();
	  }

	  if(m_lastGhostReplay != v_PlayGhostReplay || 
	     v_levelIdInGhost != pLevelSrc->getID()) {
        
	    if(m_pGhostReplay != NULL) delete m_pGhostReplay;
	    m_pGhostReplay = new Replay;
        
	    try {
	      GhostLevelID = m_pGhostReplay->openReplay(v_PlayGhostReplay,&v_ghostReplayFrameRate,v_GhostReplayPlayerName);
	      if(GhostLevelID != "") {
		m_lastGhostReplay = v_PlayGhostReplay;
		m_Renderer.setGhostReplay(m_pGhostReplay);
	      } else {
		/* bad replay */
		delete m_pGhostReplay;
		m_pGhostReplay = NULL;
	      }
	    } catch(Exception &e) {
	      /* bad replay */
	      delete m_pGhostReplay;
	      m_pGhostReplay = NULL;
	    }        

	  } else {
	    /* if last ghost loaded has the same filename, don't reload it */
	    GhostLevelID = pLevelSrc->getID();
	    m_pGhostReplay->reinitialize();
	  }
      
	  if(GhostLevelID != "") {
	    m_nGhostFrame = 0;
	    m_MotoGame.setGhostActive(true);
	    /* read first state */
	    static SerializedBikeState GhostBikeState;
	    m_pGhostReplay->peekState(GhostBikeState);
	    m_MotoGame.UpdateGhostFromReplay(&GhostBikeState);
	    /* ghost info */
	    if(m_bEnableGhostInfo) {
	      switch(m_GhostSearchStrategy) {
	      case GHOST_STRATEGY_MYBEST:
		m_Renderer.setGhostReplayDesc("Your best");
		break;
	      case GHOST_STRATEGY_THEBEST:
		m_Renderer.setGhostReplayDesc("Local best");
		break;
#if defined(SUPPORT_WEBACCESS) 
	      case GHOST_STRATEGY_BESTOFROOM:
		m_Renderer.setGhostReplayDesc(m_pWebHighscores->getRoomName() );
		break;
#endif
	      default:
		m_Renderer.setGhostReplayDesc("");
	      }
	    }
	    else
	      m_Renderer.setGhostReplayDesc("");
	  } else {
	    /* bad replay */
	  }
	}
      }
#endif
      /* Start playing right away */     
      m_InputHandler.resetScriptKeyHooks();
      if(pLevelSrc != NULL) {
	if(m_pReplay != NULL) delete m_pReplay;
	m_pReplay = NULL;
    
	if(m_bRecordReplays) {
	  m_pReplay = new Replay;
	  m_pReplay->createReplay("Latest.rpl",pLevelSrc->getID(),m_pPlayer->PlayerName,m_fReplayFrameRate,sizeof(SerializedBikeState));
	}

	try {
	  m_MotoGame.prePlayLevel(m_pGhostReplay, pLevelSrc, m_pReplay, false);
	} catch(Exception &e) {
	  Log("** Warning ** : failed to initialize level");
	  setState(GS_MENU);
	  notifyMsg(splitText(e.getMsg(), 50));
	  return;
	}

	if(!m_MotoGame.isInitOK()) {
	  Log("** Warning ** : failed to initialize level");
	  setState(GS_MENU);
	  notifyMsg(GAMETEXT_FAILEDTOINITLEVEL);
	  return;
	}
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

  }

  void GameApp::statePrestart_step() {
    prestartAnimation_step();
  }

  void GameApp::prestartAnimation_init() {
    m_fPrePlayStartTime = getRealTime();  // because the man can change ugly mode while the animation
    m_fPrePlayStartInitZoom = m_Renderer.getCurrentZoom();  // because the man can change ugly mode while the animation
    m_fPrePlayStartCameraX = m_Renderer.getCameraPositionX();
    m_fPrePlayStartCameraY = m_Renderer.getCameraPositionY();       
    
    if(m_bPrePlayAnim && m_bUglyMode == false && m_bEnableInitZoom) {

      m_MotoGame.gameMessage(m_MotoGame.getLevelSrc()->getLevelInfo()->Name, false, PRESTART_ANIMATION_LEVEL_MSG_DURATION);
      
      m_zoomX = (2.0 * ((float)getDispWidth() / (float)getDispHeight())) / (m_MotoGame.getLevelSrc()->getRightLimit() - m_MotoGame.getLevelSrc()->getLeftLimit() + 2*PRESTART_ANIMATION_MARGIN_SIZE);
      m_zoomY = 2.0 /(m_MotoGame.getLevelSrc()->getTopLimit() - m_MotoGame.getLevelSrc()->getBottomLimit()+2*PRESTART_ANIMATION_MARGIN_SIZE);
      
      if (m_zoomX > m_zoomY){
        float visibleHeight,cameraStartHeight;

        m_zoomU=m_zoomX;
        static_time = (m_MotoGame.getLevelSrc()->getTopLimit() - m_MotoGame.getLevelSrc()->getBottomLimit()) / (2.0/m_zoomU);
        
        visibleHeight = 2.0/m_zoomU;
        cameraStartHeight= visibleHeight/2.0;
        
        m_fPreCameraStartX = (m_MotoGame.getLevelSrc()->getRightLimit() + m_MotoGame.getLevelSrc()->getLeftLimit())/2;
        m_fPreCameraStartY = m_MotoGame.getLevelSrc()->getTopLimit() - cameraStartHeight + PRESTART_ANIMATION_MARGIN_SIZE;
        m_fPreCameraFinalX = (m_MotoGame.getLevelSrc()->getRightLimit() + m_MotoGame.getLevelSrc()->getLeftLimit())/2;
        m_fPreCameraFinalY = m_MotoGame.getLevelSrc()->getBottomLimit() + cameraStartHeight - PRESTART_ANIMATION_MARGIN_SIZE;

        if ( fabs(m_fPreCameraStartY - m_fPrePlayStartCameraY) > fabs(m_fPreCameraFinalY - m_fPrePlayStartCameraY)) {
          float f;
          f = m_fPreCameraFinalY;
          m_fPreCameraFinalY = m_fPreCameraStartY;
          m_fPreCameraStartY = f;
        }
        
      }else {
        float visibleWidth,cameraStartLeft;
        
        m_zoomU=m_zoomY;
        static_time = (m_MotoGame.getLevelSrc()->getRightLimit() - m_MotoGame.getLevelSrc()->getLeftLimit()) / ((2.0 * ((float)getDispWidth() / (float)getDispHeight()))/m_zoomU);
      
        visibleWidth = (2.0 * ((float)getDispWidth() / (float)getDispHeight()))/m_zoomU;
        cameraStartLeft = visibleWidth/2.0;
        
        m_fPreCameraStartX = m_MotoGame.getLevelSrc()->getRightLimit() - cameraStartLeft + PRESTART_ANIMATION_MARGIN_SIZE;
        m_fPreCameraStartY = (m_MotoGame.getLevelSrc()->getBottomLimit() + m_MotoGame.getLevelSrc()->getTopLimit())/2;
        m_fPreCameraFinalX = m_MotoGame.getLevelSrc()->getLeftLimit() + cameraStartLeft - PRESTART_ANIMATION_MARGIN_SIZE;
        m_fPreCameraFinalY = (m_MotoGame.getLevelSrc()->getBottomLimit() + m_MotoGame.getLevelSrc()->getTopLimit())/2;
   
        if ( fabs(m_fPreCameraStartX - m_fPrePlayStartCameraX) > fabs(m_fPreCameraFinalX - m_fPrePlayStartCameraX)) {
          float f;
          f = m_fPreCameraFinalX;
          m_fPreCameraFinalX = m_fPreCameraStartX;
          m_fPreCameraStartX = f;
        }
      }
    }
  }
   
  void GameApp::prestartAnimation_step() {
    if(m_bPrePlayAnim && m_bUglyMode == false && m_bEnableInitZoom) {
      
      if(getRealTime() > m_fPrePlayStartTime + static_time + PRESTART_ANIMATION_TIME) {
        setPrePlayAnim(false); // disable anim
        m_Renderer.setZoom(m_fPrePlayStartInitZoom);
        m_Renderer.setCameraPosition(m_fPrePlayStartCameraX, m_fPrePlayStartCameraY);
        setState(GS_PLAYING);
      } else if(getRealTime() > m_fPrePlayStartTime + static_time){
        float zx, zy, zz;

        zz = (logf (PRESTART_ANIMATION_CURVE * ((PRESTART_ANIMATION_TIME + static_time - getRealTime() + m_fPrePlayStartTime) / (PRESTART_ANIMATION_TIME)) + 1.0)) / logf(PRESTART_ANIMATION_CURVE + 1.0) * (m_fPrePlayStartInitZoom - m_zoomU);
        
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
    } else {
      m_Renderer.setZoom(m_fPrePlayStartInitZoom); // because the man can change ugly mode while the animation
      m_Renderer.setCameraPosition(m_fPrePlayStartCameraX, m_fPrePlayStartCameraY);
      setState(GS_PLAYING);
    }
    m_MotoGame.updateGameMessages();
  }

  int GameApp::_Pack_getNumberOfLevelsFinished(LevelPack *p_levelPack) {
    LevelSrc *v_level;
    int n;

    n = 0;
    for(int i=0; i<p_levelPack->Levels.size(); i++) {
      v_level = p_levelPack->Levels[i];
      if(m_Profiles.isLevelCompleted(m_pPlayer->PlayerName, v_level->getID())) {
	n++;
      }
    }
    return n;
  }

  void GameApp::_UpdateLevelsLists() {
    _CreateLevelPackLevelList();
    _UpdateLevelPackList();
    _UpdateLevelLists();
  }
}
