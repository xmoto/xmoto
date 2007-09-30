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

#include "StateDeadMenu.h"
#include "Game.h"
#include "drawlib/DrawLib.h"
#include "GameText.h"

/* static members */
UIRoot*  StateDeadMenu::m_sGUI = NULL;

StateDeadMenu::StateDeadMenu(GameApp* pGame,
			     bool i_doShadeAnim,
			     bool drawStateBehind,
			     bool updateStatesBehind):
  StateMenu(drawStateBehind,
	    updateStatesBehind,
	    pGame,
	    i_doShadeAnim)
{

}

StateDeadMenu::~StateDeadMenu()
{

}


void StateDeadMenu::enter()
{
  StateMenu::enter();

  m_pGame->m_State = GS_DEADMENU; // to be removed, just the time states are finished
  m_pGame->getMotoGame()->setInfos(m_pGame->getMotoGame()->getLevelSrc()->Name());
  m_pGame->setShowCursor(true);
  m_pGame->playMusic("");
  
  createGUIIfNeeded(m_pGame);
  m_GUI = m_sGUI;

  /* reset the playnext button */
  //UIButton *playNextButton = reinterpret_cast<UIButton *>(m_GUI->getChild("PAUSE_FRAME:PLAYNEXT_BUTTON"));
  //playNextButton->enableWindow(m_pGame->isThereANextLevel(m_pGame->getMotoGame()->getLevelSrc()->Id()));
}

void StateDeadMenu::leave()
{
  m_pGame->getMotoGame()->setInfos("");
}

void StateDeadMenu::enterAfterPop()
{

}

void StateDeadMenu::leaveAfterPush()
{

}

void StateDeadMenu::checkEvents() {
}

void StateDeadMenu::update()
{
  StateMenu::update();
}

void StateDeadMenu::render()
{
  StateMenu::render();
}

void StateDeadMenu::keyDown(int nKey, SDLMod mod,int nChar)
{
  switch(nKey) {

  case SDLK_ESCAPE:
    /* quit this state */
    m_pGame->abortPlaying();
    m_pGame->setState(m_pGame->m_StateAfterPlaying); // to be removed, just the time states are finished
    m_requestForEnd = true;
    break;

  default:
    StateMenu::keyDown(nKey, mod, nChar);
    checkEvents();
    break;

  }
}

void StateDeadMenu::keyUp(int nKey,   SDLMod mod)
{
  StateMenu::keyUp(nKey, mod);
}

void StateDeadMenu::mouseDown(int nButton)
{
  StateMenu::mouseDown(nButton);
}

void StateDeadMenu::mouseDoubleClick(int nButton)
{
  StateMenu::mouseDoubleClick(nButton);
}

void StateDeadMenu::mouseUp(int nButton)
{
  StateMenu::mouseUp(nButton);
}

void StateDeadMenu::clean() {
  if(StateDeadMenu::m_sGUI != NULL) {
    delete StateDeadMenu::m_sGUI;
    StateDeadMenu::m_sGUI = NULL;
  }
}

void StateDeadMenu::createGUIIfNeeded(GameApp* pGame) {
  UIButton *v_button;
  UIFrame  *v_frame;

  if(m_sGUI != NULL) return;

  m_sGUI = new UIRoot();
  m_sGUI->setApp(pGame);
  m_sGUI->setFont(pGame->getDrawLib()->getFontSmall()); 
  m_sGUI->setPosition(0, 0,
		      pGame->getDrawLib()->getDispWidth(),
		      pGame->getDrawLib()->getDispHeight());
}



//      /* In-game JUSTDEAD menu fun */
//      UIFrame *m_pJustDeadMenu;
//      int m_nJustDeadShade;
//      UIButton *m_pJustDeadMenuButtons[10];            
//      int m_nNumJustDeadMenuButtons;     

//    /* Initialize just-dead menu */
//    m_pJustDeadMenu = new UIFrame(m_Renderer->getGUI(),drawLib->getDispWidth()/2 - 200,70,"",400,540);
//    m_pJustDeadMenu->setStyle(UI_FRAMESTYLE_MENU);
//    
//    m_pJustDeadMenuButtons[0] = new UIButton(m_pJustDeadMenu,0,0,GAMETEXT_TRYAGAIN,207,57);
//    m_pJustDeadMenuButtons[0]->setContextHelp(CONTEXTHELP_TRY_LEVEL_AGAIN);
//    
//    m_pJustDeadMenuButtons[1] = new UIButton(m_pJustDeadMenu,0,0,GAMETEXT_SAVEREPLAY,207,57);
//    m_pJustDeadMenuButtons[1]->setContextHelp(CONTEXTHELP_SAVE_A_REPLAY);    
//    if(!m_bRecordReplays) m_pJustDeadMenuButtons[1]->enableWindow(false);
//    
//    m_pJustDeadMenuButtons[2] = new UIButton(m_pJustDeadMenu,0,0,GAMETEXT_PLAYNEXT,207,57);
//    m_pJustDeadMenuButtons[2]->setContextHelp(CONTEXTHELP_PLAY_NEXT_LEVEL);
//    
//    m_pJustDeadMenuButtons[3] = new UIButton(m_pJustDeadMenu,0,0,GAMETEXT_ABORT,207,57);
//    m_pJustDeadMenuButtons[3]->setContextHelp(CONTEXTHELP_BACK_TO_MAIN_MENU);
//    
//    m_pJustDeadMenuButtons[4] = new UIButton(m_pJustDeadMenu,0,0,GAMETEXT_QUIT,207,57);
//    m_pJustDeadMenuButtons[4]->setContextHelp(CONTEXTHELP_QUIT_THE_GAME);
//    m_nNumJustDeadMenuButtons = 5;
//
//    UIStatic *pJustDeadText = new UIStatic(m_pJustDeadMenu,0,100,GAMETEXT_JUSTDEAD,m_pJustDeadMenu->getPosition().nWidth,36);
//    pJustDeadText->setFont(drawLib->getFontMedium());
//    
//    for(int i=0;i<m_nNumJustDeadMenuButtons;i++) {
//      m_pJustDeadMenuButtons[i]->setPosition( 200 -207/2,10+m_pJustDeadMenu->getPosition().nHeight/2 - (m_nNumJustDeadMenuButtons*57)/2 + i*57,207,57);
//      m_pJustDeadMenuButtons[i]->setFont(drawLib->getFontSmall());
//    }
//
//    m_pJustDeadMenu->setPrimaryChild(m_pJustDeadMenuButtons[0]); /* default button: Try Again */


//  void GameApp::_HandleJustDeadMenu(void) {
//    /* Is savereplay box open? */
//    if(m_pSaveReplayMsgBox != NULL) {
//      UIMsgBoxButton Clicked = m_pSaveReplayMsgBox->getClicked();
//      if(Clicked != UI_MSGBOX_NOTHING) {
//        std::string Name = m_pSaveReplayMsgBox->getTextInput();
//      
//        delete m_pSaveReplayMsgBox;
//        m_pSaveReplayMsgBox = NULL;
//
//        if(Clicked == UI_MSGBOX_OK) {
//          saveReplay(Name);
//        }        
//      }
//    }
//    
//    /* Any of the just-dead menu buttons clicked? */
//    for(int i=0;i<m_nNumJustDeadMenuButtons;i++) {
//      if(m_pJustDeadMenuButtons[i]->getCaption() == GAMETEXT_SAVEREPLAY) {
//        /* Have we recorded a replay? If not then disable the "Save Replay" button */
//        if(m_pJustPlayReplay == NULL || m_MotoGame.Players().size() != 1) {
//          m_pJustDeadMenuButtons[i]->enableWindow(false);
//        }
//        else {
//          m_pJustDeadMenuButtons[i]->enableWindow(true);
//        }
//      }    
//
//      if(m_pJustDeadMenuButtons[i]->getCaption() == GAMETEXT_PLAYNEXT) {
//        /* Uhm... is it likely that there's a next level? */
//	m_pJustDeadMenuButtons[i]->enableWindow(isThereANextLevel(m_PlaySpecificLevelId));
//      }
//      
//      if(m_pJustDeadMenuButtons[i]->isClicked()) {
//        if(m_pJustDeadMenuButtons[i]->getCaption() == GAMETEXT_QUIT) {
//          if(m_pQuitMsgBox == NULL) {
//	    m_Renderer->getGUI()->setFont(drawLib->getFontSmall());
//            m_pQuitMsgBox = m_Renderer->getGUI()->msgBox(GAMETEXT_QUITMESSAGE,
//                                                        (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
//	  }
//        }
//        else if(m_pJustDeadMenuButtons[i]->getCaption() == GAMETEXT_TRYAGAIN) {
//
//          m_pJustDeadMenu->showWindow(false);
//	  restartLevel();
//        }
//        else if(m_pJustDeadMenuButtons[i]->getCaption() == GAMETEXT_PLAYNEXT) {
//	  std::string NextLevel = _DetermineNextLevel(m_PlaySpecificLevelId);
//	  if(NextLevel != "") {        
//	    m_pJustDeadMenu->showWindow(false);
//	    m_MotoGame.getCamera()->setPlayerToFollow(NULL);
//	    m_MotoGame.endLevel();
//	    m_InputHandler.resetScriptKeyHooks();                     
//	    m_Renderer->unprepareForNewLevel();                    
//	    
//	    m_PlaySpecificLevelId = NextLevel;
//	    
//	    setPrePlayAnim(true);
//	    setState(GS_PREPLAYING);                               
//          }
//        }
//        else if(m_pJustDeadMenuButtons[i]->getCaption() == GAMETEXT_SAVEREPLAY) {
//          if(m_pJustPlayReplay != NULL) {
//            if(m_pSaveReplayMsgBox == NULL) {
//	      m_Renderer->getGUI()->setFont(drawLib->getFontSmall());
//              m_pSaveReplayMsgBox = m_Renderer->getGUI()->msgBox(std::string(GAMETEXT_ENTERREPLAYNAME) + ":",
//                                                                (UIMsgBoxButton)(UI_MSGBOX_OK|UI_MSGBOX_CANCEL),
//                                                                true);
//              m_pSaveReplayMsgBox->setTextInputFont(drawLib->getFontMedium());
//	      m_pSaveReplayMsgBox->setTextInput(Replay::giveAutomaticName());
//            }          
//          }
//        }
//        else if(m_pJustDeadMenuButtons[i]->getCaption() == GAMETEXT_ABORT) {
//          m_pJustDeadMenu->showWindow(false);
//	  m_MotoGame.getCamera()->setPlayerToFollow(NULL);
//          m_MotoGame.endLevel();
//          m_InputHandler.resetScriptKeyHooks();                     
//          m_Renderer->unprepareForNewLevel();
//          //setState(GS_MENU);
//          setState(m_StateAfterPlaying);
//        }
//
//        /* Don't process this clickin' more than once */
//        m_pJustDeadMenuButtons[i]->setClicked(false);
//      }
//    }
//  }
