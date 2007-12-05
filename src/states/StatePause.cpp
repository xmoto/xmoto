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
#include "drawlib/DrawLib.h"
#include "GameText.h"
#include "StateMessageBox.h"
#include "helpers/Log.h"
#include "StateMenuContextReceiver.h"

/* static members */
UIRoot*  StatePause::m_sGUI = NULL;

StatePause::StatePause(GameApp* pGame,
		       StateMenuContextReceiver* i_receiver,
		       bool drawStateBehind,
		       bool updateStatesBehind):
  StateMenu(drawStateBehind,
	    updateStatesBehind,
	    pGame,
	    i_receiver,
	    true)
{
  m_name  = "StatePause";
}

StatePause::~StatePause()
{

}

void StatePause::enter()
{
  m_pGame->getMotoGame()->setInfos(m_pGame->getMotoGame()->getLevelSrc()->Name());
  
  createGUIIfNeeded(m_pGame);
  m_GUI = m_sGUI;

  /* reset the playnext button */
  UIButton *playNextButton = reinterpret_cast<UIButton *>(m_GUI->getChild("PAUSE_FRAME:PLAYNEXT_BUTTON"));
  playNextButton->enableWindow(m_pGame->isThereANextLevel(m_pGame->getMotoGame()->getLevelSrc()->Id()));

  StateMenu::enter();
}

void StatePause::leave()
{
  StateMenu::leave();
  m_pGame->getMotoGame()->setInfos("");
}

void StatePause::enterAfterPop()
{
  StateMenu::enterAfterPop();
}

void StatePause::leaveAfterPush()
{
  StateMenu::leaveAfterPush();
}

void StatePause::checkEvents() {
  UIButton *pResumeButton = reinterpret_cast<UIButton *>(m_GUI->getChild("PAUSE_FRAME:RESUME_BUTTON"));
  if(pResumeButton->isClicked()) {
    pResumeButton->setClicked(false);

    m_requestForEnd = true;
  }

  UIButton *pRestartButton = reinterpret_cast<UIButton *>(m_GUI->getChild("PAUSE_FRAME:RESTART_BUTTON"));
  if(pRestartButton->isClicked()) {
    pRestartButton->setClicked(false);

    m_requestForEnd = true;
    if(m_receiver != NULL) {
      Logger::Log("StatePause::checkEvents RESTART");
      m_receiver->send(getId(), "RESTART");
    }
  }

  UIButton *pPlayNextButton = reinterpret_cast<UIButton *>(m_GUI->getChild("PAUSE_FRAME:PLAYNEXT_BUTTON"));
  if(pPlayNextButton->isClicked()) {
    pPlayNextButton->setClicked(false);

    m_requestForEnd = true;    
    if(m_receiver != NULL) {
      m_receiver->send(getId(), "NEXTLEVEL");
    }
  }

  UIButton *pAbortButton = reinterpret_cast<UIButton *>(m_GUI->getChild("PAUSE_FRAME:ABORT_BUTTON"));
  if(pAbortButton->isClicked()) {
    pAbortButton->setClicked(false);

    m_requestForEnd = true;
    if(m_receiver != NULL) {
      m_receiver->send(getId(), "ABORT");
    }
  }

  UIButton *pQuitButton = reinterpret_cast<UIButton *>(m_GUI->getChild("PAUSE_FRAME:QUIT_BUTTON"));
  if(pQuitButton->isClicked()) {
    pQuitButton->setClicked(false);

    StateMessageBox* v_msgboxState = new StateMessageBox(this, m_pGame, GAMETEXT_QUITMESSAGE, UI_MSGBOX_YES|UI_MSGBOX_NO);
    v_msgboxState->setId("QUIT");
    m_pGame->getStateManager()->pushState(v_msgboxState);
  }
}

bool StatePause::update()
{
  return StateMenu::update();
}

bool StatePause::render()
{
  return StateMenu::render();
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
    StateMenu::keyDown(nKey, mod, nChar);
    checkEvents();
    break;

  }
}

void StatePause::keyUp(int nKey, SDLMod mod)
{
  StateMenu::keyUp(nKey, mod);
}

void StatePause::mouseDown(int nButton)
{
  StateMenu::mouseDown(nButton);
}

void StatePause::mouseDoubleClick(int nButton)
{
  StateMenu::mouseDoubleClick(nButton);
}

void StatePause::mouseUp(int nButton)
{
  StateMenu::mouseUp(nButton);
}

void StatePause::clean() {
  if(StatePause::m_sGUI != NULL) {
    delete StatePause::m_sGUI;
    StatePause::m_sGUI = NULL;
  }
}

void StatePause::createGUIIfNeeded(GameApp* pGame) {
  UIButton *v_button;
  UIFrame  *v_frame;

  if(m_sGUI != NULL) return;

  m_sGUI = new UIRoot();
  m_sGUI->setApp(pGame);
  m_sGUI->setFont(pGame->getDrawLib()->getFontSmall()); 
  m_sGUI->setPosition(0, 0,
		      pGame->getDrawLib()->getDispWidth(),
		      pGame->getDrawLib()->getDispHeight());
  
  
  v_frame = new UIFrame(m_sGUI,
			pGame->getDrawLib()->getDispWidth()/2  - 400/2,
			pGame->getDrawLib()->getDispHeight()/2 - 540/2,
			"", 400, 540);
  v_frame->setID("PAUSE_FRAME");
  v_frame->setStyle(UI_FRAMESTYLE_MENU);

  UIStatic *pPauseText = new UIStatic(v_frame, 0, 100, GAMETEXT_PAUSE, v_frame->getPosition().nWidth, 36);
  pPauseText->setFont(pGame->getDrawLib()->getFontMedium());

  v_button = new UIButton(v_frame, 400/2 - 207/2, v_frame->getPosition().nHeight/2 - 5*57/2 + 0*57, GAMETEXT_RESUME, 207, 57);
  v_button->setID("RESUME_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_BACK_TO_GAME);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
  v_frame->setPrimaryChild(v_button); /* default button */

  v_button = new UIButton(v_frame, 400/2 - 207/2, v_frame->getPosition().nHeight/2 - 5*57/2 + 1*57, GAMETEXT_RESTART, 207, 57);
  v_button->setID("RESTART_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_TRY_LEVEL_AGAIN_FROM_BEGINNING);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());

  v_button = new UIButton(v_frame, 400/2 - 207/2, v_frame->getPosition().nHeight/2 - 5*57/2 + 2*57, GAMETEXT_PLAYNEXT, 207, 57);
  v_button->setID("PLAYNEXT_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_PLAY_NEXT_INSTEAD);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());

  v_button = new UIButton(v_frame, 400/2 - 207/2, v_frame->getPosition().nHeight/2 - 5*57/2 + 3*57, GAMETEXT_ABORT, 207, 57);
  v_button->setID("ABORT_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_BACK_TO_MAIN_MENU);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
  
  v_button = new UIButton(v_frame, 400/2 - 207/2, v_frame->getPosition().nHeight/2 - 5*57/2 + 4*57, GAMETEXT_QUIT, 207, 57);
  v_button->setID("QUIT_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_QUIT_THE_GAME);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());

}

void StatePause::send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input) {
  if(i_id == "QUIT") {
    switch(i_button) {
    case UI_MSGBOX_YES:
      m_requestForEnd = true;
      m_pGame->requestEnd();
      break;
    case UI_MSGBOX_NO:
      return;
      break;
    }
  }
}
