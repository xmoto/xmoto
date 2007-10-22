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
 *  Game application. (drawFrame()-related stuff)
 */
 
/* rneckelmann 2006-09-30: moved a lot of stuff from Game.cpp into here to 
                           make it a tad smaller */ 
#include "GameText.h"
#include "Game.h"
#include "VFileIO.h"
#include "Sound.h"
#include "PhysSettings.h"
#include "Input.h"
#include "xmscene/Bike.h"
#include "xmscene/BikeGhost.h"
#include "xmscene/BikePlayer.h"
#include "helpers/Log.h"
#include "XMSession.h"
#include "drawlib/DrawLib.h"
#include "SysMessage.h"
#include "Credits.h"
#include "xmscene/Camera.h"
#include "xmscene/Entity.h"
#include "states/StateManager.h"
#include "states/StateFinished.h"

#include <curl/curl.h>

/* Set the following #defines to simulate slow systems */
#define SIMULATE_SLOW_RENDERING     0 /* extra ms to add to rendering */
#define SIMULATE_SLOW_PHYSICS       0 /* extra ms to add to physics calcs */

  /*===========================================================================
  Draw frame
  ===========================================================================*/
  void GameApp::drawFrame(void) {

    /* This function is called by the framework as fast as possible */
    bool bIsPaused = m_State == GS_PAUSE;

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

	// handle in the state manager
      case GS_LEVEL_INFO_VIEWER:
        _DrawMainGUI();
	break;

      case GS_EDIT_WEBCONFIG:
        /* Following is done for all the above states */
        _DrawMainGUI();
        break;

      case GS_LEVELPACK_VIEWER:
      case GS_EDIT_PROFILES:
        _DrawMainGUI();
	break;

      case GS_FINISHED:
      case GS_DEADMENU:

      case GS_DEADJUST:
      case GS_PREPLAYING:
     {

        /* These states all requires that the actual game graphics are rendered (i.e. inside 
           the game, not the main menu) */
        try {
          int nPhysSteps = 0;
        
          /* When did the frame start? */
          double fStartFrameTime = getXMTime();                    
	  int numberCam = m_MotoGame.getNumberCameras();
          if(m_State == GS_PREPLAYING) {
            /* If "preplaying" / "initial-zoom" is enabled, this is where it's done */
	    if(numberCam > 1){
	      m_MotoGame.setCurrentCamera(numberCam);
	    }
            statePrestart_step();

	    if(m_xmsession->timedemo() == false) {
	      /* limit framerate while PREPLAY (100 fps)*/
	      // TODO::MANU::the sleep is done in only one place now.
	      // put it there
	      /*
	      double timeElapsed = getXMTime() - fStartFrameTime;
	      if(timeElapsed < 0.01)
		setFrameDelay(10 - (int)(timeElapsed*1000.0));
	      */
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
  
	  if(m_State == GS_PLAYING) {
	    if(numberCam > 1){
	      m_MotoGame.setCurrentCamera(numberCam);
	    }
	    autoZoom();
	  }

          /* Render */
          if(!getDrawLib()->isNoGraphics()) {
	    try {
	      if((m_autoZoom || (m_bPrePlayAnim && m_xmsession->ugly() == false)) && numberCam > 1){
		m_Renderer->render(bIsPaused);
		ParticlesSource::setAllowParticleGeneration(m_Renderer->nbParticlesRendered() < NB_PARTICLES_TO_RENDER_LIMITATION);
	      }else{
		for(int i=0; i<numberCam; i++){
		  m_MotoGame.setCurrentCamera(i);
		  m_Renderer->render(bIsPaused);
		  ParticlesSource::setAllowParticleGeneration(m_Renderer->nbParticlesRendered() < NB_PARTICLES_TO_RENDER_LIMITATION);
		}
	      }
	    } catch(Exception &e) {
	      m_MotoGame.endLevel();
	      setState(m_StateAfterPlaying);
	      notifyMsg(splitText(e.getMsg(), 50));
	    }
	    getDrawLib()->getMenuCamera()->setCamera2d();
	  }
#if SIMULATE_SLOW_RENDERING
          SDL_Delay(SIMULATE_SLOW_RENDERING);
#endif
  
          /* When actually playing, check if something happened (like dying or finishing) */
          if(m_State == GS_PLAYING) {        
            _PostUpdatePlaying();
          }

	  /* TODO::MANU::get out !!
          // When did frame rendering end?
          double fEndFrameTime = getXMTime();
          
          // Calculate how large a delay should be inserted after the frame, to keep the 
	  // desired frame rate 
          int nADelay = 0;    
          
	  if (m_State == GS_DEADJUST) {
            setFrameDelay(10);
          } else {
            // become idle only if we hadn't to skip any frame, recently, and more globaly (80% of fps)
            if((nPhysSteps <= 1) && (m_fFPS_Rate > (0.8f / PHYS_STEP_SIZE)))
              nADelay = ((m_fLastPhysTime + PHYS_STEP_SIZE) - fEndFrameTime) * 1000.0f;

	    if(m_autoZoom){
	      // limit framerate while zooming (100 fps)
	      double timeElapsed = getXMTime() - fStartFrameTime;
	      if(timeElapsed < 0.01)
		nADelay = 10 - (int)(timeElapsed*1000.0);
	    }
          }

          if(nADelay > 0) {
            if(m_xmsession->timedemo() == false) {
              setFrameDelay(nADelay);
            }
          }
	  */

          if(m_State == GS_DEADJUST) {
            /* Hmm, you're dead and you know it. */
            _PostUpdateJustDead();
          }
         
          /* Context menu? */
          if(m_State == GS_PREPLAYING || m_State == GS_PLAYING || !m_bEnableContextHelp)
            m_Renderer->getGUI()->enableContextMenuDrawing(false);
          else
            m_Renderer->getGUI()->enableContextMenuDrawing(true);
          
          /* Draw GUI */
	  // only if it's not the autozoom camera
	  if(m_MotoGame.getCurrentCamera() != m_MotoGame.getNumberCameras()){
	    m_Renderer->getGUI()->paint();        
	  }
        
          break;
        }
        catch(Exception &e) {
	  Logger::Log("** Warning ** : drawFrame failed ! (%s)", e.getMsg().c_str());
	  // it doesn't work
	  m_MotoGame.endLevel();
	  setState(m_StateAfterPlaying);
          notifyMsg(splitText(e.getMsg(), 50));
        }
      }
    }

    // states managed via the state manager
    if(m_State == GS_PAUSE    	       ||
       m_State == GS_FINISHED 	       ||
       m_State == GS_DEADMENU 	       ||
       m_State == GS_EDIT_PROFILES     ||
       m_State == GS_LEVEL_INFO_VIEWER ||
       m_State == GS_REPLAYING         ||
       m_State == GS_PLAYING           ||
       m_State == GS_CREDITSMODE
       ) {
      /* draw the game states */
      m_stateManager->update();
      m_stateManager->render(); 
    }

 
    /* */
    /* Draw a little FPS counter */
    if(m_xmsession->fps()) {
      char cTemp[256];        
      sprintf(cTemp,
	      "u(%i) d(%i)",
	      m_stateManager->getCurrentUpdateFPS(),
	      m_stateManager->getCurrentRenderFPS());

      FontManager* v_fm = getDrawLib()->getFontSmall();
      FontGlyph* v_fg = v_fm->getGlyph(cTemp);
      v_fm->printString(v_fg,
			0, 130,
			MAKE_COLOR(255,255,255,255), true);
    }    
       
    /* Draw mouse cursor */
    if(m_bShowCursor)
      _DrawMouseCursor();

    /* system message */
    m_sysMsg->render();
  }

  /*===========================================================================
  Main loop utility functions
  ===========================================================================*/
  void GameApp::_DrawMouseCursor(void) {
    if(!getDrawLib()->isNoGraphics() && m_pCursor != NULL && m_xmsession->ugly() == false) {
      int nMX,nMY;
      getMousePos(&nMX,&nMY);      
      getDrawLib()->drawImage(Vector2f(nMX-2,nMY-2),Vector2f(nMX+30,nMY+30),m_pCursor);
    }
  }
  
  void GameApp::_PrepareFrame(void) {
    m_MotoGame.setDeathAnim(m_bEnableDeathAnim); /* hack hack hack */
    
    /* Update sound system and input */
    if(!getDrawLib()->isNoGraphics()) {        
      Sound::update();

      m_InputHandler.updateInput(m_MotoGame.Players());
    }    
    
    /* Whether or not we should have a mouse cursor? */
    switch(m_State) {
      case GS_MENU:
      case GS_EDIT_WEBCONFIG:
      case GS_LEVELPACK_VIEWER:
        m_bShowCursor = true;
        //SDL_ShowCursor(SDL_ENABLE);
        break;

      break;
    }
  }  

  void GameApp::_PreUpdateGUI(void) {
    /* Quit msg box open? */
    if(m_pQuitMsgBox != NULL) {
      UIMsgBoxButton Button = m_pQuitMsgBox->getClicked();
      if(Button == UI_MSGBOX_YES) {
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
	_UpdateWebTheme(m_Config.getString("Theme"), false);      
      }
      else if(Button == UI_MSGBOX_NO) {
        delete m_pInfoMsgBox;
        m_pInfoMsgBox = NULL;
      }
    }
  }
  
  void GameApp::_UpdateFPSCounter(void) {
    /* Perform a rather precise calculation of the frame rate */    
    m_fFrameTime = getXMTime();
    static int nFPS_Frames = 0;
    static double fFPS_LastTime = 0.0f;
    static double fFPS_CurrentTime = 0.0f;
    
    fFPS_CurrentTime = getXMTime();
    if(fFPS_CurrentTime - fFPS_LastTime > 1.0f && nFPS_Frames>0) {
      m_fFPS_Rate = ((float)nFPS_Frames) / (fFPS_CurrentTime - fFPS_LastTime);
      nFPS_Frames = 0;
      fFPS_LastTime = fFPS_CurrentTime;
    }
    nFPS_Frames++;
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
    m_Renderer->getGUI()->dispatchMouseHover();
    
    /* Blah... */
    if(m_State == GS_MENU)
      _HandleMainMenu();
    else if(m_State == GS_EDIT_WEBCONFIG)
      _HandleWebConfEditor();
    else if(m_State == GS_LEVELPACK_VIEWER)
      _HandleLevelPackViewer();
              
    /* Draw GUI */
    if(m_bEnableContextHelp)
      m_Renderer->getGUI()->enableContextMenuDrawing(true);
    else
      m_Renderer->getGUI()->enableContextMenuDrawing(false);
      
    m_Renderer->getGUI()->paint();                
  }
  
  int GameApp::_UpdateGamePlaying(void) {
    /* Increase frame counter */
    m_nFrame++;

    /* Following time code is made by Eric Piel, but I took the liberty to change the minimum
        frame-miss number from 50 to 10, because it wasn't working well. */

    /* reinitialise if we can't catch up */
    if (m_fLastPhysTime - getXMTime() < -0.1f)
      m_fLastPhysTime = getXMTime() - PHYS_STEP_SIZE;

    /* Update game until we've catched up with the real time */
    int nPhysSteps = 0;
    do {
      if(m_State == GS_PLAYING || m_State == GS_DEADJUST) {
        m_MotoGame.updateLevel(PHYS_STEP_SIZE, m_pJustPlayReplay);
      }
      m_fLastPhysTime += PHYS_STEP_SIZE;
      nPhysSteps++;

#if SIMULATE_SLOW_PHYSICS
      SDL_Delay(SIMULATE_SLOW_PHYSICS);
#endif

      /* don't do this infinitely, maximum miss 10 frames, then give up */
    } while ((m_fLastPhysTime + PHYS_STEP_SIZE <= getXMTime()) && (nPhysSteps < 10));

		for(int i=0; i<m_MotoGame.getNumberCameras(); i++){
			m_MotoGame.setCurrentCamera(i);
			m_MotoGame.getCamera()->setSpeedMultiplier(nPhysSteps);
		}
  
    if(m_xmsession->timedemo() == false) {
      /* Never pass this point while being ahead of time, busy wait until it's time */
      if(nPhysSteps <= 1) {  
        while (m_fLastPhysTime > getXMTime());
      }
    }

    return nPhysSteps;
  }  

  int GameApp::_UpdateGameReplaying(void) {
    int nPhysSteps = 1;
    m_nFrame++;
    static float fGTime = 0, fRTime = 0;
		for(int i=0; i<m_MotoGame.getNumberCameras(); i++){
			m_MotoGame.setCurrentCamera(i);
			m_MotoGame.getCamera()->setSpeedMultiplier(nPhysSteps);
		}

    return nPhysSteps;
  }

  void GameApp::_PostUpdatePlaying(void) {
    bool v_all_dead = true;
    bool v_one_still_play = false;
    bool v_one_finished = false;
 
    for(unsigned int i=0; i<m_MotoGame.Players().size(); i++) {
      if(m_MotoGame.Players()[i]->isDead() == false) {
	v_all_dead = false;
      }
      if(m_MotoGame.Players()[i]->isFinished()) {
	v_one_finished = true;
      }

      if(m_MotoGame.Players()[i]->isFinished() == false && m_MotoGame.Players()[i]->isDead() == false) {
	v_one_still_play = true;
      }
    }
    
    /* News? */
    if(v_one_still_play == false || m_bMultiStopWhenOneFinishes) { // let people continuing when one finished or not
      if(v_one_finished) {
	/* You're done maaaan! :D */
	
	/* finalize the replay */
	if(m_pJustPlayReplay != NULL) {
	  if(m_MotoGame.Players().size() == 1) {
	    /* save the last state because scene don't record each frame */
	    SerializedBikeState BikeState;
	    MotoGame::getSerializedBikeState(m_MotoGame.Players()[0]->getState(), m_MotoGame.getTime(), &BikeState);
	    m_pJustPlayReplay->storeState(BikeState);
	    m_pJustPlayReplay->finishReplay(true,m_MotoGame.Players()[0]->finishTime());
	  }
	}

	/* update profiles */
	float v_finish_time = 0.0;
	std::string TimeStamp = getTimeStamp();
	for(unsigned int i=0; i<m_MotoGame.Players().size(); i++) {
	  if(m_MotoGame.Players()[i]->isFinished()) {
	    v_finish_time  = m_MotoGame.Players()[i]->finishTime();
	  }
	}
	if(m_MotoGame.Players().size() == 1) {
	  m_db->profiles_addFinishTime(m_xmsession->profile(), m_MotoGame.getLevelSrc()->Id(),
				       TimeStamp, v_finish_time);
	}
  
	/* Update stats */
	/* update stats only in one player mode */
	if(m_MotoGame.Players().size() == 1) {       
	  m_db->stats_levelCompleted(m_xmsession->profile(),
				     m_MotoGame.getLevelSrc()->Id(),
				     m_MotoGame.Players()[0]->finishTime());
	  _UpdateLevelsLists();
	  _UpdateCurrentPackList(m_MotoGame.getLevelSrc()->Id(),
				 m_MotoGame.Players()[0]->finishTime());
	}

	m_stateManager->pushState(new StateFinished(this));
      } else if(v_all_dead) {
	/* You're dead maan! */
	setState(GS_DEADJUST);
      }
    }
  }

  void GameApp::_PostUpdateJustDead(void) {
    if(m_xmsession->ugly() == false) {
      if(m_nJustDeadShade < 150) m_nJustDeadShade+=8;
      getDrawLib()->drawBox(Vector2f(0,0),Vector2f(getDrawLib()->getDispWidth(),getDrawLib()->getDispHeight()),0,MAKE_COLOR(0,0,0,m_nJustDeadShade));     
    }
  }
