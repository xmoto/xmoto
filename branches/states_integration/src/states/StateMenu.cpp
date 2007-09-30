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

#include "StateMenu.h"
#include "gui/basic/GUI.h"
#include "Game.h"
#include "XMSession.h"
#include "drawlib/DrawLib.h"

#define MENU_SHADING_TIME 0.3
#define MENU_SHADING_VALUE 150

StateMenu::StateMenu(bool drawStateBehind,
		     bool updateStatesBehind,
		     GameApp* pGame,
		     bool i_doShade,
		     bool i_doShadeAnim):
  GameState(drawStateBehind,
	    updateStatesBehind,
	    pGame)
{
  m_GUI = NULL;
  m_doShade     = i_doShade;
  m_doShadeAnim = i_doShadeAnim;
}

StateMenu::~StateMenu()
{

}


void StateMenu::enter()
{
  m_nShadeTime = GameApp::getXMTime();
}

void StateMenu::leave()
{

}

void StateMenu::enterAfterPop()
{

}

void StateMenu::leaveAfterPush()
{

}

void StateMenu::update()
{
  m_GUI->dispatchMouseHover();
}

void StateMenu::render()
{
  // rendering of the gui must be done by the mother call : to add here when states will be almost finished
  m_pGame->setFrameDelay(10);

  if(m_pGame->getSession()->ugly() == false && m_doShade) {
    float v_currentTime = GameApp::getXMTime();
    int   v_nShade;

    if(v_currentTime - m_nShadeTime < MENU_SHADING_TIME && m_doShadeAnim) {
      v_nShade = (int ) ((v_currentTime - m_nShadeTime) * (MENU_SHADING_VALUE / MENU_SHADING_TIME));
    } else {
      v_nShade = MENU_SHADING_VALUE;
    }

    m_pGame->getDrawLib()->drawBox(Vector2f(0,0),
				   Vector2f(m_pGame->getDrawLib()->getDispWidth(),
					    m_pGame->getDrawLib()->getDispHeight()),
				   0,
				   MAKE_COLOR(0,0,0, v_nShade)
				   );
  }

  m_GUI->paint();
}

void StateMenu::keyDown(int nKey, SDLMod mod,int nChar)
{
  m_GUI->keyDown(nKey, mod, nChar);
  checkEvents();
}

void StateMenu::keyUp(int nKey, SDLMod mod)
{
  m_GUI->keyUp(nKey, mod);
  checkEvents();
}

void StateMenu::mouseDown(int nButton)
{
  int nX,nY;        
  GameApp::getMousePos(&nX,&nY);
        
  if(nButton == SDL_BUTTON_LEFT) {
    m_GUI->mouseLDown(nX,nY);
    checkEvents();
  } else if(nButton == SDL_BUTTON_RIGHT) {
    m_GUI->mouseRDown(nX,nY);
    checkEvents();
  } else if(nButton == SDL_BUTTON_WHEELUP) {
    m_GUI->mouseWheelUp(nX,nY);
    checkEvents();
  } else if(nButton == SDL_BUTTON_WHEELDOWN) {
    m_GUI->mouseWheelDown(nX,nY);
    checkEvents();
  }
}

void StateMenu::mouseDoubleClick(int nButton)
{
  int nX,nY;        
  GameApp::getMousePos(&nX,&nY);
        
  if(nButton == SDL_BUTTON_LEFT) {
    m_GUI->mouseLDoubleClick(nX,nY);
    checkEvents();
  }
}

void StateMenu::mouseUp(int nButton)
{
  int nX,nY;
  GameApp::getMousePos(&nX,&nY);
  
  if(nButton == SDL_BUTTON_LEFT) {
    m_GUI->mouseLUp(nX,nY);
    checkEvents();
  } else if(nButton == SDL_BUTTON_RIGHT) {
    m_GUI->mouseRUp(nX,nY);
    checkEvents();
  }
}
