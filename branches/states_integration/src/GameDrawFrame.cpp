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

    m_stateManager->update();
    m_stateManager->render(); 
  }
  
  void GameApp::_PrepareFrame(void) {
    Sound::update();
    m_InputHandler.updateInput(m_MotoGame.Players());
  }  

  void GameApp::_PreUpdateGUI(void) {
    /* What about the notify box then? */
    if(m_pNotifyMsgBox != NULL) {
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

  void GameApp::_DrawMainGUI(void) {
    /* Update mouse stuff */
    m_Renderer->getGUI()->dispatchMouseHover();
    
    /* Blah... */
    if(m_State == GS_MENU)
      _HandleMainMenu();
              
    /* Draw GUI */
    if(m_bEnableContextHelp)
      m_Renderer->getGUI()->enableContextMenuDrawing(true);
    else
      m_Renderer->getGUI()->enableContextMenuDrawing(false);
      
    m_Renderer->getGUI()->paint();                
  }
