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

    }

    // states managed via the state manager
    if(m_State == GS_PAUSE    	       ||
       m_State == GS_FINISHED 	       ||
       m_State == GS_DEADMENU 	       ||
       m_State == GS_EDIT_PROFILES     ||
       m_State == GS_LEVEL_INFO_VIEWER ||
       m_State == GS_REPLAYING         ||
       m_State == GS_PLAYING           ||
       m_State == GS_PREPLAYING        ||
       m_State == GS_CREDITSMODE       ||
       m_State == GS_DEADJUST
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
