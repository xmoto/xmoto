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

#include "StatePause.h"
#include "Game.h"
#include "XMSession.h"
#include "drawlib/DrawLib.h"
#include "GameText.h"
#include "helpers/Log.h"

/* static members */
UIRoot*  StatePause::m_GUI        = NULL;
UIFrame* StatePause::m_pPauseMenu = NULL;

StatePause::StatePause(GameApp* pGame,
		       bool drawStateBehind,
		       bool updateStatesBehind):
  GameState(drawStateBehind,
	    updateStatesBehind,
	    pGame)
{
}

StatePause::~StatePause()
{

}

void StatePause::enter()
{
  m_nPauseShade = 0; 
  m_pGame->m_State = GS_PAUSE; // to be removed, just the time states are finished
  m_pGame->getMotoGame()->setInfos(m_pGame->getMotoGame()->getLevelSrc()->Name());
  m_pGame->setShowCursor(true);
  
  createGUIIfNeeded(m_pGame);
}

void StatePause::leave()
{
  m_pGame->m_State = GS_PLAYING; // to be removed, just the time states are finished
  m_pGame->getMotoGame()->setInfos("");
}

void StatePause::enterAfterPop()
{

}

void StatePause::leaveAfterPush()
{

}

void StatePause::update()
{
  m_GUI->dispatchMouseHover();

//    _HandlePauseMenu();


//    /* Any of the pause menu buttons clicked? */
//    for(int i=0;i<m_nNumPauseMenuButtons;i++) {
//      if(m_pPauseMenuButtons[i]->getCaption() == GAMETEXT_PLAYNEXT) {
//        /* Uhm... is it likely that there's a next level? */
//	m_pPauseMenuButtons[i]->enableWindow(_IsThereANextLevel(m_PlaySpecificLevelId));
//      }
//
//      if(m_pPauseMenuButtons[i]->isClicked()) {
//        if(m_pPauseMenuButtons[i]->getCaption() == GAMETEXT_QUIT) {
//          if(m_pQuitMsgBox == NULL) {
//	    m_Renderer->getGUI()->setFont(drawLib->getFontSmall());
//            m_pQuitMsgBox = m_Renderer->getGUI()->msgBox(GAMETEXT_QUITMESSAGE,
//                                                        (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
//	  }
//        }
//        else if(m_pPauseMenuButtons[i]->getCaption() == GAMETEXT_ABORT) {
//          m_pPauseMenu->showWindow(false);
//	  if(m_MotoGame.Players().size() == 1) {
//	    m_db->stats_abortedLevel(m_xmsession->profile(),
//				     m_MotoGame.getLevelSrc()->Id(),
//				     m_MotoGame.getTime());
//	  }
//
//	  m_MotoGame.getCamera()->setPlayerToFollow(NULL);
//          m_MotoGame.endLevel();
//          m_InputHandler.resetScriptKeyHooks();                     
//          m_Renderer->unprepareForNewLevel();
//
//          setState(m_StateAfterPlaying);
//          //setState(GS_MENU);          
//        }
//        else if(m_pPauseMenuButtons[i]->getCaption() == GAMETEXT_RESTART) {
//          m_pPauseMenu->showWindow(false);
//	  _RestartLevel();
//        }
//        else if(m_pPauseMenuButtons[i]->getCaption() == GAMETEXT_PLAYNEXT) {
//	  std::string NextLevel = _DetermineNextLevel(m_PlaySpecificLevelId);
//	  if(NextLevel != "") {        
//	    m_pPauseMenu->showWindow(false);              
//	    if(m_MotoGame.Players().size() == 1) {
//	      m_db->stats_abortedLevel(m_xmsession->profile(),
//				       m_MotoGame.getLevelSrc()->Id(),
//				       m_MotoGame.getTime());
//	    }
//	    m_MotoGame.getCamera()->setPlayerToFollow(NULL);
//	    m_MotoGame.endLevel();
//	    m_InputHandler.resetScriptKeyHooks();                     
//	    m_Renderer->unprepareForNewLevel();                    
//	    
//	    m_PlaySpecificLevelId = NextLevel;              
//	    
//	    setPrePlayAnim(true);
//              setState(GS_PREPLAYING);
//          }
//        }
//        else if(m_pPauseMenuButtons[i]->getCaption() == GAMETEXT_RESUME) {
//          m_pPauseMenu->showWindow(false);
//	  m_MotoGame.setInfos("");
//          m_State = GS_PLAYING; /* no don't use setState() for this. Old code, depends on madness */
//        }
//
//        /* Don't process this clickin' more than once */
//        m_pPauseMenuButtons[i]->setClicked(false);
//      }
//    }
//  }

}

void StatePause::render()
{
  // rendering of the gui must be done by the mother call : to add here when states will be almost finished
  m_pGame->setFrameDelay(10);

  if(m_pGame->getSession()->ugly() == false) {
    if(m_nPauseShade < 150) m_nPauseShade+=8;

    m_pGame->getDrawLib()->drawBox(Vector2f(0,0),
				   Vector2f(m_pGame->getDrawLib()->getDispWidth(),
					    m_pGame->getDrawLib()->getDispHeight()),
				   0,
				   MAKE_COLOR(0,0,0, m_nPauseShade)
				   );
  }

  m_GUI->paint();
}

void StatePause::keyDown(int nKey, SDLMod mod,int nChar)
{
  switch(nKey) {

  case SDLK_ESCAPE:
    /* quit this state */
    m_requestForEnd = true;
    break;

  case SDLK_F3:
    m_pGame->switchLevelToFavorite(m_pGame->getMotoGame()->getLevelSrc()->Id(), true);
    break;

  default:
    m_GUI->keyDown(nKey, mod, nChar);
    break;

  }
}

void StatePause::keyUp(int nKey, SDLMod mod)
{
  m_GUI->keyUp(nKey, mod);
}

void StatePause::mouseDown(int nButton)
{
  int nX,nY;        
  GameApp::getMousePos(&nX,&nY);
        
  if(nButton == SDL_BUTTON_LEFT)
    m_GUI->mouseLDown(nX,nY);
  else if(nButton == SDL_BUTTON_RIGHT)
    m_GUI->mouseRDown(nX,nY);
  else if(nButton == SDL_BUTTON_WHEELUP)
    m_GUI->mouseWheelUp(nX,nY);
  else if(nButton == SDL_BUTTON_WHEELDOWN)        
    m_GUI->mouseWheelDown(nX,nY);
}

void StatePause::mouseDoubleClick(int nButton)
{
  int nX,nY;        
  GameApp::getMousePos(&nX,&nY);
        
  if(nButton == SDL_BUTTON_LEFT)
    m_GUI->mouseLDoubleClick(nX,nY);
}

void StatePause::mouseUp(int nButton)
{
  int nX,nY;
  GameApp::getMousePos(&nX,&nY);
  
  if(nButton == SDL_BUTTON_LEFT)
    m_GUI->mouseLUp(nX,nY);
  else if(nButton == SDL_BUTTON_RIGHT)
    m_GUI->mouseRUp(nX,nY);
}

void StatePause::clean() {
  if(StatePause::m_GUI != NULL) {
    delete StatePause::m_GUI;
    StatePause::m_GUI = NULL;
  }
}

void StatePause::createGUIIfNeeded(GameApp* pGame) {
  UIButton *v_button;

  if(m_GUI != NULL) return;

  m_GUI = new UIRoot();
  m_GUI->setApp(pGame);
  m_GUI->setFont(pGame->getDrawLib()->getFontSmall()); 
  m_GUI->setPosition(0, 0,
		     pGame->getDrawLib()->getDispWidth(),
		     pGame->getDrawLib()->getDispHeight());


  m_pPauseMenu = new UIFrame(m_GUI,
			     pGame->getDrawLib()->getDispWidth()/2  - 400/2,
			     pGame->getDrawLib()->getDispHeight()/2 - 540/2,
			     "", 400, 540);
  m_pPauseMenu->setStyle(UI_FRAMESTYLE_MENU);

  UIStatic *pPauseText = new UIStatic(m_pPauseMenu, 0, 100, GAMETEXT_PAUSE, m_pPauseMenu->getPosition().nWidth, 36);
  pPauseText->setFont(pGame->getDrawLib()->getFontMedium());

  v_button = new UIButton(m_pPauseMenu, 0, 0, GAMETEXT_RESUME, 207, 57);
  v_button->setContextHelp(CONTEXTHELP_BACK_TO_GAME);
  v_button->setPosition(400/2 - 207/2, m_pPauseMenu->getPosition().nHeight/2 - 5*57/2 + 0*57, 207, 57);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
  m_pPauseMenu->setPrimaryChild(v_button); /* default button */

  v_button = new UIButton(m_pPauseMenu,0,0,GAMETEXT_RESTART,207,57);
  v_button->setContextHelp(CONTEXTHELP_TRY_LEVEL_AGAIN_FROM_BEGINNING);
  v_button->setPosition(400/2 - 207/2, m_pPauseMenu->getPosition().nHeight/2 - 5*57/2 + 1*57, 207, 57);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());

  v_button = new UIButton(m_pPauseMenu,0,0,GAMETEXT_PLAYNEXT,207,57);
  v_button->setContextHelp(CONTEXTHELP_PLAY_NEXT_INSTEAD);
  v_button->setPosition(400/2 - 207/2, m_pPauseMenu->getPosition().nHeight/2 - 5*57/2 + 2*57, 207, 57);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());

  v_button = new UIButton(m_pPauseMenu,0,0,GAMETEXT_ABORT,207,57);
  v_button->setContextHelp(CONTEXTHELP_BACK_TO_MAIN_MENU);
  v_button->setPosition(400/2 - 207/2, m_pPauseMenu->getPosition().nHeight/2 - 5*57/2 + 3*57, 207, 57);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
  
  v_button = new UIButton(m_pPauseMenu,0,0,GAMETEXT_QUIT,207,57);
  v_button->setContextHelp(CONTEXTHELP_QUIT_THE_GAME);
  v_button->setPosition(400/2 - 207/2, m_pPauseMenu->getPosition().nHeight/2 - 5*57/2 + 4*57, 207, 57);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());

}
