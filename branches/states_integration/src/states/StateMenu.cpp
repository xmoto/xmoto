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

StateMenu::StateMenu(bool drawStateBehind,
		     bool updateStatesBehind,
		     GameApp* pGame,
		     StateMenuContextReceiver* i_receiver,
		     bool i_doShade,
		     bool i_doShadeAnim):
  GameState(drawStateBehind,
	    updateStatesBehind,
	    pGame,
	    i_doShade, i_doShadeAnim)
{
  m_GUI        = NULL;
  m_showCursor = true;
  m_receiver   = i_receiver;
}

StateMenu::~StateMenu()
{

}


void StateMenu::enter()
{
  GameState::enter();
}

void StateMenu::leave()
{
}

void StateMenu::enterAfterPop()
{
  m_GUI->enableWindow(true);
}

void StateMenu::leaveAfterPush()
{
  m_GUI->enableWindow(false);
}

bool StateMenu::update()
{
  if(doUpdate() == false){
    return false;
  }

  m_GUI->dispatchMouseHover();
  return true;
}

bool StateMenu::render()
{
  GameState::render();
  m_GUI->paint();
  return true;
}

void StateMenu::keyDown(int nKey, SDLMod mod,int nChar)
{
  GameState::keyDown(nKey, mod, nChar);
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
  
  if(nButton == SDL_BUTTON_LEFT)
    m_GUI->mouseLDown(nX, nY);
  else if(nButton == SDL_BUTTON_RIGHT)
    m_GUI->mouseRDown(nX, nY);
  else if(nButton == SDL_BUTTON_WHEELUP)
    m_GUI->mouseWheelUp(nX, nY);
  else if(nButton == SDL_BUTTON_WHEELDOWN)        
    m_GUI->mouseWheelDown(nX, nY);
  
  checkEvents();
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

void StateMenu::send(const std::string& i_id, const std::string& i_message) 
{
  m_commands.push(i_message);
}

void StateMenu::executeOneCommand(std::string cmd)
{
  // default do nothing
}
