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
#include "StateUpdateDb.h"
#include "common/XMSession.h"
#include "drawlib/DrawLib.h"
#include "gui/basic/GUI.h"
#include "helpers/Log.h"
#include "xmoto/Game.h"
#include "xmoto/input/Input.h"
#include "xmoto/input/Joystick.h"
#include "xmscene/Camera.h"
#include <cstdlib>
#include <stdint.h>
#include <utility>

StateMenu::StateMenu(bool drawStateBehind, bool updateStatesBehind)
  : GameState(drawStateBehind, updateStatesBehind) {
  m_GUI = NULL;
  m_showCursor = true;

  m_renderFps = XMSession::instance()->maxRenderFps();
  m_updateFps = 30;
}

StateMenu::~StateMenu() {}

void StateMenu::enter() {
  GameState::enter();
  m_GUI->enableContextMenuDrawing(XMSession::instance()->enableContextHelp());
}

void StateMenu::leave() {}

void StateMenu::enterAfterPop() {
  GameState::enterAfterPop();
  m_GUI->enableWindow(true);

  JoystickInput::instance()->resetJoyAxes();
}

void StateMenu::leaveAfterPush() {
  m_GUI->enableWindow(false);
}

bool StateMenu::update() {
  if (doUpdate() == false) {
    return false;
  }

  m_GUI->dispatchMouseHover();
  return true;
}

bool StateMenu::render() {
  GameState::render();
  DrawLib *pDrawlib = GameApp::instance()->getDrawLib();
  pDrawlib->getMenuCamera()->setCamera2d();
  m_GUI->paint();
  return true;
}

void StateMenu::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  int nX, nY;
  Uint8 nButton;
  Sint32 wheelX, wheelY;
  JoyAxisEvent axisEvent;
  Uint8 v_joyButton;
  SDL_Keycode v_nKey;
  SDL_Keymod v_mod;
  std::string v_utf8Char;

  if (i_type == INPUT_DOWN &&
      i_xmkey == (*Input::instance()->getGlobalKey(INPUT_RELOADFILESTODB))) {
    StateManager::instance()->pushState(new StateUpdateDb());
  }

  else if (i_xmkey.toKeyboard(v_nKey, v_mod, v_utf8Char)) {
    switch (i_type) {
      case INPUT_DOWN:
        m_GUI->keyDown(v_nKey, v_mod, v_utf8Char);
        break;
      case INPUT_UP:
        m_GUI->keyUp(v_nKey, v_mod, v_utf8Char);
        break;
      case INPUT_TEXT:
        m_GUI->textInput(v_nKey, v_mod, v_utf8Char);
        break;
    }
    checkEvents();
  }

  else if (i_xmkey.toMouse(nX, nY, nButton)) {
    if (i_xmkey.getRepetition() == 0) {
      if (i_type == INPUT_DOWN) {
        if (nButton == SDL_BUTTON_LEFT) {
          m_GUI->mouseLDown(nX, nY);
          checkEvents();
        } else if (nButton == SDL_BUTTON_RIGHT) {
          m_GUI->mouseRDown(nX, nY);
          checkEvents();
        }
      } else if (i_type == INPUT_UP) {
        if (nButton == SDL_BUTTON_LEFT) {
          m_GUI->mouseLUp(nX, nY);
          checkEvents();
        } else if (nButton == SDL_BUTTON_RIGHT) {
          m_GUI->mouseRUp(nX, nY);
          checkEvents();
        }
      }
    } else if (i_xmkey.getRepetition() == 1) {
      if (nButton == SDL_BUTTON_LEFT) {
        m_GUI->mouseLDoubleClick(nX, nY);
        checkEvents();
      }
    }
  }

  else if (i_xmkey.toMouseWheel(nX, nY, wheelX, wheelY)) {
    if (i_type == INPUT_SCROLL) {
      if (wheelY > 0) {
        m_GUI->mouseWheelUp(nX, nY, wheelX, wheelY);
      } else if (wheelY < 0) {
        m_GUI->mouseWheelDown(nX, nY, wheelX, wheelY);
      }
      checkEvents();
    }
  }

  else if (i_type == INPUT_DOWN &&
           i_xmkey.toJoystickButton(axisEvent.joystickNum, v_joyButton)) {
    m_GUI->joystickButtonDown(axisEvent.joystickNum, v_joyButton);
    checkEvents();
  }

  else if (i_xmkey.toJoystickAxisMotion(axisEvent)) {
    if (JoystickInput::instance()->joyAxisRepeat(axisEvent))
      m_GUI->joystickAxisMotion(axisEvent);

    checkEvents();
  }

  GameState::xmKey(i_type, i_xmkey);
}
