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
#include "xmscene/Camera.h"
#include <stdint.h>
#include <cstdlib>
#include <utility>

static const uint32_t JOYSTICK_REPEAT_DELAY_MS = 500;
static const float JOYSTICK_REPEAT_RATE_HZ = 1000 / 33.0f;

StateMenu::StateMenu(bool drawStateBehind, bool updateStatesBehind)
  : GameState(drawStateBehind, updateStatesBehind) {
  m_GUI = NULL;
  m_showCursor = true;

  m_renderFps = XMSession::instance()->maxRenderFps();
  m_updateFps = 30;

  for (Uint8 i = 0; i < Input::instance()->getNumJoysticks(); i++) {
    m_joyAxes.push_back(JoyAxes());
  }
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

  resetJoyAxes();
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

Uint32 StateMenu::repeatTimerCallback(Uint32 interval, void *param) {
  SDL_Event event;
  SDL_UserEvent userEvent;

  userEvent.type = SDL_USEREVENT;

  userEvent.code = SDL_CONTROLLERAXISMOTION;
  userEvent.data1 = param;

  event.type = SDL_USEREVENT;
  event.user = userEvent;

  SDL_PushEvent(&event);

  return JOYSTICK_REPEAT_RATE_HZ;
}

void StateMenu::resetJoyAxis(JoyAxis &axis) {
  clearRepeatTimer(axis.repeatTimer);
  axis.isHeld = false;
  axis.dir = 0;
}

void StateMenu::resetJoyAxes() {
  for (auto &axes : m_joyAxes)
    for (auto &axis : axes)
      resetJoyAxis(axis);
}

void StateMenu::clearRepeatTimer(SDL_TimerID &timer) {
  if (timer) {
    SDL_RemoveTimer(timer);
    timer = 0;
  }
}

void StateMenu::handleJoyAxis(JoyAxisEvent event) {
  if (event.axis == (Uint8)SDL_CONTROLLER_AXIS_INVALID)
    return;

  auto &axis = getAxesByJoyIndex(event.joystickNum)[event.axis];

  std::string *joyId = Input::instance()->getJoyId(event.joystickNum);
  SDL_GameController *joystick = Input::instance()->getJoyById(joyId);

  if (isAxisInsideDeadzone(event.axis, event.axisValue)) {
    axis.lastDir = axis.dir;
    resetJoyAxis(axis);
    return;
  }

  int8_t dir = (0 < event.axisValue) - (event.axisValue < 0);
  bool dirSame = dir != 0 && std::abs(dir) == std::abs(axis.lastDir);

  if (dirSame || !axis.isHeld) {
    for (auto &axes : m_joyAxes)
      for (auto &axis : axes)
        clearRepeatTimer(axis.repeatTimer);

    axis.isHeld = true;
    axis.repeatTimer = SDL_AddTimer(JOYSTICK_REPEAT_DELAY_MS,
        repeatTimerCallback, static_cast<void *>(this));
    axis.lastDir = axis.dir;
    axis.dir = dir;

    m_joystickRepeat = event;
    m_GUI->joystickAxisMotion(event);
  }
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
      i_xmkey ==
        (*Input::instance()->getGlobalKey(INPUT_RELOADFILESTODB))) {
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
    if (i_xmkey.getRepetition() == 1) {
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
    } else if (i_xmkey.getRepetition() == 2) {
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
    handleJoyAxis(axisEvent);

    checkEvents();
  }

  GameState::xmKey(i_type, i_xmkey);
}
