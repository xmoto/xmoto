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

#include "StateUpdate.h"
#include "Game.h"
#include "drawlib/DrawLib.h"
#include "GameText.h"
#include "XMSession.h"


/* static members */
UIRoot*  StateUpdate::m_sGUI = NULL;

StateUpdate::StateUpdate(GameApp* pGame,
			 bool drawStateBehind,
			 bool updateStatesBehind):
  StateMenu(drawStateBehind,
	    updateStatesBehind,
	    pGame)
{
  m_name  = "StateUpdate";
  m_threadStarted = false;
}

StateUpdate::~StateUpdate()
{

}

void StateUpdate::enter()
{
  createGUIIfNeeded(m_pGame);
  m_GUI = m_sGUI;
  updateGUI();

  StateMenu::enter();
}

void StateUpdate::leave()
{
  StateMenu::leave();
}

void StateUpdate::enterAfterPop()
{
  StateMenu::enterAfterPop();
}

void StateUpdate::leaveAfterPush()
{
  StateMenu::leaveAfterPush();
}

void StateUpdate::checkEvents()
{
}

bool StateUpdate::update()
{
  updateGUI();

  return StateMenu::update();
}

bool StateUpdate::render()
{
  return StateMenu::render();
}

void StateUpdate::keyDown(int nKey, SDLMod mod,int nChar)
{
  // don't call the parent keydown function. 
  // because it would allow to press F5 again for example
  checkEvents();
}

void StateUpdate::keyUp(int nKey,   SDLMod mod)
{
}

void StateUpdate::mouseDown(int nButton)
{
  StateMenu::mouseDown(nButton);
}

void StateUpdate::mouseDoubleClick(int nButton)
{
  StateMenu::mouseDoubleClick(nButton);
}

void StateUpdate::mouseUp(int nButton)
{
  StateMenu::mouseUp(nButton);
}

void StateUpdate::clean()
{
  if(StateUpdate::m_sGUI != NULL) {
    delete StateUpdate::m_sGUI;
    StateUpdate::m_sGUI = NULL;
  }
}

void StateUpdate::createGUIIfNeeded(GameApp* pGame)
{
  if(m_sGUI != NULL)
    return;

  DrawLib* drawLib = pGame->getDrawLib();

  m_sGUI = new UIRoot();
  m_sGUI->setApp(pGame);
  m_sGUI->setFont(drawLib->getFontSmall()); 
  m_sGUI->setPosition(0, 0,
		      drawLib->getDispWidth(),
		      drawLib->getDispHeight());

  /* Initialize level info viewer */
  int width = drawLib->getDispWidth();
  int height= drawLib->getDispHeight();

  int x = width / 8;
  int y = height / 4;
  std::string caption = "";
  int nWidth  = width * 3/4;
  int nHeight = height / 2;

  UIFrame  *v_frame;
  v_frame = new UIFrame(m_sGUI, x, y, caption, nWidth, nHeight); 
  v_frame->setID("LEVEL_UPDATE_FRAME");
  v_frame->setStyle(UI_FRAMESTYLE_TRANS);
}
