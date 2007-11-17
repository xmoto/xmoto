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

#include "StateRequestKey.h"
#include "Game.h"
#include "GameText.h"
#include "drawlib/DrawLib.h"

/* static members */
UIRoot*  StateRequestKey::m_sGUI = NULL;

StateRequestKey::StateRequestKey(GameApp* pGame,
				 StateMenuContextReceiver* i_receiver,
				 bool drawStateBehind,
				 bool updateStatesBehind):
StateMenu(drawStateBehind,
	  updateStatesBehind,
	  pGame,
	  i_receiver,
	  false,
	  true)
{
  m_name  = "StateRequestKey";
}

StateRequestKey::~StateRequestKey()
{

}


void StateRequestKey::enter()
{
  createGUIIfNeeded(m_pGame);
  m_GUI = m_sGUI;

  StateMenu::enter();
}

void StateRequestKey::leave()
{
}

void StateRequestKey::enterAfterPop()
{
  StateMenu::enterAfterPop();
}

void StateRequestKey::leaveAfterPush()
{
  StateMenu::leaveAfterPush();
}

bool StateRequestKey::update()
{
  return StateMenu::update();
}

bool StateRequestKey::render()
{
  return StateMenu::render();
}

void StateRequestKey::keyDown(int nKey, SDLMod mod,int nChar)
{
  std::string v_msg;

  switch(nKey) {
  case SDLK_ESCAPE:
    m_requestForEnd = true;
    break;
  default:
    v_msg = InputHandler::keyToString(nKey);

    if(m_receiver != NULL) {
      if(v_msg != "") {
	m_requestForEnd = true;
	m_receiver->send("REQUESTKEY", v_msg);
      }
    }
  }
}

void StateRequestKey::keyUp(int nKey, SDLMod mod)
{
}

void StateRequestKey::mouseDown(int nButton)
{
}

void StateRequestKey::mouseDoubleClick(int nButton)
{
}

void StateRequestKey::mouseUp(int nButton)
{
}

void StateRequestKey::checkEvents() {
}

void StateRequestKey::clean() {
  if(StateRequestKey::m_sGUI != NULL) {
    delete StateRequestKey::m_sGUI;
    StateRequestKey::m_sGUI = NULL;
  }
}

void StateRequestKey::createGUIIfNeeded(GameApp* pGame) {
  UIStatic *v_someText;
  UIFrame  *v_frame;

  if(m_sGUI != NULL) return;

  m_sGUI = new UIRoot();
  m_sGUI->setApp(pGame);
  m_sGUI->setFont(pGame->getDrawLib()->getFontSmall()); 
  m_sGUI->setPosition(0, 0,
		      pGame->getDrawLib()->getDispWidth(),
		      pGame->getDrawLib()->getDispHeight());

  v_frame = new UIFrame(m_sGUI,
			20,
			pGame->getDrawLib()->getDispHeight()/2 - 50,
			"", pGame->getDrawLib()->getDispWidth() -20*2, 50*2);
  v_frame->setID("FRAME");

  v_someText = new UIStatic(v_frame, 0, 0, GAMETEXT_PRESSANYKEYTO, v_frame->getPosition().nWidth, v_frame->getPosition().nHeight);
  v_someText->setFont(pGame->getDrawLib()->getFontMedium());            
  v_someText->setHAlign(UI_ALIGN_CENTER);
}
