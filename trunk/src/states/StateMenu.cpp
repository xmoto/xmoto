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
#include "../gui/basic/GUI.h"
#include "../Game.h"
#include "../XMSession.h"
#include "../drawlib/DrawLib.h"
#include "StateUpdateDb.h"
#include "../helpers/Log.h"

StateMenu::StateMenu(bool drawStateBehind,
		     bool updateStatesBehind,
		     bool i_doShade,
		     bool i_doShadeAnim):
  GameState(drawStateBehind,
	    updateStatesBehind,
	    i_doShade, i_doShadeAnim)
{
  m_GUI        = NULL;
  m_showCursor = true;

  m_renderFps = 30; // is enouh for menus
  m_updateFps = 30;
}

StateMenu::~StateMenu()
{
}

void StateMenu::enter()
{
  GameState::enter();
  m_GUI->enableContextMenuDrawing(XMSession::instance()->enableContextHelp());
}

void StateMenu::leave()
{
}

void StateMenu::enterAfterPop()
{
  GameState::enterAfterPop();
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

void StateMenu::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  int nX,nY;
  Uint8 nButton;
  Uint8 v_joyNum;
  Uint8 v_joyAxis;
  Sint16 v_joyAxisValue;
  Uint8 v_joyButton;

  if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_F5, KMOD_NONE)) {
    StateManager::instance()->pushState(new StateUpdateDb());
  }

  else if(i_xmkey.isCharInput()) {
    if(i_xmkey.getCharInputMod() == KMOD_NONE) {
      switch(i_type) {
      case INPUT_DOWN:
	m_GUI->keyDown(i_xmkey.getCharInputKey(), i_xmkey.getCharInputMod(), i_xmkey.getCharInputUtf8());
	break;
      case INPUT_UP:
	m_GUI->keyUp(i_xmkey.getCharInputKey(), i_xmkey.getCharInputMod(), i_xmkey.getCharInputUtf8());
	break;
      }
      checkEvents();
    }
  }

  else if(i_xmkey.toMouse(nX, nY, nButton)) {
    if(i_xmkey.getRepetition() == 1) {
      if(i_type == INPUT_DOWN) {
	if(nButton == SDL_BUTTON_LEFT) {
	  m_GUI->mouseLDown(nX, nY);
	  checkEvents();
	} else if(nButton == SDL_BUTTON_RIGHT) {
	  m_GUI->mouseRDown(nX, nY);
	  checkEvents();
	} else if(nButton == SDL_BUTTON_WHEELUP) {
	  m_GUI->mouseWheelUp(nX, nY);
	  checkEvents();
	} else if(nButton == SDL_BUTTON_WHEELDOWN) {
	  m_GUI->mouseWheelDown(nX, nY);
	  checkEvents();
	}
	
      } else if(i_type == INPUT_UP) {
	if(nButton == SDL_BUTTON_LEFT) {
	  m_GUI->mouseLUp(nX, nY);
	  checkEvents();
	} else if(nButton == SDL_BUTTON_RIGHT) {
	  m_GUI->mouseRUp(nX, nY);
	  checkEvents();
	}
      }
    } else if(i_xmkey.getRepetition() == 2) {
      if(nButton == SDL_BUTTON_LEFT) {
	m_GUI->mouseLDoubleClick(nX, nY);
	checkEvents();
      }
    }
  }

  else if(i_type == INPUT_DOWN && i_xmkey.toJoystickButton(v_joyNum, v_joyButton)) {
    m_GUI->joystickButtonDown(v_joyNum, v_joyButton);
    checkEvents();
  }

  else if(i_xmkey.toJoystickAxisMotion(v_joyNum, v_joyAxis, v_joyAxisValue)) {
    m_GUI->joystickAxisMotion(v_joyNum, v_joyAxis, v_joyAxisValue);
  }

  GameState::xmKey(i_type, i_xmkey);
}
