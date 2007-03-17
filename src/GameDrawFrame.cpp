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
    
    /* Prepare frame rendering / game update */
    _PrepareFrame();    

    /* Check some GUI controls */
    _PreUpdateGUI();     
     
    /* Update FPS stuff */
    _UpdateFPSCounter();
            
    /* What state? */
    switch(m_State) {
      case GS_MENU:
        /* Menu specifics */
        _PreUpdateMenu();
        /* Note the lack of break; the following are done for GS_MENU too */

      case GS_LEVEL_INFO_VIEWER:
      case GS_LEVELPACK_VIEWER:
#if defined(SUPPORT_WEBACCESS)
      case GS_EDIT_WEBCONFIG:
#endif
      case GS_EDIT_PROFILES:
        /* Following is done for all the above states */
        _DrawMainGUI();
        
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
        
          /* When did the frame start? */
          double fStartFrameTime = getTime();                    

          if(m_State == GS_PREPLAYING) {
            /* If "preplaying" / "initial-zoom" is enabled, this is where it's done */
            statePrestart_step();

	    if(!m_bTimeDemo) {
	      /* limit framerate while PREPLAY (100 fps)*/
	      double timeElapsed = getTime() - fStartFrameTime;
	      if(timeElapsed < 0.01)
		setFrameDelay(10 - (int)(timeElapsed*1000.0));
	    }
          } else if(m_State == GS_PLAYING ||
		    ((m_State == GS_DEADMENU || m_State == GS_DEADJUST) && m_bEnableDeathAnim)
		    ) {
            /* When actually playing or when dead and the bike is falling apart, 
               a physics update is required */
	    if(isLockedMotoGame()) {
	      nPhysSteps = 0;
	    } else {
	      nPhysSteps = _UpdateGamePlaying();            
	    }
          }
          else if(m_State == GS_REPLAYING) {
            /* When playing back a replay, no physics update is requried - instead
               the game state is streamed out of a binary .rpl file */
	    nPhysSteps = _UpdateGamePlaying();
          }
  
	  if(m_State == GS_PLAYING) {
	    autoZoom();
	  }

          /* Render */
          if(!getDrawLib()->isNoGraphics()) {
            m_Renderer.render(bIsPaused);
          }
#if SIMULATE_SLOW_RENDERING
          SDL_Delay(SIMULATE_SLOW_RENDERING);
#endif
  
          /* When actually playing, check if something happened (like dying or finishing) */
          if(m_State == GS_PLAYING) {        
            _PostUpdatePlaying();
          }
        
          /* When did frame rendering end? */
          double fEndFrameTime = getTime();
          
          /* Calculate how large a delay should be inserted after the frame, to keep the 
             desired frame rate */
          int nADelay = 0;    
          
	  if ((m_State == GS_FINISHED) || (m_State == GS_DEADMENU || m_State == GS_DEADJUST) || (m_State == GS_PAUSE)) {
            setFrameDelay(10);
          } else {
            /* become idle only if we hadn't to skip any frame, recently, and more globaly (80% of fps) */
            if((nPhysSteps <= 1) && (m_fFPS_Rate > (0.8f / PHYS_STEP_SIZE)))
              nADelay = ((m_fLastPhysTime + PHYS_STEP_SIZE) - fEndFrameTime) * 1000.0f;

	    if(m_autoZoom){
	      /* limit framerate while zooming (100 fps)*/
	      double timeElapsed = getTime() - fStartFrameTime;
	      if(timeElapsed < 0.01)
		nADelay = 10 - (int)(timeElapsed*1000.0);
	    }
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


          if(m_State == GS_PAUSE) {
            /* Okay, nifty thing. Paused! */
            _PostUpdatePause();
          }        
          else if(m_State == GS_DEADJUST) {
            /* Hmm, you're dead and you know it. */
            _PostUpdateJustDead();
          }
          else if(m_State == GS_DEADMENU) {
            /* Hmm, you're dead and you know it. */
            _PostUpdateMenuDead();
          }
          else if(m_State == GS_FINISHED) {
            /* Hmm, you've won and you know it. */
            _PostUpdateFinished();
          }        

          /* Level and player name to draw? */
          if(!m_bCreditsModeActive &&
            (m_State == GS_DEADMENU || m_State == GS_DEADJUST || m_State == GS_PAUSE || m_State == GS_FINISHED || m_State == GS_REPLAYING) &&
            m_MotoGame.getLevelSrc() != NULL) {
            UIFont *v_font = m_Renderer.getMediumFont();
            std::string v_infos;
	  }
         
          /* Context menu? */
          if(m_State == GS_PREPLAYING || m_State == GS_PLAYING || m_State == GS_REPLAYING || !m_bEnableContextHelp)
            m_Renderer.getGUI()->enableContextMenuDrawing(false);
          else
            m_Renderer.getGUI()->enableContextMenuDrawing(true);
          
          /* Draw GUI */
          m_Renderer.getGUI()->paint();        
        
          /* Credits? */
          if(m_State == GS_REPLAYING && m_bCreditsModeActive && m_pCredits!=NULL) {
            m_pCredits->render(m_MotoGame.getTime());
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
      Sound::update();

      for(unsigned int i=0; i<m_MotoGame.Players().size(); i++) {
	m_InputHandler.updateInput(m_MotoGame.Players()[i]->getControler(), i);
      }
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
	  if(m_MotoGame.Players().size() == 1) {
	    m_GameStats.abortedLevel(m_pPlayer->PlayerName,m_MotoGame.getLevelSrc()->Id(),m_MotoGame.getLevelSrc()->Name(),m_MotoGame.getTime()); 
	  }
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

    if(m_State == GS_REPLAYING) {
      if(m_replayBiker->isDead() || m_replayBiker->isFinished()) {

	if(m_bBenchmark) {
	  double fBenchmarkTime = getRealTime() - m_fStartTime;
	  printf("\n");
	  printf(" * %d frames rendered in %.0f seconds\n",m_nFrame,fBenchmarkTime);
	  printf(" * Average framerate: %.1f fps\n",((double)m_nFrame) / fBenchmarkTime);
	  quit();
	}

	if(m_replayBiker->isDead()) {
	  return 0;
	}
	if(m_replayBiker->isFinished()) {
	  m_MotoGame.setTime(m_replayBiker->finishTime());
	  return 0;
	}
      }
    }

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
      if(m_State == GS_PLAYING || m_State == GS_DEADJUST || m_State == GS_REPLAYING) {
        m_MotoGame.updateLevel(PHYS_STEP_SIZE, m_pJustPlayReplay);
      }
      m_fLastPhysTime += PHYS_STEP_SIZE;
      nPhysSteps++;

#if SIMULATE_SLOW_PHYSICS
      SDL_Delay(SIMULATE_SLOW_PHYSICS);
#endif

      /* don't do this infinitely, maximum miss 10 frames, then give up */
    } while ((m_fLastPhysTime + PHYS_STEP_SIZE <= getTime()) && (nPhysSteps < 10));
  
    m_Renderer.setSpeedMultiplier(nPhysSteps);
  
    if(m_bTimeDemo == false) {
      /* Never pass this point while being ahead of time, busy wait until it's time */
      if(nPhysSteps <= 1) {  
        while (m_fLastPhysTime > getTime());
      }
    }
      
    return nPhysSteps;
  }  

  int GameApp::_UpdateGameReplaying(void) {
    int nPhysSteps = 1;
    m_nFrame++;
    static float fGTime = 0, fRTime = 0;
    m_Renderer.setSpeedMultiplier(nPhysSteps);    

    return nPhysSteps;
  }

  void GameApp::_PostUpdatePlaying(void) {
    bool v_all_dead = true;
    bool v_one_finished = false;
    float v_finish_time = 0.0;

    for(unsigned int i=0; i<m_MotoGame.Players().size(); i++) {
      if(m_MotoGame.Players()[i]->isDead() == false) {
	v_all_dead = false;
      }
      if(m_MotoGame.Players()[i]->isFinished()) {
	v_one_finished = true;
	v_finish_time  = m_MotoGame.Players()[i]->finishTime();
      }
    }

    /* News? */
    if(v_one_finished) {
      /* You're done maaaan! :D */
      if(m_MotoGame.Players().size() == 1) {
	std::string TimeStamp = getTimeStamp();
	m_Profiles.addFinishTime(m_pPlayer->PlayerName,"",
				 m_MotoGame.getLevelSrc()->Id(),v_finish_time,TimeStamp); 
	_MakeBestTimesWindow(m_pBestTimes,m_pPlayer->PlayerName,m_MotoGame.getLevelSrc()->Id(),
			     v_finish_time,TimeStamp);
      }

      //_UpdateLevelsLists();     
      setState(GS_FINISHED);
    } else if(v_all_dead) {
      /* You're dead maan! */
      setState(GS_DEADJUST);
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

