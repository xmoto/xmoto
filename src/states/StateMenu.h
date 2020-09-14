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

#ifndef __STATEMENU_H__
#define __STATEMENU_H__

#include <array>

#include "GameState.h"

/* These should be pretty reasonable defaults */
#define JOYSTICK_REPEAT_DELAY 500 // 500 ms
#define JOYSTICK_REPEAT_RATE (1000 / 33) // 33 times/second

class UIRoot;
class GameApp;

struct JoystickEvent {
  Uint8 joyNum;
  Uint8 joyAxis;
  Sint16 joyAxisValue;
};

struct JoystickRepeatInfo {
  bool isHeld;
  SDL_TimerID timer;
};

enum JoyAxisIndex {
  JOYAXIS_LEFTX = 0,
  JOYAXIS_LEFTY,

  JOYAXIS_RIGHTX,
  JOYAXIS_RIGHTY,

  JOYAXIS_TRIGGERLEFT,
  JOYAXIS_TRIGGERRIGHT,

  NUM_JOYAXES
};

class StateMenu : public GameState {
public:
  StateMenu(bool drawStateBehind, bool updateStatesBehind);
  virtual ~StateMenu();

  virtual void enter();
  virtual void leave();
  /* called when a new state is pushed or poped on top of the
     current one*/
  virtual void enterAfterPop();
  virtual void leaveAfterPush();

  virtual bool update();
  virtual bool render();
  /* input */
  virtual void xmKey(InputEventType i_type, const XMKey &i_xmkey);

  void handleJoyAxisRepeat(Uint8 v_joyNum, Uint8 v_joyAxis, Sint16 v_joyAxisValue);
  void clearRepeatTimer(SDL_TimerID &timer);

  UIRoot *getGUI() const { return m_GUI; }
  JoystickEvent getJoystickRepeat() const { return m_joystickRepeat; }

  static Uint32 timerCallback(Uint32 interval, void *param);

private:
  // the inner array has 2 values for indicating the direction; 0 is negative, 1 is positive
  std::array<std::array<JoystickRepeatInfo, 2>, NUM_JOYAXES> joyAxes;
  JoystickEvent m_joystickRepeat;

protected:
  virtual void checkEvents() = 0;
  UIRoot *m_GUI;
};

#endif
