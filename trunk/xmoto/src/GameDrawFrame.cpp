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
 *  Game application. (drawFrame()-related stuff)
 */
 
/* rneckelmann 2006-09-30: moved a lot of stuff from Game.cpp into here to 
                           make it a tad smaller */ 
#include "Game.h"
#include "VFileIO.h"
#include "Sound.h"
#include "PhysSettings.h"
#include "Input.h"

#if defined(SUPPORT_WEBACCESS)
  #include <curl/curl.h>
#endif

#include "helpers/HighPrecisionTimer.h"

/* Set the following #defines to simulate slow systems */
#define SIMULATE_SLOW_RENDERING     0 /* extra ms to add to rendering */
#define SIMULATE_SLOW_PHYSICS       0 /* extra ms to add to physics calcs */

namespace vapp {

  /*===========================================================================
  Draw frame
  ===========================================================================*/
  void GameApp::drawFrame(void) {
    /* This function is called by the framework as fast as possible */
    bool bIsPaused = false;
    bool bDrawFPS = false;
    
    HighPrecisionTimer::reset();

    /* Prepare frame rendering / game update */
    _PrepareFrame();    
    HighPrecisionTimer::checkTime("_PrepareFrame");

    /* Check some GUI controls */
    _PreUpdateGUI();     
    HighPrecisionTimer::checkTime("_PreUpdateGUI");
     
    /* Update FPS stuff */
    _UpdateFPSCounter();
    HighPrecisionTimer::checkTime("_UpdateFPSCounter");
            
    /* What state? */
    switch(m_State) {
      case GS_MENU:
        /* Menu specifics */
        _PreUpdateMenu();
        HighPrecisionTimer::checkTime("_PreUpdateMenu");
        /* Note the lack of break; the following are done for GS_MENU too */

      case GS_LEVEL_INFO_VIEWER:
      case GS_LEVELPACK_VIEWER:
#if defined(SUPPORT_WEBACCESS)
      case GS_EDIT_WEBCONFIG:
#endif
      case GS_EDIT_PROFILES:
        /* Following is done for all the above states */
        _DrawMainGUI();
        HighPrecisionTimer::checkTime("_DrawMainGUI");
        
        /* Show frame rate? */
        if(m_bShowFrameRate) bDrawFPS = true;

        /* Delay a bit so we don't eat all CPU */
        setFrameDelay(10);
        break;
        
      case GS_PAUSE:
        bIsPaused = true;
              
      case GS_DEADMENU:
      case GS_DEADJUST:
      case GS_FINISHED:
      case GS_REPLAYING:
      case GS_PREPLAYING:
      case GS_PLAYING: {
        /* These states all requires that the actual game graphics are rendered (i.e. inside 
           the game, not the main menu) */
        try {
          int nPhysSteps = 0;
          bool bValidGameState = true;
        
          /* When did the frame start? */
          double fStartFrameTime = getTime();                    

          if(m_State == GS_PREPLAYING) {
            /* If "preplaying" / "initial-zoom" is enabled, this is where it's done */
            statePrestart_step();
            HighPrecisionTimer::checkTime("statePrestart_step");
          } 
          else if(m_State == GS_PLAYING || ((m_State == GS_DEADMENU || m_State == GS_DEADJUST) && m_bEnableDeathAnim)) {
            /* When actually playing or when dead and the bike is falling apart, 
               a physics update is required */
						if(isLockedMotoGame()) {
							nPhysSteps = 0;
						} else {
							nPhysSteps = _UpdateGamePlaying();            
						}
            HighPrecisionTimer::checkTime("_UpdateGamePlaying");
          }
          else if(m_State == GS_REPLAYING) {
            /* When playing back a replay, no physics update is requried - instead
               the game state is streamed out of a binary .rpl file */
							nPhysSteps = _UpdateGameReplaying();
            HighPrecisionTimer::checkTime("_UpdateGameReplaying");
            bValidGameState = nPhysSteps > 0;
            //printf("%d ",nPhysSteps);
          }
  
					if(m_State == GS_PLAYING) {
						autoZoom();
					}

          /* Render */
          if(!getDrawLib()->isNoGraphics() && bValidGameState) {
            m_Renderer.render(bIsPaused);
            HighPrecisionTimer::checkTime("m_Renderer.render");
      
            if(m_bShowMiniMap && !m_bCreditsModeActive) {
              if(m_MotoGame.getBikeState()->Dir == DD_LEFT &&
                 (m_bShowEngineCounter == false || m_State == GS_REPLAYING)) {
                m_Renderer.renderMiniMap(getDrawLib()->getDispWidth()-150,getDrawLib()->getDispHeight()-100,150,100);
                HighPrecisionTimer::checkTime("m_Renderer.renderMiniMap");
              } 
              else {
                m_Renderer.renderMiniMap(0,getDrawLib()->getDispHeight()-100,150,100);
                HighPrecisionTimer::checkTime("m_Renderer.renderMiniMap");
              }
            }             
      
            if(m_bShowEngineCounter && m_bUglyMode == false && m_State != GS_REPLAYING) {
              m_Renderer.renderEngineCounter(getDrawLib()->getDispWidth()-128,getDrawLib()->getDispHeight()-128,128,128,
                                             m_MotoGame.getBikeEngineSpeed());
              HighPrecisionTimer::checkTime("m_Renderer.renderEngineCounter");
            } 
          }
#if SIMULATE_SLOW_RENDERING
          SDL_Delay(SIMULATE_SLOW_RENDERING);
          HighPrecisionTimer::checkTime("(dummy)");
#endif
  
          /* When actually playing, check if something happened (like dying or finishing) */
          if(m_State == GS_PLAYING) {        
            _PostUpdatePlaying();
            HighPrecisionTimer::checkTime("_PostUpdatePlaying");
          }
        
          /* When did frame rendering end? */
          double fEndFrameTime = getTime();
          
          /* Calculate how large a delay should be inserted after the frame, to keep the 
             desired frame rate */
          int nADelay = 0;    
          
          if(m_State == GS_REPLAYING) {
            /* When replaying... */
            //printf("{ %f } ",m_fCurrentReplayFrameRate);
            if(nPhysSteps <= 1)                        
              nADelay = ((1.0f/m_fCurrentReplayFrameRate - (fEndFrameTime-fStartFrameTime)) * 1000.0f) * 0.5;
            //printf(" { %f }  %d\n",fEndFrameTime-fStartFrameTime,nADelay);            
          }
          else if ((m_State == GS_FINISHED) || (m_State == GS_DEADMENU || m_State == GS_DEADJUST) || (m_State == GS_PAUSE)) {
            setFrameDelay(10);
          }
          else {
            /* become idle only if we hadn't to skip any frame, recently, and more globaly (80% of fps) */
            if((nPhysSteps <= 1) && (m_fFPS_Rate > (0.8f / PHYS_STEP_SIZE)))
              nADelay = ((m_fLastPhysTime + PHYS_STEP_SIZE) - fEndFrameTime) * 1000.0f;
          }
                  
          if(nADelay > 0) {
            if(!m_bTimeDemo) {
              setFrameDelay(nADelay);
            }
          }        

          /* Show fps (debug modish) */
          if(m_bDebugMode) {
            static char cBuf[256] = ""; 
            static int nFrameCnt = 0;
            if(m_fFrameTime - m_fLastPerfStateTime > 0.5f) {
              float f = m_fFrameTime - m_fLastPerfStateTime;
              sprintf(cBuf,"%.1f",((float)nFrameCnt)/f);
              nFrameCnt = 0;
              m_fLastPerfStateTime = m_fFrameTime;
            }
            getDrawLib()->drawText(Vector2f(0,100),cBuf,MAKE_COLOR(0,0,0,255),-1);        
            nFrameCnt++;
          }

          HighPrecisionTimer::checkTime("(misc)");

          if(m_State == GS_PAUSE) {
            /* Okay, nifty thing. Paused! */
            _PostUpdatePause();
            HighPrecisionTimer::checkTime("_PostUpdatePause");
          }        
          else if(m_State == GS_DEADJUST) {
            /* Hmm, you're dead and you know it. */
            _PostUpdateJustDead();
            HighPrecisionTimer::checkTime("_PostUpdateJustDead");
          }
          else if(m_State == GS_DEADMENU) {
            /* Hmm, you're dead and you know it. */
            _PostUpdateMenuDead();
            HighPrecisionTimer::checkTime("_PostUpdateMenuDead");
          }
          else if(m_State == GS_FINISHED) {
            /* Hmm, you've won and you know it. */
            _PostUpdateFinished();
            HighPrecisionTimer::checkTime("_PostUpdateFinished");
          }        

          /* Level and player name to draw? */
          if(!m_bCreditsModeActive &&
            (m_State == GS_DEADMENU || m_State == GS_DEADJUST || m_State == GS_PAUSE || m_State == GS_FINISHED || m_State == GS_REPLAYING) &&
            m_MotoGame.getLevelSrc() != NULL) {
            UIFont *v_font = m_Renderer.getMediumFont();
            std::string v_infos;
            
            v_infos = m_MotoGame.getLevelSrc()->Name();

            if(m_State == GS_REPLAYING && m_pReplay != NULL) {
              v_infos += " (" + std::string(GAMETEXT_BY) + " " + m_pReplay->getPlayerName() + ")";
            }

            if(v_font != NULL) {
              UITextDraw::printRaw(v_font,0,getDrawLib()->getDispHeight()-4,
                v_infos,
                MAKE_COLOR(255,255,255,255));
            }
            HighPrecisionTimer::checkTime("(level and player name)");
          }
         
          /* Context menu? */
          if(m_State == GS_PREPLAYING || m_State == GS_PLAYING || m_State == GS_REPLAYING || !m_bEnableContextHelp)
            m_Renderer.getGUI()->enableContextMenuDrawing(false);
          else
            m_Renderer.getGUI()->enableContextMenuDrawing(true);
          
          /* Draw GUI */
          m_Renderer.getGUI()->paint();        
          HighPrecisionTimer::checkTime("m_Renderer.getGUI()->paint");
        
          /* Credits? */
          if(m_State == GS_REPLAYING && m_bCreditsModeActive && m_pCredits!=NULL) {
            m_pCredits->render(m_MotoGame.getTime());
            HighPrecisionTimer::checkTime("m_pCredits->render");
          }

          /* Show frame rate */
          if(m_bShowFrameRate) bDrawFPS = true;

          break;
        } 
        catch(Exception &e) {
    setState(m_StateAfterPlaying);
          notifyMsg(splitText(e.getMsg(), 50));
        }
      }
    }
    
    /* Draw a little FPS counter */
    if(bDrawFPS) {
      char cTemp[256];        
      sprintf(cTemp,"%f",m_fFPS_Rate);
      getDrawLib()->drawText(Vector2f(130,0),cTemp);
    }    
    
    /* Profiling? */
#if defined(PROFILE_MAIN_LOOP)
    int nCurY = 60;
    float fTotal = 0;
    for(int i=0;i<HighPrecisionTimer::numTimeChecks();i++) {
      HighPrecisionTimer::TimeCheck *pTc = HighPrecisionTimer::getTimeCheck(i);
      if(pTc != NULL) {
        fTotal += pTc->fTime;
      }
    }
    for(int i=0;i<HighPrecisionTimer::numTimeChecks();i++) {
      char cTemp[256];
      HighPrecisionTimer::TimeCheck *pTc = HighPrecisionTimer::getTimeCheck(i);
      if(pTc != NULL) {
        sprintf(cTemp,"%-20s : %.0f",pTc->cWhere,pTc->fTime);
        drawText(Vector2f(30,nCurY),cTemp);
        sprintf(cTemp,"%.0f%%",(pTc->fTime * 100) / fTotal);
        drawText(Vector2f(350,nCurY),cTemp);
        nCurY += 16;
      }
    }
#endif   
    
    /* Draw mouse cursor */
    if(m_bShowCursor)
      _DrawMouseCursor();
  }

  /*===========================================================================
  Main loop utility functions
  ===========================================================================*/
  void GameApp::_DrawMouseCursor(void) {
    if(!getDrawLib()->isNoGraphics() && m_pCursor != NULL && m_bUglyMode == false) {
      int nMX,nMY;
      getMousePos(&nMX,&nMY);      
      getDrawLib()->drawImage(Vector2f(nMX-2,nMY-2),Vector2f(nMX+30,nMY+30),m_pCursor);
    }
  }
  
  void GameApp::_PrepareFrame(void) {
    m_MotoGame.setDeathAnim(m_bEnableDeathAnim); /* hack hack hack */
                
    /* Per default, don't wait between frames */
    setFrameDelay(0);
    
    /* Update sound system and input */
    if(!getDrawLib()->isNoGraphics()) {        
      m_EngineSound.update(getRealTime());
      m_EngineSound.setRPM(0); /* per default don't have engine sound */
      Sound::update();
      
      m_InputHandler.updateInput(m_MotoGame.getBikeController());
    }    
    
    /* Whether or not we should have a mouse cursor? */
    switch(m_State) {
      case GS_MENU:
      case GS_EDIT_PROFILES:
#if defined(SUPPORT_WEBACCESS)
      case GS_EDIT_WEBCONFIG:
#endif
      case GS_LEVEL_INFO_VIEWER:
      case GS_PAUSE:
      case GS_DEADMENU:
      case GS_FINISHED:
      case GS_LEVELPACK_VIEWER:
        m_bShowCursor = true;
        //SDL_ShowCursor(SDL_ENABLE);
        break;

      case GS_PREPLAYING:
      case GS_PLAYING:
      case GS_REPLAYING:
        m_bShowCursor = false;
        //SDL_ShowCursor(SDL_DISABLE);      

        /* Music playing? Not anymore that is! */
        if(m_pMenuMusic != NULL && Mix_PlayingMusic()) {
          Mix_FadeOutMusic(500);
        }
        
        break;
      case GS_DEADJUST:
      break;
    }
  }  

  void GameApp::_PreUpdateGUI(void) {
    /* Quit msg box open? */
    if(m_pQuitMsgBox != NULL) {
      UIMsgBoxButton Button = m_pQuitMsgBox->getClicked();
      if(Button == UI_MSGBOX_YES) {
        if(m_State == GS_PAUSE) {
          m_GameStats.abortedLevel(m_pPlayer->PlayerName,m_MotoGame.getLevelSrc()->Id(),m_MotoGame.getLevelSrc()->Name(),m_MotoGame.getTime()); 
        }
      
        quit();      
        delete m_pQuitMsgBox;
        m_pQuitMsgBox = NULL;
        return;
      }
      else if(Button == UI_MSGBOX_NO) {
        delete m_pQuitMsgBox;
        m_pQuitMsgBox = NULL;
      }  
    }
    /* What about the notify box then? */
    else if(m_pNotifyMsgBox != NULL) {
      if(m_pNotifyMsgBox->getClicked() == UI_MSGBOX_OK) {
        delete m_pNotifyMsgBox;
        m_pNotifyMsgBox = NULL;
      }
    }
#if defined(SUPPORT_WEBACCESS)
    /* And the download-levels box? */
    else if(m_pInfoMsgBox != NULL) {
      UIMsgBoxButton Button = m_pInfoMsgBox->getClicked();
      if(Button == UI_MSGBOX_YES) {
        delete m_pInfoMsgBox;
        m_pInfoMsgBox = NULL;

        /* Download levels! */
        _DownloadExtraLevels();
        
        /* current theme should be updated when there are new levels */
        _UpdateWebThemes(true);
        ThemeChoice* v_ThemeChoice;
        v_ThemeChoice = m_themeChoicer->getChoiceByName(m_Config.getString("Theme"));
        if(v_ThemeChoice != NULL) {
          _UpdateWebTheme(v_ThemeChoice, false);      
        }
      }
      else if(Button == UI_MSGBOX_NO) {
        delete m_pInfoMsgBox;
        m_pInfoMsgBox = NULL;
      }
    }
#endif       
  }
  
  void GameApp::_UpdateFPSCounter(void) {
    /* Perform a rather precise calculation of the frame rate */    
    m_fFrameTime = getRealTime();
    static int nFPS_Frames = 0;
    static double fFPS_LastTime = 0.0f;
    static double fFPS_CurrentTime = 0.0f;
    
    fFPS_CurrentTime = getRealTime();
    if(fFPS_CurrentTime - fFPS_LastTime > 1.0f && nFPS_Frames>0) {
      m_fFPS_Rate = ((float)nFPS_Frames) / (fFPS_CurrentTime - fFPS_LastTime);
      nFPS_Frames = 0;
      fFPS_LastTime = fFPS_CurrentTime;
    }
    nFPS_Frames++;
    
    m_fLastFrameTime = m_fFrameTime; /* FIXME unsused*/
  }
  
  void GameApp::_PreUpdateMenu(void) {
    /* Did the initializer come up with messages for the user? */
    if(getUserNotify() != "") {
      notifyMsg(getUserNotify());
    }                
    /* Should we show a notification box? (with important one-time info) */
    else if(m_Config.getBool("NotifyAtInit")) {
      if(m_pNotifyMsgBox == NULL) {
        notifyMsg(GAMETEXT_NOTIFYATINIT);                    
        
        /* Don't do this again, please */
        m_Config.setBool("NotifyAtInit",false); 
      }
    }        

    if(m_bEnableMenuMusic && Sound::isEnabled()) {
      /* No music playing? If so, playback time! */
      if(m_pMenuMusic == NULL) {
        /* No music available, try loading */
        std::string MenuMusicPath = FS::getDataDir() + std::string("/xmoto.ogg");
        const char *pc = MenuMusicPath.c_str();
        #if defined(WIN32) /* this works around a bug in SDL_mixer 1.2.7 on Windows */
	//SDL_RWops *rwfp;              
	//rwfp = SDL_RWFromFile(pc, "rb");                          
	//m_pMenuMusic = Mix_LoadMUS_RW(rwfp);
	m_pMenuMusic = NULL;
        #else
          m_pMenuMusic = Mix_LoadMUS(pc);
        #endif
        /* (Don't even complain the slightest if music isn't found...) */          
      }
      
      if(m_pMenuMusic != NULL) {
        if(!Mix_PlayingMusic()) {
          /* Not playing, start it */
          if(Mix_PlayMusic(m_pMenuMusic,-1) < 0) {
            Log("** Warning ** : Mix_PlayMusic() failed, disabling music");
            m_bEnableMenuMusic = false;                
          }
        }
      }
    }
    else {
      /* Hmm, no music please. If it's playing, stop it */
      if(m_pMenuMusic != NULL) {
        if(Mix_PlayingMusic()) {
          Mix_FadeOutMusic(500);              
        }
      }
    }
  }

  void GameApp::_DrawMainGUI(void) {
    /* Draw menu background */
    _DrawMenuBackground();
            
    /* Update mouse stuff */
    _DispatchMouseHover();
    
    /* Blah... */
    if(m_State == GS_MENU)
      _HandleMainMenu();
    else if(m_State == GS_EDIT_PROFILES)
      _HandleProfileEditor();
#if defined(SUPPORT_WEBACCESS) 
    else if(m_State == GS_EDIT_WEBCONFIG)
      _HandleWebConfEditor();
#endif
    else if(m_State == GS_LEVEL_INFO_VIEWER)
      _HandleLevelInfoViewer();
    else if(m_State == GS_LEVELPACK_VIEWER)
      _HandleLevelPackViewer();
              
    /* Draw GUI */
    if(m_bEnableContextHelp)
      m_Renderer.getGUI()->enableContextMenuDrawing(true);
    else
      m_Renderer.getGUI()->enableContextMenuDrawing(false);
      
    m_Renderer.getGUI()->paint();                
  }
  
  int GameApp::_UpdateGamePlaying(void) {
    /* Increase frame counter */
    m_nFrame++;

    /* Following time code is made by Eric Piel, but I took the liberty to change the minimum
        frame-miss number from 50 to 10, because it wasn't working well. */

    /* reinitialise if we can't catch up */
    if (m_fLastPhysTime - getTime() < -0.1f)
      m_fLastPhysTime = getTime() - PHYS_STEP_SIZE;

    /* Update game until we've catched up with the real time */
    int nPhysSteps = 0;
    do {
      if(m_State == GS_PLAYING) {
        m_MotoGame.updateLevel( PHYS_STEP_SIZE,NULL,m_pReplay );
      } 
      else {
        m_MotoGame.updateLevel( PHYS_STEP_SIZE,NULL, NULL );
      }
      m_fLastPhysTime += PHYS_STEP_SIZE;
      nPhysSteps++;

#if SIMULATE_SLOW_PHYSICS
      SDL_Delay(SIMULATE_SLOW_PHYSICS);
#endif

      if(m_Config.getBool("LimitFramerate")) {
        SDL_Delay(1);
      }     

      /* don't do this infinitely, maximum miss 10 frames, then give up */
    } while ((m_fLastPhysTime + PHYS_STEP_SIZE <= getTime()) && (nPhysSteps < 10));
  
    m_Renderer.setSpeedMultiplier(nPhysSteps);
  
    if(!m_bTimeDemo) {
      /* Never pass this point while being ahead of time, busy wait until it's time */
      if(nPhysSteps <= 1) {  
        while (m_fLastPhysTime > getTime());
      }
    }

    if(m_bEnableEngineSound) {
      /* Update engine RPM */
      m_EngineSound.setRPM( m_MotoGame.getBikeEngineRPM() ); 
    }
  
    /* Squeeking? */
    if(m_MotoGame.isSqueeking()) {
      if(getTime() - m_fLastSqueekTime > 0.1f) {
        /* (this is crappy, not enabled) */
        //Sound::playSampleByName("Sounds/Squeek.ogg",m_MotoGame.howMuchSqueek());
        m_fLastSqueekTime = getTime();
      }
    }
  
#if defined(ALLOW_GHOST)
    /* Read replay state */
    if(m_pGhostReplay != NULL) {
      static SerializedBikeState GhostBikeState;
      static SerializedBikeState previousGhostBikeState;
      
      m_pGhostReplay->peekState(previousGhostBikeState);
      if(previousGhostBikeState.fGameTime < m_MotoGame.getTime() && m_pGhostReplay->endOfFile() == false) {
        do {
          m_pGhostReplay->loadState(GhostBikeState);
        } while(GhostBikeState.fGameTime < m_MotoGame.getTime() && m_pGhostReplay->endOfFile() == false);

        if(m_nGhostFrame%2 || m_nGhostFrame==1) {
          /* NON-INTERPOLATED FRAME */
          m_MotoGame.UpdateGhostFromReplay(&GhostBikeState);
        } 
        else {
          /* INTERPOLATED FRAME */
          SerializedBikeState ibs;
          m_MotoGame.interpolateGameState(&previousGhostBikeState,&GhostBikeState,&ibs,0.5f);
          m_MotoGame.UpdateGhostFromReplay(&ibs);
        }
        m_nGhostFrame++;
      }
    }
#endif    

    if(m_State == GS_PLAYING) {
      /* We'd like to serialize the game state 25 times per second for the replay */
      if(getRealTime() - m_fLastStateSerializationTime >= 1.0f/m_fReplayFrameRate) {
        m_fLastStateSerializationTime = getRealTime();
        
        /* Get it */
        SerializedBikeState BikeState;
        m_MotoGame.getSerializedBikeState(&BikeState);
        if(m_pReplay != NULL)
          m_pReplay->storeState(BikeState);              
      }
    }
    
    return nPhysSteps;
  }  

  int GameApp::_UpdateGameReplaying(void) {
    int nPhysSteps = 1;
    m_nFrame++;
    static float fGTime = 0, fRTime = 0;

    /* Following (automatic replay speed adjustment) is only enabled 
       when showing credits.  */     
    if(m_pReplay != NULL && m_bCreditsModeActive) {
      /* Compare game time with real time */
      float fGCurTime = m_MotoGame.getTime();
      float fRCurTime = getRealTime();
      float fGDiff = fGCurTime - fGTime;
      float fRDiff = fRCurTime - fRTime;
      fGTime = fGCurTime;
      fRTime = fRCurTime;
      
      if(fabs(fRDiff - fGDiff) > 0.01f) {            
        if(fGDiff + 0.01f < fRDiff && !m_pReplay->isPaused() ||
          fGDiff - 0.01f > fRDiff && !m_pReplay->isPaused()) {
          float fControl = (fRDiff - fGDiff)*0.6;
          if(fControl < -0.1f) fControl = -0.1f;
          if(fControl > 0.1f) fControl = 0.1f;
          m_pReplay->setSpeed(m_pReplay->getSpeed() + fControl);
        }
        
        if(m_pReplay->getSpeed() < 0.2f) m_pReplay->setSpeed(0.2f);
        if(m_pReplay->getSpeed() > 8.0f) m_pReplay->setSpeed(8.0);
      }
    }
    
    /* Read replay state */
    static SerializedBikeState BikeState;          
    if(m_pReplay != NULL) {       
      /* Even frame number: Read the next state */
      if(m_nFrame%2 || m_nFrame==1) {       
        /* REAL NON-INTERPOLATED FRAME */
        if(m_pReplay->endOfFile() == false) {
          m_pReplay->loadState(BikeState);
        
          /* Update game */
          m_MotoGame.updateLevel( 1/m_pReplay->getFrameRate(),&BikeState,m_pReplay ); 
          //m_MotoGame.updateLevel( PHYS_STEP_SIZE,&BikeState,m_pReplay ); 
        
          if(m_bEnableEngineSound) {
            /* Update engine RPM */
            m_EngineSound.setRPM( m_MotoGame.getBikeEngineRPM() ); 
          }
        }
        else {
          if(m_pReplay->didFinish()) {
            /* Make sure that it's the same finish time */
            m_MotoGame.setTime(m_pReplay->getFinishTime());
          }
        }
      }
      else {                          
        /* INTERPOLATED FRAME */
        SerializedBikeState NextBikeState,ibs;
        
        if(m_pReplay->endOfFile() == false) {
          m_pReplay->peekState(NextBikeState);
          /* Nice. Interpolate the states! */
          m_MotoGame.interpolateGameState(&BikeState,&NextBikeState,&ibs,0.5f);

          /* Update game */
          //m_MotoGame.updateLevel( PHYS_STEP_SIZE,&ibs,m_pReplay );                 
          //printf("[%f]\n",m_pReplay->getFrameRate());
          m_MotoGame.updateLevel( 1/m_pReplay->getFrameRate(),&ibs,m_pReplay );                 
        }
      }
    
      /* Benchmarking and finished? If so, print the report and quit */
      if(m_pReplay->endOfFile() && m_bBenchmark) {
        double fBenchmarkTime = getRealTime() - m_fStartTime;
        printf("\n");
        printf(" * %d frames rendered in %.0f seconds\n",m_nFrame,fBenchmarkTime);
        printf(" * Average framerate: %.1f fps\n",((double)m_nFrame) / fBenchmarkTime);
        quit();
      }
    }
    else return 0; /* something went wrong */
  
    //if(m_pReplay->getSpeed() < 0) {
    //m_Renderer.clearAllParticles();
    //m_Renderer.skipBackTime(100000);            
    //}
  
    m_Renderer.setSpeedMultiplier(nPhysSteps);    

    return nPhysSteps;
  }

  void GameApp::_PostUpdatePlaying(void) {
    /* News? */
    if(m_MotoGame.isDead()) {
      /* You're dead maan! */
      setState(GS_DEADJUST);
    }
    else if(m_MotoGame.isFinished()) {
      /* You're done maaaan! :D */
      std::string TimeStamp = getTimeStamp();
      m_Profiles.addFinishTime(m_pPlayer->PlayerName,"",
                              m_MotoGame.getLevelSrc()->Id(),m_MotoGame.getFinishTime(),TimeStamp); 
      _MakeBestTimesWindow(m_pBestTimes,m_pPlayer->PlayerName,m_MotoGame.getLevelSrc()->Id(),
                          m_MotoGame.getFinishTime(),TimeStamp);

      _UpdateLevelsLists();     
      setState(GS_FINISHED);
    }
  }

  void GameApp::_PostUpdatePause(void) {
    if(!m_bUglyMode) {
      if(m_nPauseShade < 150) m_nPauseShade+=8;
      getDrawLib()->drawBox(Vector2f(0,0),Vector2f(getDrawLib()->getDispWidth(),getDrawLib()->getDispHeight()),0,MAKE_COLOR(0,0,0,m_nPauseShade));                                        
    }

    /* Update mouse stuff */
    _DispatchMouseHover();
    
    /* Blah... */
    _HandlePauseMenu();
  }

  void GameApp::_PostUpdateJustDead(void) {
    if(!m_bUglyMode) {
      if(m_nJustDeadShade < 150) m_nJustDeadShade+=8;
      getDrawLib()->drawBox(Vector2f(0,0),Vector2f(getDrawLib()->getDispWidth(),getDrawLib()->getDispHeight()),0,MAKE_COLOR(0,0,0,m_nJustDeadShade));     
    }
  }

  void GameApp::_PostUpdateMenuDead(void) {
    if(!m_bUglyMode) {
      if(m_nJustDeadShade < 150) m_nJustDeadShade+=8;
      getDrawLib()->drawBox(Vector2f(0,0),Vector2f(getDrawLib()->getDispWidth(),getDrawLib()->getDispHeight()),0,MAKE_COLOR(0,0,0,m_nJustDeadShade));     
    }
    
    /* Update mouse stuff */
    _DispatchMouseHover();
    
    if(getRealTime() > m_fCoolDownEnd) {
      /* Blah... */
      _HandleJustDeadMenu();
    }
  }

  void GameApp::_PostUpdateFinished(void) {
    if(!m_bUglyMode) {
      if(m_nFinishShade < 150) m_nFinishShade+=8;
      getDrawLib()->drawBox(Vector2f(0,0),Vector2f(getDrawLib()->getDispWidth(),getDrawLib()->getDispHeight()),0,MAKE_COLOR(0,0,0,m_nFinishShade));     
    }

    /* Update mouse stuff */
    _DispatchMouseHover();
    
    /* Blah... */
    _HandleFinishMenu();
  }
  
}

