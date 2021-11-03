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
#include "xmscene/Camera.h"

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

Uint32 StateMenu::timerCallback(Uint32 interval, void *param) {
  SDL_Event event;
  SDL_UserEvent userEvent;

  userEvent.type = SDL_USEREVENT;

  userEvent.code = SDL_JOYAXISMOTION;
  userEvent.data1 = param;

  event.type = SDL_USEREVENT;
  event.user = userEvent;

  SDL_PushEvent(&event);

  return JOYSTICK_REPEAT_RATE;
}

void StateMenu::clearRepeatTimer(SDL_TimerID &timer) {
  if (timer) {
    SDL_RemoveTimer(timer);
    timer = 0;
  }
}

void StateMenu::handleJoyAxisRepeat(Uint8 v_joyNum, Uint8 v_joyAxis, Sint16 v_joyAxisValue) {
  JoyAxisIndex axis;

  switch (v_joyAxis) {
    case SDL_CONTROLLER_AXIS_LEFTX:        axis = JOYAXIS_LEFTX;        break;
    case SDL_CONTROLLER_AXIS_LEFTY:        axis = JOYAXIS_LEFTY;        break;
    case SDL_CONTROLLER_AXIS_RIGHTX:       axis = JOYAXIS_RIGHTX;       break;
    case SDL_CONTROLLER_AXIS_RIGHTY:       axis = JOYAXIS_RIGHTY;       break;
    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:  axis = JOYAXIS_TRIGGERLEFT;  break;
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: axis = JOYAXIS_TRIGGERRIGHT; break;
    default:
      return;
  }

  const bool isTrigger = (axis == JOYAXIS_TRIGGERLEFT || axis == JOYAXIS_TRIGGERRIGHT);

  int dir;
  /*
   * If the axis value is below the negative deadzone and this is a trigger, treat it
   * as if the trigger was released (because we want this to have a boolean state)
   */
  if (v_joyAxisValue <= -(GUI_JOYSTICK_MINIMUM_DETECTION) && !isTrigger)
    dir = 0;
  else if (v_joyAxisValue >= GUI_JOYSTICK_MINIMUM_DETECTION)
    dir = 1;
  else {
    // Only clear the opposite axes
    joyAxes[axis][0].isHeld = false;
    joyAxes[axis][1].isHeld = false;

    clearRepeatTimer(joyAxes[axis][0].timer);
    clearRepeatTimer(joyAxes[axis][1].timer);
    return;
  }

  if (!joyAxes[axis][dir].isHeld) {
    // Clear the timers so they don't keep ghosting. Note though that the `isHeld` state
    // will only be reset when the corresponding axis is released. This prevents a scenario
    // where a bogus event is triggered after the latter of 2 simultaneously held axes is released
    for (auto &axis : joyAxes) {
      clearRepeatTimer(axis[0].timer);
      clearRepeatTimer(axis[1].timer);
    }

    joyAxes[axis][dir].isHeld = true;

    m_GUI->joystickAxisMotion(v_joyNum, v_joyAxis, v_joyAxisValue);

    m_joystickRepeat = { v_joyNum, v_joyAxis, v_joyAxisValue };
    joyAxes[axis][dir].timer = SDL_AddTimer(JOYSTICK_REPEAT_DELAY, timerCallback, (void *)this);
  }

  // Clear the timer for the opposite axis
  clearRepeatTimer(joyAxes[axis][1-dir].timer);
}

void StateMenu::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  int nX, nY;
  Uint8 nButton;
  Sint32 wheelX, wheelY;
  Uint8 v_joyNum;
  Uint8 v_joyAxis;
  Sint16 v_joyAxisValue;
  Uint8 v_joyButton;
  SDL_Keycode v_nKey;
  SDL_Keymod v_mod;
  std::string v_utf8Char;

  if (i_type == INPUT_DOWN &&
      i_xmkey ==
        (*InputHandler::instance()->getGlobalKey(INPUT_RELOADFILESTODB))) {
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
           i_xmkey.toJoystickButton(v_joyNum, v_joyButton)) {
    m_GUI->joystickButtonDown(v_joyNum, v_joyButton);
    checkEvents();
  }

  else if (i_xmkey.toJoystickAxisMotion(v_joyNum, v_joyAxis, v_joyAxisValue)) {
    handleJoyAxisRepeat(v_joyNum, v_joyAxis, v_joyAxisValue);

    checkEvents();
  }

  GameState::xmKey(i_type, i_xmkey);
}
